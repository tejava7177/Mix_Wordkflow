#include "ChannelStripComponent.h"

ChannelStripComponent::ChannelStripComponent(Binding bindingToUse)
    : binding(std::move(bindingToUse))
{
    nameLabel.setText(binding.name, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel);

    roleLabel.setText(binding.roleText, juce::dontSendNotification);
    roleLabel.setJustificationType(juce::Justification::centred);
    roleLabel.setFont(juce::FontOptions(11.0f));
    roleLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(150, 162, 176));
    addAndMakeVisible(roleLabel);

    if (binding.pan != nullptr)
    {
        panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        panSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        panSlider.setRange(-1.0, 1.0, 0.01);
        panSlider.setValue(binding.pan->load(), juce::dontSendNotification);
        panSlider.onValueChange = [this] { binding.pan->store((float) panSlider.getValue()); };
        addAndMakeVisible(panSlider);
    }

    if (binding.solo != nullptr)
    {
        soloButton.setClickingTogglesState(true);
        soloButton.setToggleState(binding.solo->load(), juce::dontSendNotification);
        soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(90, 150, 60));
        soloButton.onClick = [this] { binding.solo->store(soloButton.getToggleState()); };
        addAndMakeVisible(soloButton);
    }

    if (binding.mute != nullptr)
    {
        muteButton.setClickingTogglesState(true);
        muteButton.setToggleState(binding.mute->load(), juce::dontSendNotification);
        muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(200, 70, 70));
        muteButton.onClick = [this] { binding.mute->store(muteButton.getToggleState()); };
        addAndMakeVisible(muteButton);
    }

    faderSlider.setSliderStyle(juce::Slider::LinearVertical);
    faderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    faderSlider.setRange(-60.0, 6.0, 0.1);
    faderSlider.setValue(binding.faderDb != nullptr ? binding.faderDb->load() : 0.0,
                         juce::dontSendNotification);
    faderSlider.onValueChange = [this]
    {
        if (binding.faderDb != nullptr)
            binding.faderDb->store((float) faderSlider.getValue());
        updateDbLabel();
    };
    addAndMakeVisible(faderSlider);

    dbLabel.setJustificationType(juce::Justification::centred);
    dbLabel.setFont(juce::FontOptions(11.0f));
    dbLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(150, 162, 176));
    addAndMakeVisible(dbLabel);
    updateDbLabel();

    addAndMakeVisible(meter);
}

void ChannelStripComponent::updateDbLabel()
{
    const double db = faderSlider.getValue();
    dbLabel.setText((db <= -60.0 ? juce::String("-inf") : juce::String(db, 1)) + " dB",
                    juce::dontSendNotification);
}

void ChannelStripComponent::refreshMeter()
{
    const float l = binding.meterL != nullptr ? binding.meterL->load() : 0.0f;
    const float r = binding.meterR != nullptr ? binding.meterR->load() : 0.0f;
    meter.setLevels(l, r);
}

void ChannelStripComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colour::fromRGB(30, 36, 45));
    g.fillRoundedRectangle(bounds, 6.0f);

    // colour tab at the top
    auto tab = bounds.removeFromTop(5.0f).reduced(6.0f, 0.0f);
    g.setColour(binding.colour);
    g.fillRoundedRectangle(tab.withY(bounds.getY() - 3.0f).withHeight(4.0f), 2.0f);
}

void ChannelStripComponent::resized()
{
    auto area = getLocalBounds().reduced(6, 8);

    nameLabel.setBounds(area.removeFromTop(18));
    roleLabel.setBounds(area.removeFromTop(15));
    area.removeFromTop(4);

    if (binding.pan != nullptr)
    {
        panSlider.setBounds(area.removeFromTop(20));
        area.removeFromTop(4);
    }

    if (binding.solo != nullptr || binding.mute != nullptr)
    {
        auto row = area.removeFromTop(22);
        const int half = row.getWidth() / 2;
        if (binding.solo != nullptr)
            soloButton.setBounds(row.removeFromLeft(half).reduced(2, 0));
        if (binding.mute != nullptr)
            muteButton.setBounds(row.reduced(2, 0));
        area.removeFromTop(6);
    }

    dbLabel.setBounds(area.removeFromBottom(16));
    area.removeFromBottom(4);

    // fader on the left, meter on the right of the remaining tall area
    auto faderArea = area.removeFromLeft(area.getWidth() - 16);
    area.removeFromLeft(4);
    faderSlider.setBounds(faderArea);
    meter.setBounds(area);
}
