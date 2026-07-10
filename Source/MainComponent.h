#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

// Placeholder shell for the MixMentor main window. The guided mixing console,
// channel strips, and audio engine are implemented in later phases (see
// docs/DEVELOPMENT_ROADMAP.md).
class MainComponent final : public juce::Component
{
public:
    MainComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::Label taglineLabel;
    juce::Label stagesLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
