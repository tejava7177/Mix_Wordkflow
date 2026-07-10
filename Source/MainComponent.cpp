#include "MainComponent.h"

MainComponent::MainComponent()
{
    titleLabel.setText("MixMentor", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    taglineLabel.setText("Learn to mix by doing it — a guided mixing studio.",
                         juce::dontSendNotification);
    taglineLabel.setJustificationType(juce::Justification::centredLeft);
    taglineLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(165, 178, 194));
    addAndMakeVisible(taglineLabel);

    stagesLabel.setText("Gain staging  >  Balance  >  Panning  >  EQ  >  Compression  >  Master",
                        juce::dontSendNotification);
    stagesLabel.setJustificationType(juce::Justification::centredLeft);
    stagesLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(120, 200, 175));
    addAndMakeVisible(stagesLabel);

    setSize(1200, 820);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(20, 24, 30));

    auto area = getLocalBounds().reduced(24);
    g.setColour(juce::Colour::fromRGB(34, 40, 50));
    g.fillRoundedRectangle(area.toFloat(), 18.0f);
    g.setColour(juce::Colour::fromRGB(54, 62, 76));
    g.drawRoundedRectangle(area.toFloat(), 18.0f, 2.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(48);
    titleLabel.setBounds(area.removeFromTop(44));
    area.removeFromTop(6);
    taglineLabel.setBounds(area.removeFromTop(26));
    area.removeFromTop(18);
    stagesLabel.setBounds(area.removeFromTop(24));
}
