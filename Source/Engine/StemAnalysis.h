#pragma once

#include <cmath>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "../Domain/Channel.h"

// Measure a stem: level, peak, dynamics, spectral balance, and clipping. All
// deterministic DSP, run once on load, off the audio thread.
inline StemAnalysis analyzeStem(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    StemAnalysis a;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmax(1, buffer.getNumChannels());
    if (numSamples <= 0)
        return a;

    auto monoAt = [&](int i)
    {
        float s = 0.0f;
        for (int c = 0; c < numChannels; ++c)
            s += buffer.getReadPointer(c)[i];
        return s / static_cast<float>(numChannels);
    };

    double sumSquares = 0.0;
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        const float s = monoAt(i);
        const float mag = std::abs(s);
        peak = juce::jmax(peak, mag);
        if (mag >= 0.999f)
            a.clipping = true;
        sumSquares += static_cast<double>(s) * s;
    }
    const float rms = static_cast<float>(std::sqrt(sumSquares / numSamples));
    a.peakDb = juce::Decibels::gainToDecibels(peak, -100.0f);
    a.loudnessDb = juce::Decibels::gainToDecibels(rms, -100.0f);
    a.crestDb = a.peakDb - a.loudnessDb;

    // Spectral balance from an averaged FFT across the file.
    constexpr int order = 11;
    constexpr int fftSize = 1 << order;
    if (numSamples >= fftSize)
    {
        juce::dsp::FFT fft(order);
        juce::dsp::WindowingFunction<float> window((size_t) fftSize, juce::dsp::WindowingFunction<float>::hann);
        std::vector<float> fftData(2 * fftSize, 0.0f);
        std::vector<double> accum((size_t) (fftSize / 2), 0.0);

        const int windows = juce::jmin(24, numSamples / fftSize);
        for (int w = 0; w < windows; ++w)
        {
            const int start = windows > 1 ? (int) ((long long) w * (numSamples - fftSize) / (windows - 1)) : 0;
            for (int i = 0; i < fftSize; ++i)
                fftData[(size_t) i] = monoAt(start + i);
            std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);
            window.multiplyWithWindowingTable(fftData.data(), (size_t) fftSize);
            fft.performFrequencyOnlyForwardTransform(fftData.data());
            for (int i = 0; i < fftSize / 2; ++i)
                accum[(size_t) i] += fftData[(size_t) i];
        }

        double low = 0.0, mid = 0.0, high = 0.0;
        for (int i = 1; i < fftSize / 2; ++i)
        {
            const double freq = i * sampleRate / fftSize;
            const double m = accum[(size_t) i];
            if (freq < 250.0) low += m;
            else if (freq < 4000.0) mid += m;
            else high += m;
        }
        const double total = low + mid + high;
        if (total > 0.0)
        {
            a.lowRatio = (float) (low / total);
            a.midRatio = (float) (mid / total);
            a.highRatio = (float) (high / total);
        }
    }

    a.valid = true;
    return a;
}
