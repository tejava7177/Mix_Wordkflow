#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>

#include "Channel.h"

// A mixing session: the set of channels plus master state. Owns its channels.
struct Session
{
    std::vector<std::unique_ptr<Channel>> channels;

    double sampleRate { 48000.0 };
    juce::int64 lengthSamples { 0 };   // longest stem, defines the timeline

    // Master bus (message thread writes controls, audio thread writes meters).
    std::atomic<float> masterFaderDb { 0.0f };
    std::atomic<float> masterMeterPeakL { 0.0f };
    std::atomic<float> masterMeterPeakR { 0.0f };

    [[nodiscard]] bool isEmpty() const noexcept { return channels.empty(); }
    [[nodiscard]] int numChannels() const noexcept { return static_cast<int>(channels.size()); }

    [[nodiscard]] bool anyChannelSoloed() const noexcept
    {
        for (const auto& c : channels)
            if (c->solo.load())
                return true;
        return false;
    }

    [[nodiscard]] float masterLinearGain() const noexcept
    {
        return juce::Decibels::decibelsToGain(masterFaderDb.load());
    }

    void clear()
    {
        channels.clear();
        lengthSamples = 0;
    }
};
