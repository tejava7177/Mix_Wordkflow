#pragma once

#include <cmath>

#include <juce_audio_basics/juce_audio_basics.h>

#include "../Domain/Channel.h"

// Total makeup gain (dB): the user's makeup plus, when auto-gain is on, a static
// estimate that roughly matches the compressed output back to the input loudness
// so the effect can be judged at equal level (not just "quieter"). Shared by the
// engine and the transfer-curve display so they agree.
inline float compressorTotalMakeupDb(const CompValues& c) noexcept
{
    float makeup = c.makeupDb;
    if (c.autoGain)
    {
        const float slope = 1.0f - 1.0f / juce::jmax(1.0f, c.ratio);
        makeup += (0.0f - c.thresholdDb) * slope * 0.6f;   // level-match estimate
    }
    return makeup;
}

// A simple, transparent feed-forward peak compressor with stereo-linked
// detection. We roll our own (rather than a black box) so the gain-reduction
// meter is exact and every control maps to one clear behaviour — which suits an
// educational tool.
//
// Signal flow per sample: detect peak -> how far over threshold -> target gain
// reduction from the ratio -> smooth it with attack/release -> apply (plus
// makeup). The smoothed reduction in dB is exactly what the meter shows.
class ChannelCompressor
{
public:
    void reset() noexcept { envelopeGrDb = 0.0f; }

    // Processes a stereo block in place; returns the peak gain reduction (dB, >= 0)
    // over the block for metering.
    float process(float* left, float* right, int numSamples, const CompValues& c, double sampleRate) noexcept
    {
        const float threshold = c.thresholdDb;
        const float ratio = juce::jmax(1.0f, c.ratio);
        const float slope = 1.0f - 1.0f / ratio;
        const float attackCoeff = timeToCoeff(c.attackMs, sampleRate, 0.1f);
        const float releaseCoeff = timeToCoeff(c.releaseMs, sampleRate, 1.0f);
        const float makeup = juce::Decibels::decibelsToGain(compressorTotalMakeupDb(c));

        float maxGr = 0.0f;
        for (int n = 0; n < numSamples; ++n)
        {
            const float detector = juce::jmax(std::abs(left[n]), std::abs(right[n]));
            const float levelDb = juce::Decibels::gainToDecibels(detector, -100.0f);
            const float overDb = levelDb - threshold;
            const float targetGrDb = overDb > 0.0f ? overDb * slope : 0.0f;

            // Attack when reduction is increasing, release when it's recovering.
            const float coeff = targetGrDb > envelopeGrDb ? attackCoeff : releaseCoeff;
            envelopeGrDb += (targetGrDb - envelopeGrDb) * coeff;

            const float gain = juce::Decibels::decibelsToGain(-envelopeGrDb) * makeup;
            left[n] *= gain;
            right[n] *= gain;
            maxGr = juce::jmax(maxGr, envelopeGrDb);
        }
        return maxGr;
    }

private:
    static float timeToCoeff(float ms, double sampleRate, float minMs) noexcept
    {
        const float t = juce::jmax(minMs, ms) * 0.001f;
        return 1.0f - std::exp(-1.0f / (t * static_cast<float>(sampleRate)));
    }

    float envelopeGrDb { 0.0f };
};
