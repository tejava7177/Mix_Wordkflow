#pragma once

#include <juce_dsp/juce_dsp.h>

#include "../Domain/Channel.h"

// Shared 3-band EQ definition. Both the audio engine (to process) and the EQ
// editor (to draw the response curve) build coefficients from the same EqValues,
// so what the user sees always matches what they hear.
namespace EqDsp
{
using Filter = juce::dsp::IIR::Filter<float>;
using Coeffs = juce::dsp::IIR::Coefficients<float>;
using Duplicator = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;
using Chain = juce::dsp::ProcessorChain<Duplicator, Duplicator, Duplicator>; // HP, bell, high shelf

enum BandIndex { HighPass = 0, Bell = 1, HighShelf = 2 };

inline float clampFreq(double sampleRate, float freq)
{
    return juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.49), freq);
}

inline Coeffs::Ptr makeHighPass(double sampleRate, const EqValues& eq)
{
    return Coeffs::makeHighPass(sampleRate, clampFreq(sampleRate, eq.hpFreq));
}

inline Coeffs::Ptr makeBell(double sampleRate, const EqValues& eq)
{
    return Coeffs::makePeakFilter(sampleRate, clampFreq(sampleRate, eq.bellFreq),
                                  juce::jlimit(0.2f, 8.0f, eq.bellQ),
                                  juce::Decibels::decibelsToGain(eq.bellGainDb));
}

inline Coeffs::Ptr makeHighShelf(double sampleRate, const EqValues& eq)
{
    return Coeffs::makeHighShelf(sampleRate, clampFreq(sampleRate, eq.shelfFreq),
                                 0.707f, juce::Decibels::decibelsToGain(eq.shelfGainDb));
}

// Combined magnitude response (linear) at a frequency, honouring on/off flags.
inline double magnitude(double sampleRate, const EqValues& eq, double frequency)
{
    if (! eq.eqOn)
        return 1.0;

    double m = 1.0;
    if (eq.hpOn)
        m *= makeHighPass(sampleRate, eq)->getMagnitudeForFrequency(frequency, sampleRate);
    m *= makeBell(sampleRate, eq)->getMagnitudeForFrequency(frequency, sampleRate);
    m *= makeHighShelf(sampleRate, eq)->getMagnitudeForFrequency(frequency, sampleRate);
    return m;
}
} // namespace EqDsp
