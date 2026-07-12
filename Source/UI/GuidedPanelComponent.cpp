#include "GuidedPanelComponent.h"

GuidedPanelComponent::GuidedPanelComponent()
{
    stepLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    stepLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(232, 176, 75));
    addAndMakeVisible(stepLabel);

    instructionLabel.setFont(juce::FontOptions(13.0f));
    instructionLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(210, 218, 228));
    instructionLabel.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(instructionLabel);

    backButton.onClick = [this] { if (onBack) onBack(); };
    addAndMakeVisible(backButton);
    nextButton.onClick = [this] { if (onNext) onNext(); };
    addAndMakeVisible(nextButton);
    exitButton.onClick = [this] { if (onExit) onExit(); };
    addAndMakeVisible(exitButton);
}

void GuidedPanelComponent::setStage(int index, int total, const MixStage& stage)
{
    stepLabel.setText("Step " + juce::String(index + 1) + " of " + juce::String(total)
                          + "  ·  " + stage.title,
                      juce::dontSendNotification);
    instructionLabel.setText(stage.instruction, juce::dontSendNotification);
    backButton.setEnabled(index > 0);
    nextButton.setButtonText(index == total - 1 ? "Finish" : "Next");
}

void GuidedPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour::fromRGB(38, 40, 34));
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(juce::Colour::fromRGB(232, 176, 75).withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 10.0f, 1.5f);
}

void GuidedPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(14, 10);

    auto buttons = area.removeFromRight(230);
    exitButton.setBounds(buttons.removeFromRight(80).reduced(2, 6));
    buttons.removeFromRight(8);
    nextButton.setBounds(buttons.removeFromRight(64).reduced(2, 6));
    buttons.removeFromRight(6);
    backButton.setBounds(buttons.removeFromRight(64).reduced(2, 6));

    area.removeFromRight(16);
    stepLabel.setBounds(area.removeFromTop(20));
    instructionLabel.setBounds(area);
}
