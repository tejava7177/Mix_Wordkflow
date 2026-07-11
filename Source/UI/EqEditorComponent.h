#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Domain/Channel.h"

// Editor for one channel's 3-band EQ: a live frequency-response curve plus
// controls for a high-pass, a mid bell, and a high shelf. Reads/writes the
// channel's atomic EQ parameters; the curve is drawn from the same values the
// audio engine uses, so it always matches what is heard.
class EqEditorComponent final : public juce::Component
{
public:
    EqEditorComponent();

    // Point the editor at a channel (nullptr shows a placeholder).
    void setChannel(Channel* channel, juce::Colour accent, double sampleRate);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void pushToChannel();
    void pullFromChannel();
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text,
                         double min, double max, double interval, double skewMidpoint = 0.0);

    Channel* channel { nullptr };
    juce::Colour accentColour { juce::Colours::teal };
    double sampleRate { 48000.0 };

    juce::Label titleLabel;
    juce::ToggleButton eqOnButton { "EQ on" };
    juce::ToggleButton hpOnButton { "High-pass" };

    juce::Slider hpFreq, bellFreq, bellGain, bellQ, shelfFreq, shelfGain;
    juce::Label hpFreqL, bellFreqL, bellGainL, bellQL, shelfFreqL, shelfGainL;

    juce::Rectangle<int> curveArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqEditorComponent)
};
