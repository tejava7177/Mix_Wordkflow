#pragma once

#include <atomic>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// Real-time FFT spectrum analyzer for one channel's signal.
//
// The audio thread pushes samples in; when a full FFT block has accumulated it is
// handed to the GUI thread (via a ready flag) which performs the transform and
// reads out magnitudes. This is the standard JUCE analyser hand-off: the audio
// thread never allocates or transforms, and the copy is guarded by the flag so
// the two threads never touch the FFT buffer at once.
class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;   // 2048
    static constexpr int numBins = fftSize / 2;

    // --- audio thread ---
    void pushBlock(const float* data, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
            pushSample(data[i]);
    }

    // --- GUI thread: fill magnitudes (dBFS) if a new block is ready ---
    bool render(std::vector<float>& magnitudesDb)
    {
        if (! nextBlockReady.load())
            return false;

        window.multiplyWithWindowingTable(fftData, (size_t) fftSize);
        fft.performFrequencyOnlyForwardTransform(fftData);

        magnitudesDb.resize((size_t) numBins);
        const float norm = 2.0f / (float) fftSize;
        for (int i = 0; i < numBins; ++i)
            magnitudesDb[(size_t) i] = juce::Decibels::gainToDecibels(fftData[i] * norm, -100.0f);

        nextBlockReady.store(false);
        return true;
    }

private:
    void pushSample(float sample) noexcept
    {
        if (fifoIndex == fftSize)
        {
            if (! nextBlockReady.load())
            {
                juce::FloatVectorOperations::clear(fftData, 2 * fftSize);
                juce::FloatVectorOperations::copy(fftData, fifo, fftSize);
                nextBlockReady.store(true);
            }
            fifoIndex = 0;
        }
        fifo[fifoIndex++] = sample;
    }

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize,
                                                 juce::dsp::WindowingFunction<float>::hann };
    float fifo[fftSize] {};
    float fftData[2 * fftSize] {};
    int fifoIndex { 0 };
    std::atomic<bool> nextBlockReady { false };
};
