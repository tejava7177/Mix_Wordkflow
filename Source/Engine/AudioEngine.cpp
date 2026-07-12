#include "AudioEngine.h"

#include <cmath>

#include "StemAnalysis.h"

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

// A sentinel that never equals real Eq values, so the first block always builds
// coefficients.
EqValues staleEqSentinel()
{
    EqValues v;
    v.hpFreq = -1.0f;
    return v;
}
} // namespace

AudioEngine::AudioEngine()
{
    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);
    deviceManager.addAudioCallback(this);

    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        currentSampleRate = device->getCurrentSampleRate();
        currentBlockSize = device->getCurrentBufferSizeSamples();
    }
}

AudioEngine::~AudioEngine()
{
    deviceManager.removeAudioCallback(this);
}

int AudioEngine::loadStems(const juce::Array<juce::File>& files)
{
    stop();
    session.clear();
    channelEq.clear();
    channelComp.clear();
    lastEqValues.clear();
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
        channel->analysis = analyzeStem(channel->audio, reader->sampleRate);

        maxLength = juce::jmax(maxLength, length);
        sourceRate = reader->sampleRate;
        session.channels.push_back(std::move(channel));
        channelEq.push_back(std::make_unique<EqDsp::Chain>());
        channelComp.emplace_back();
        lastEqValues.push_back(staleEqSentinel());
        ++index;
    }

    if (session.isEmpty())
        return 0;

    session.sampleRate = sourceRate;
    session.lengthSamples = maxLength;
    stemSampleRate = sourceRate;

    prepareBuffersForSampleRate(currentSampleRate);
    prepareDsp(currentSampleRate, currentBlockSize);
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

void AudioEngine::prepareDsp(double sampleRate, int blockSize)
{
    currentBlockSize = juce::jmax(blockSize, 512);
    const int scratchSize = juce::jmax(currentBlockSize, 2048);
    scratch.setSize(2, scratchSize);
    analyzerMono.resize(static_cast<size_t>(scratchSize));

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(scratchSize), 2 };
    for (size_t i = 0; i < channelEq.size(); ++i)
    {
        auto& chain = *channelEq[i];
        const EqValues eq = session.channels[i]->readEq();

        // Seed coefficients BEFORE prepare so the duplicator's per-channel filters
        // are created referencing a valid coefficients object. Afterwards we only
        // ever update those coefficients in place (see the callback).
        chain.get<EqDsp::HighPass>().state = EqDsp::makeHighPass(sampleRate, eq);
        chain.get<EqDsp::Bell>().state = EqDsp::makeBell(sampleRate, eq);
        chain.get<EqDsp::HighShelf>().state = EqDsp::makeHighShelf(sampleRate, eq);
        chain.prepare(spec);
        chain.reset();
        chain.setBypassed<EqDsp::HighPass>(! eq.hpOn);
    }
    for (auto& comp : channelComp)
        comp.reset();
    std::fill(lastEqValues.begin(), lastEqValues.end(), staleEqSentinel());
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

bool AudioEngine::exportMixdown(const juce::File& outputFile)
{
    if (session.isEmpty() || session.lengthSamples <= 0)
        return false;

    const double sr = currentSampleRate;
    const int total = static_cast<int>(session.lengthSamples);
    const int numCh = session.numChannels();
    const int blockSize = 4096;

    // Fresh, independent DSP so the live audio thread's state is never touched.
    juce::dsp::ProcessSpec spec { sr, static_cast<juce::uint32>(blockSize), 2 };
    std::vector<std::unique_ptr<EqDsp::Chain>> eqs;
    std::vector<ChannelCompressor> comps(static_cast<size_t>(numCh));
    for (int i = 0; i < numCh; ++i)
    {
        auto chain = std::make_unique<EqDsp::Chain>();
        const EqValues eq = session.channels[static_cast<size_t>(i)]->readEq();
        chain->get<EqDsp::HighPass>().state = EqDsp::makeHighPass(sr, eq);
        chain->get<EqDsp::Bell>().state = EqDsp::makeBell(sr, eq);
        chain->get<EqDsp::HighShelf>().state = EqDsp::makeHighShelf(sr, eq);
        chain->prepare(spec);
        chain->reset();
        chain->setBypassed<EqDsp::HighPass>(! eq.hpOn);
        eqs.push_back(std::move(chain));
    }

    outputFile.deleteFile();
    auto stream = std::make_unique<juce::FileOutputStream>(outputFile);
    if (! stream->openedOk())
        return false;

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(stream.get(), sr, 2, 24, {}, 0));
    if (writer == nullptr)
        return false;
    stream.release();   // writer owns the stream now

    juce::AudioBuffer<float> channelBuffer(2, blockSize);
    juce::AudioBuffer<float> mixBuffer(2, blockSize);
    const float masterGain = session.masterLinearGain();

    for (int pos = 0; pos < total; pos += blockSize)
    {
        const int n = juce::jmin(blockSize, total - pos);
        mixBuffer.clear();

        for (int i = 0; i < numCh; ++i)
        {
            Channel& ch = *session.channels[static_cast<size_t>(i)];
            if (ch.mute.load())
                continue;

            const int clen = ch.audio.getNumSamples();
            const float* sL = ch.audio.getReadPointer(0);
            const float* sR = ch.audio.getNumChannels() > 1 ? ch.audio.getReadPointer(1) : sL;
            float* dL = channelBuffer.getWritePointer(0);
            float* dR = channelBuffer.getWritePointer(1);
            for (int k = 0; k < n; ++k)
            {
                const int p = pos + k;
                dL[k] = p < clen ? sL[p] : 0.0f;
                dR[k] = p < clen ? sR[p] : 0.0f;
            }

            const EqValues eq = ch.readEq();
            if (eq.eqOn)
            {
                *eqs[static_cast<size_t>(i)]->get<EqDsp::HighPass>().state = *EqDsp::makeHighPass(sr, eq);
                *eqs[static_cast<size_t>(i)]->get<EqDsp::Bell>().state = *EqDsp::makeBell(sr, eq);
                *eqs[static_cast<size_t>(i)]->get<EqDsp::HighShelf>().state = *EqDsp::makeHighShelf(sr, eq);
                eqs[static_cast<size_t>(i)]->setBypassed<EqDsp::HighPass>(! eq.hpOn);
                juce::dsp::AudioBlock<float> block(channelBuffer);
                auto sub = block.getSubBlock(0, static_cast<size_t>(n));
                juce::dsp::ProcessContextReplacing<float> context(sub);
                eqs[static_cast<size_t>(i)]->process(context);
            }

            const CompValues comp = ch.readComp();
            if (comp.compOn)
                comps[static_cast<size_t>(i)].process(dL, dR, n, comp, sr);

            const float gain = ch.linearGain();
            const float theta = (ch.pan.load() + 1.0f) * (juce::MathConstants<float>::pi * 0.25f);
            const float leftGain = std::cos(theta) * gain;
            const float rightGain = std::sin(theta) * gain;
            float* mL = mixBuffer.getWritePointer(0);
            float* mR = mixBuffer.getWritePointer(1);
            for (int k = 0; k < n; ++k)
            {
                mL[k] += dL[k] * leftGain;
                mR[k] += dR[k] * rightGain;
            }
        }

        mixBuffer.applyGain(0, n, masterGain);
        if (! writer->writeFromAudioSampleBuffer(mixBuffer, 0, n))
            return false;
    }

    writer.reset();   // flush and close
    return true;
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

    const bool active = playing.load() && ! session.isEmpty() && numSamples <= scratch.getNumSamples();
    if (! active)
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

    for (int i = 0; i < session.numChannels(); ++i)
    {
        Channel& channel = *session.channels[static_cast<size_t>(i)];

        // Copy this channel's block into the scratch buffer (zero-padded past its end).
        const int channelLength = channel.audio.getNumSamples();
        const float* srcL = channel.audio.getReadPointer(0);
        const float* srcR = channel.audio.getNumChannels() > 1 ? channel.audio.getReadPointer(1) : srcL;
        float* dstL = scratch.getWritePointer(0);
        float* dstR = scratch.getWritePointer(1);
        for (int n = 0; n < numSamples; ++n)
        {
            const int pos = startPos + n;
            const bool inRange = pos < channelLength;
            dstL[n] = inRange ? srcL[pos] : 0.0f;
            dstR[n] = inRange ? srcR[pos] : 0.0f;
        }

        // EQ: rebuild coefficients only when the parameters changed.
        const EqValues eq = channel.readEq();
        auto& chain = *channelEq[static_cast<size_t>(i)];
        if (eq != lastEqValues[static_cast<size_t>(i)])
        {
            // Update coefficients IN PLACE so the duplicator's per-channel filters
            // (which share these objects) pick up the change.
            *chain.get<EqDsp::HighPass>().state = *EqDsp::makeHighPass(currentSampleRate, eq);
            *chain.get<EqDsp::Bell>().state = *EqDsp::makeBell(currentSampleRate, eq);
            *chain.get<EqDsp::HighShelf>().state = *EqDsp::makeHighShelf(currentSampleRate, eq);
            chain.setBypassed<EqDsp::HighPass>(! eq.hpOn);
            lastEqValues[static_cast<size_t>(i)] = eq;
        }
        if (eq.eqOn)
        {
            juce::dsp::AudioBlock<float> block(scratch);
            auto sub = block.getSubBlock(0, static_cast<size_t>(numSamples));
            juce::dsp::ProcessContextReplacing<float> context(sub);
            chain.process(context);
        }

        // Compressor (after EQ), with exact gain reduction for the meter.
        const CompValues comp = channel.readComp();
        float grDb = 0.0f;
        if (comp.compOn)
            grDb = channelComp[static_cast<size_t>(i)].process(dstL, dstR, numSamples, comp, currentSampleRate);
        channel.meterGrDb.store(grDb);

        // Feed the analyzer with this channel's fully processed signal (post EQ +
        // compressor), so the spectrum reflects everything the user changes.
        if (i == analyzedChannel.load() && numSamples <= static_cast<int>(analyzerMono.size()))
        {
            for (int n = 0; n < numSamples; ++n)
                analyzerMono[static_cast<size_t>(n)] = 0.5f * (dstL[n] + dstR[n]);
            analyzer.pushBlock(analyzerMono.data(), numSamples);
        }

        // Fader, pan, sum, meter.
        const bool audible = ! channel.mute.load() && (! anySolo || channel.solo.load());
        const float gain = channel.linearGain();
        const float theta = (channel.pan.load() + 1.0f) * (juce::MathConstants<float>::pi * 0.25f);
        const float leftGain = std::cos(theta) * gain;
        const float rightGain = std::sin(theta) * gain;

        float peakL = 0.0f;
        float peakR = 0.0f;
        for (int n = 0; n < numSamples; ++n)
        {
            const float sampleL = dstL[n] * leftGain;
            const float sampleR = dstR[n] * rightGain;
            if (audible)
            {
                outL[n] += sampleL;
                outR[n] += sampleR;
                peakL = juce::jmax(peakL, std::abs(sampleL));
                peakR = juce::jmax(peakR, std::abs(sampleR));
            }
        }
        channel.meterPeakL.store(peakL);
        channel.meterPeakR.store(peakR);
    }

    const float masterGain = session.masterLinearGain();
    float masterPeakL = 0.0f;
    float masterPeakR = 0.0f;
    for (int n = 0; n < numSamples; ++n)
    {
        outL[n] *= masterGain;
        outR[n] *= masterGain;
        masterPeakL = juce::jmax(masterPeakL, std::abs(outL[n]));
        masterPeakR = juce::jmax(masterPeakR, std::abs(outR[n]));
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
    if (device == nullptr)
        return;

    currentBlockSize = device->getCurrentBufferSizeSamples();
    prepareBuffersForSampleRate(device->getCurrentSampleRate());
    prepareDsp(device->getCurrentSampleRate(), currentBlockSize);
}

void AudioEngine::audioDeviceStopped()
{
}
