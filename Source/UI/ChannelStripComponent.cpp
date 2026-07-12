#include "ChannelStripComponent.h"

namespace
{
// Draws single-letter button text without an ellipsis, at a small font, so "S"
// and "M" stay legible on very narrow strips instead of collapsing to "...".
class CompactButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                              : juce::TextButton::textColourOffId));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, false);
    }
};
} // namespace

ChannelStripComponent::ChannelStripComponent(Binding bindingToUse)
    : binding(std::move(bindingToUse))
{
    // Receive mouse-downs from all child controls too, so clicking anywhere on the
    // strip (fader, buttons, labels) selects the channel for editing.
    addMouseListener(this, true);

    compactButtonLnf = std::make_unique<CompactButtonLookAndFeel>();
    soloButton.setLookAndFeel(compactButtonLnf.get());
    muteButton.setLookAndFeel(compactButtonLnf.get());

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
    faderSlider.setRange(-60.0, 12.0, 0.1);
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

ChannelStripComponent::~ChannelStripComponent()
{
    // Detach the custom LookAndFeel before it is destroyed.
    soloButton.setLookAndFeel(nullptr);
    muteButton.setLookAndFeel(nullptr);
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

void ChannelStripComponent::refreshControls()
{
    if (binding.faderDb != nullptr)
        faderSlider.setValue(binding.faderDb->load(), juce::dontSendNotification);
    if (binding.pan != nullptr)
        panSlider.setValue(binding.pan->load(), juce::dontSendNotification);
    updateDbLabel();
}

void ChannelStripComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colour::fromRGB(30, 36, 45));
    g.fillRoundedRectangle(bounds, 6.0f);

    if (selected)
    {
        g.setColour(binding.colour.withAlpha(0.9f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 2.0f);
    }

    // colour tab at the top
    auto tab = bounds.removeFromTop(5.0f).reduced(6.0f, 0.0f);
    g.setColour(binding.colour);
    g.fillRoundedRectangle(tab.withY(bounds.getY() - 3.0f).withHeight(4.0f), 2.0f);
}

void ChannelStripComponent::mouseDown(const juce::MouseEvent&)
{
    if (onSelect != nullptr)
        onSelect();
}

void ChannelStripComponent::setSelected(bool shouldBeSelected)
{
    if (selected != shouldBeSelected)
    {
        selected = shouldBeSelected;
        repaint();
    }
}

void ChannelStripComponent::resized()
{
    // Narrow strips (many tracks) drop the pan control and tighten spacing so the
    // remaining controls stay legible instead of collapsing to "...".
    const bool compact = getWidth() < 74;
    auto area = getLocalBounds().reduced(compact ? 4 : 6, 8);

    nameLabel.setBounds(area.removeFromTop(18));
    roleLabel.setBounds(area.removeFromTop(15));
    area.removeFromTop(4);

    if (binding.pan != nullptr)
    {
        panSlider.setVisible(! compact);
        if (! compact)
        {
            panSlider.setBounds(area.removeFromTop(20));
            area.removeFromTop(4);
        }
    }

    if (binding.solo != nullptr || binding.mute != nullptr)
    {
        auto row = area.removeFromTop(22);
        const int g = compact ? 2 : 4;
        const int half = (row.getWidth() - g) / 2;
        if (binding.solo != nullptr)
            soloButton.setBounds(row.removeFromLeft(half));
        if (binding.mute != nullptr)
        {
            row.removeFromLeft(g);
            muteButton.setBounds(row);
        }
        area.removeFromTop(6);
    }

    dbLabel.setBounds(area.removeFromBottom(16));
    area.removeFromBottom(4);

    // fader on the left, meter on the right of the remaining tall area
    const int meterW = compact ? 10 : 16;
    auto faderArea = area.removeFromLeft(area.getWidth() - meterW);
    area.removeFromLeft(compact ? 2 : 4);
    faderSlider.setBounds(faderArea);
    meter.setBounds(area);
}
