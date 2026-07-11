#pragma once

#include <atomic>

#include <juce_audio_basics/juce_audio_basics.h>

#include "StemRole.h"

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

    // Metering (audio thread writes, UI reads). Linear peak per block, 0..1+.
    std::atomic<float> meterPeakL { 0.0f };
    std::atomic<float> meterPeakR { 0.0f };

    [[nodiscard]] float linearGain() const noexcept
    {
        return juce::Decibels::decibelsToGain(faderDb.load());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Channel)
};
