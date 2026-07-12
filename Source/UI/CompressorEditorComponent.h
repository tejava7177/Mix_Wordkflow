#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Domain/Channel.h"

// Editor for one channel's compressor: threshold, ratio, attack, release and
// makeup, plus a live gain-reduction meter so the user sees the compressor work.
class CompressorEditorComponent final : public juce::Component
{
public:
    CompressorEditorComponent();

    void setChannel(Channel* channel, juce::Colour accent);
    void refresh();        // re-read channel values into the controls
    void refreshMeter();   // called from the UI timer

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void pushToChannel();
    void pullFromChannel();
    void configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text,
                         double min, double max, double interval, double skewMidpoint = 0.0);

    Channel* channel { nullptr };
    juce::Colour accentColour { juce::Colours::teal };
    float displayGrDb { 0.0f };

    juce::Label titleLabel;
    juce::ToggleButton compOnButton { "Comp on" };
    juce::ToggleButton autoGainButton { "Auto gain" };
    juce::Slider threshold, ratio, attack, release, makeup;
    juce::Label thresholdL, ratioL, attackL, releaseL, makeupL;

    juce::Rectangle<int> meterArea;
    juce::Rectangle<int> curveArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorEditorComponent)
};
