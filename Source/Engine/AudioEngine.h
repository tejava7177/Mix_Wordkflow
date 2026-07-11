#pragma once

#include <atomic>

#include <memory>
#include <vector>

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "../Domain/Session.h"
#include "Compressor.h"
#include "EqDsp.h"
#include "SpectrumAnalyzer.h"

// Owns the audio device and mixes the loaded stems in real time.
//
// Design: every stem is loaded fully into memory, so the whole session shares a
// single sample-accurate playhead — synced multitrack playback is then just
// indexing every channel's buffer at the same position. Per block, each channel
// is summed with its fader, pan, mute/solo applied, and peak levels are captured
// for the meters. The audio thread only reads atomics; the UI only writes them.
class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    AudioEngine();
    ~AudioEngine() override;

    // Load stem files as channels. Returns the number successfully loaded.
    int loadStems(const juce::Array<juce::File>& files);

    Session& getSession() noexcept { return session; }

    // Transport
    void play();
    void stop();
    void togglePlay();
    [[nodiscard]] bool isPlaying() const noexcept { return playing.load(); }
    void setLooping(bool shouldLoop) noexcept { looping.store(shouldLoop); }
    [[nodiscard]] bool isLooping() const noexcept { return looping.load(); }

    [[nodiscard]] double getPositionSeconds() const noexcept;
    [[nodiscard]] double getLengthSeconds() const noexcept;
    [[nodiscard]] double getSampleRate() const noexcept { return currentSampleRate; }

    // Spectrum analyzer for the currently selected channel (post-EQ output signal).
    void setAnalyzedChannel(int index) noexcept { analyzedChannel.store(index); }
    bool renderSpectrum(std::vector<float>& magnitudesDb) { return analyzer.render(magnitudesDb); }

    // juce::AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

private:
    void prepareBuffersForSampleRate(double sampleRate);
    void prepareDsp(double sampleRate, int blockSize);

    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
    Session session;

    // Per-channel processing state, parallel to session.channels.
    std::vector<std::unique_ptr<EqDsp::Chain>> channelEq;
    std::vector<ChannelCompressor> channelComp;
    std::vector<EqValues> lastEqValues;  // cache so coeffs rebuild only on change
    juce::AudioBuffer<float> scratch;    // one channel's block, reused
    int currentBlockSize { 512 };

    SpectrumAnalyzer analyzer;
    std::atomic<int> analyzedChannel { 0 };
    std::vector<float> analyzerMono;     // reused mono buffer for the analyzed channel

    std::atomic<bool> playing { false };
    std::atomic<bool> looping { true };
    std::atomic<int>  playhead { 0 };

    double currentSampleRate { 48000.0 };
    double stemSampleRate { 48000.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
