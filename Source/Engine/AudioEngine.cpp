#include "AudioEngine.h"

#include <cmath>

namespace
{
const juce::Colour kPalette[] = {
    juce::Colour::fromRGB(0xE8, 0x8B, 0x4B), // orange
    juce::Colour::fromRGB(0x4E, 0xA1, 0x8A), // teal
    juce::Colour::fromRGB(0x7F, 0x77, 0xDD), // purple
    juce::Colour::fromRGB(0xC0, 0x6A, 0x8E), // pink
    juce::Colour::fromRGB(0x9A, 0xB0, 0x5A), // green
    juce::Colour::fromRGB(0x5B, 0x94, 0xC7), // blue
    juce::Colour::fromRGB(0xC7, 0x8A, 0x3E), // amber
    juce::Colour::fromRGB(0x88, 0x87, 0x80), // grey
};

void resampleChannel(juce::AudioBuffer<float>& buffer, double fromRate, double toRate)
{
    if (fromRate <= 0.0 || toRate <= 0.0 || juce::approximatelyEqual(fromRate, toRate))
        return;

    const double ratio = fromRate / toRate;   // input samples consumed per output sample
    const int newLength = static_cast<int>(std::ceil(buffer.getNumSamples() / ratio));
    juce::AudioBuffer<float> resampled(buffer.getNumChannels(), newLength);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        juce::LagrangeInterpolator interpolator;
        interpolator.reset();
        interpolator.process(ratio, buffer.getReadPointer(ch), resampled.getWritePointer(ch), newLength);
    }

    buffer = std::move(resampled);
}
} // namespace

AudioEngine::AudioEngine()
{
    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);
    deviceManager.addAudioCallback(this);

    if (auto* device = deviceManager.getCurrentAudioDevice())
        currentSampleRate = device->getCurrentSampleRate();
}

AudioEngine::~AudioEngine()
{
    deviceManager.removeAudioCallback(this);
}

int AudioEngine::loadStems(const juce::Array<juce::File>& files)
{
    stop();
    session.clear();
    playhead.store(0);

    int maxLength = 0;
    double sourceRate = 0.0;
    int index = 0;

    for (const auto& file : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr || reader->lengthInSamples <= 0)
            continue;

        const int length = static_cast<int>(reader->lengthInSamples);
        auto channel = std::make_unique<Channel>(file.getFileNameWithoutExtension());
        channel->audio.setSize(2, length);
        channel->audio.clear();
        reader->read(&channel->audio, 0, length, 0, true, true);
        channel->sourceSampleRate = reader->sampleRate;
        channel->role = guessRoleFromName(channel->name);
        channel->colour = kPalette[index % static_cast<int>(juce::numElementsInArray(kPalette))];

        maxLength = juce::jmax(maxLength, length);
        sourceRate = reader->sampleRate;
        session.channels.push_back(std::move(channel));
        ++index;
    }

    if (session.isEmpty())
        return 0;

    session.sampleRate = sourceRate;
    session.lengthSamples = maxLength;
    stemSampleRate = sourceRate;

    // Match whatever rate the device is currently running at.
    prepareBuffersForSampleRate(currentSampleRate);
    return session.numChannels();
}

void AudioEngine::prepareBuffersForSampleRate(double sampleRate)
{
    currentSampleRate = sampleRate;
    if (session.isEmpty() || juce::approximatelyEqual(sampleRate, stemSampleRate))
        return;

    int maxLength = 0;
    for (auto& channel : session.channels)
    {
        resampleChannel(channel->audio, stemSampleRate, sampleRate);
        maxLength = juce::jmax(maxLength, channel->audio.getNumSamples());
    }

    stemSampleRate = sampleRate;
    session.sampleRate = sampleRate;
    session.lengthSamples = maxLength;
    if (playhead.load() >= maxLength)
        playhead.store(0);
}

void AudioEngine::play()
{
    if (session.isEmpty())
        return;
    if (playhead.load() >= static_cast<int>(session.lengthSamples))
        playhead.store(0);
    playing.store(true);
}

void AudioEngine::stop()
{
    playing.store(false);
}

void AudioEngine::togglePlay()
{
    if (playing.load())
        stop();
    else
        play();
}

double AudioEngine::getPositionSeconds() const noexcept
{
    return currentSampleRate > 0.0 ? playhead.load() / currentSampleRate : 0.0;
}

double AudioEngine::getLengthSeconds() const noexcept
{
    return currentSampleRate > 0.0 ? session.lengthSamples / currentSampleRate : 0.0;
}

void AudioEngine::audioDeviceIOCallbackWithContext(const float* const*,
                                                   int,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext&)
{
    for (int ch = 0; ch < numOutputChannels; ++ch)
        if (outputChannelData[ch] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);

    if (numOutputChannels < 1 || outputChannelData[0] == nullptr)
        return;

    float* outL = outputChannelData[0];
    float* outR = numOutputChannels > 1 && outputChannelData[1] != nullptr ? outputChannelData[1] : outL;

    if (! playing.load() || session.isEmpty())
    {
        for (auto& channel : session.channels)
        {
            channel->meterPeakL.store(0.0f);
            channel->meterPeakR.store(0.0f);
        }
        session.masterMeterPeakL.store(0.0f);
        session.masterMeterPeakR.store(0.0f);
        return;
    }

    const int startPos = playhead.load();
    const int length = static_cast<int>(session.lengthSamples);
    const bool anySolo = session.anyChannelSoloed();

    for (auto& channel : session.channels)
    {
        const bool audible = ! channel->mute.load() && (! anySolo || channel->solo.load());
        const float gain = channel->linearGain();
        const float theta = (channel->pan.load() + 1.0f) * (juce::MathConstants<float>::pi * 0.25f);
        const float leftGain = std::cos(theta) * gain;
        const float rightGain = std::sin(theta) * gain;

        const int channelLength = channel->audio.getNumSamples();
        const float* srcL = channel->audio.getReadPointer(0);
        const float* srcR = channel->audio.getNumChannels() > 1 ? channel->audio.getReadPointer(1) : srcL;

        float peakL = 0.0f;
        float peakR = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            const int pos = startPos + i;
            if (pos >= channelLength)
                break;

            const float sampleL = srcL[pos] * leftGain;
            const float sampleR = srcR[pos] * rightGain;

            if (audible)
            {
                outL[i] += sampleL;
                outR[i] += sampleR;
                peakL = juce::jmax(peakL, std::abs(sampleL));
                peakR = juce::jmax(peakR, std::abs(sampleR));
            }
        }

        channel->meterPeakL.store(peakL);
        channel->meterPeakR.store(peakR);
    }

    const float masterGain = session.masterLinearGain();
    float masterPeakL = 0.0f;
    float masterPeakR = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        outL[i] *= masterGain;
        outR[i] *= masterGain;
        masterPeakL = juce::jmax(masterPeakL, std::abs(outL[i]));
        masterPeakR = juce::jmax(masterPeakR, std::abs(outR[i]));
    }
    session.masterMeterPeakL.store(masterPeakL);
    session.masterMeterPeakR.store(masterPeakR);

    int nextPos = startPos + numSamples;
    if (nextPos >= length)
    {
        if (looping.load())
            nextPos = 0;
        else
        {
            nextPos = length;
            playing.store(false);
        }
    }
    playhead.store(nextPos);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    if (device != nullptr)
        prepareBuffersForSampleRate(device->getCurrentSampleRate());
}

void AudioEngine::audioDeviceStopped()
{
}
