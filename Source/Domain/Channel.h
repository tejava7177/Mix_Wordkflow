#pragma once

#include <atomic>

#include <juce_audio_basics/juce_audio_basics.h>

#include "StemRole.h"

// A plain (non-atomic) snapshot of a channel's 3-band EQ, used to build filter
// coefficients on the audio thread and to draw the response curve in the UI.
struct EqValues
{
    bool  eqOn { true };
    bool  hpOn { false };
    float hpFreq { 80.0f };        // high-pass corner
    float bellFreq { 1000.0f };    // mid bell centre
    float bellGainDb { 0.0f };
    float bellQ { 1.0f };
    float shelfFreq { 6000.0f };   // high shelf corner
    float shelfGainDb { 0.0f };
};

inline bool operator==(const EqValues& a, const EqValues& b) noexcept
{
    return a.eqOn == b.eqOn && a.hpOn == b.hpOn && a.hpFreq == b.hpFreq
        && a.bellFreq == b.bellFreq && a.bellGainDb == b.bellGainDb && a.bellQ == b.bellQ
        && a.shelfFreq == b.shelfFreq && a.shelfGainDb == b.shelfGainDb;
}

inline bool operator!=(const EqValues& a, const EqValues& b) noexcept { return ! (a == b); }

// One mixer channel: a loaded stem plus its real-time-controllable state.
//
// The user-controlled parameters and the meter read-outs are atomics so the UI
// (message thread) and the audio callback (audio thread) can share them without
// locks. The audio buffer itself is written once at load time and only read on
// the audio thread thereafter.
struct Channel
{
    explicit Channel(juce::String channelName) : name(std::move(channelName)) {}

    juce::String name;
    juce::Colour colour { juce::Colours::grey };
    StemRole role { StemRole::Other };

    juce::AudioBuffer<float> audio;   // stereo, resampled to the engine sample rate
    double sourceSampleRate { 0.0 };

    // User controls (message thread writes, audio thread reads).
    std::atomic<float> faderDb { 0.0f };  // fader gain in dB
    std::atomic<float> pan { 0.0f };      // -1 (L) .. 0 (C) .. +1 (R)
    std::atomic<bool>  mute { false };
    std::atomic<bool>  solo { false };

    // 3-band EQ parameters (message thread writes, audio thread reads).
    std::atomic<bool>  eqOn { true };
    std::atomic<bool>  hpOn { false };
    std::atomic<float> hpFreq { 80.0f };
    std::atomic<float> bellFreq { 1000.0f };
    std::atomic<float> bellGainDb { 0.0f };
    std::atomic<float> bellQ { 1.0f };
    std::atomic<float> shelfFreq { 6000.0f };
    std::atomic<float> shelfGainDb { 0.0f };

    // Metering (audio thread writes, UI reads). Linear peak per block, 0..1+.
    std::atomic<float> meterPeakL { 0.0f };
    std::atomic<float> meterPeakR { 0.0f };

    [[nodiscard]] float linearGain() const noexcept
    {
        return juce::Decibels::decibelsToGain(faderDb.load());
    }

    [[nodiscard]] EqValues readEq() const noexcept
    {
        return { eqOn.load(), hpOn.load(), hpFreq.load(),
                 bellFreq.load(), bellGainDb.load(), bellQ.load(),
                 shelfFreq.load(), shelfGainDb.load() };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Channel)
};
