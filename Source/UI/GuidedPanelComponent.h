#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Guidance/MixStage.h"

// The guided-mode banner: shows the current step, a plain-language instruction,
// and navigation. Sits below the transport while guided mode is active.
class GuidedPanelComponent final : public juce::Component
{
public:
    GuidedPanelComponent();

    void setStage(int index, int total, const MixStage& stage);

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onBack;
    std::function<void()> onNext;
    std::function<void()> onExit;

private:
    juce::Label stepLabel;
    juce::Label instructionLabel;
    juce::TextButton backButton { "Back" };
    juce::TextButton nextButton { "Next" };
    juce::TextButton exitButton { "Exit guide" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuidedPanelComponent)
};
