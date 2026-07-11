#include "CompressorEditorComponent.h"

#include "../Engine/Compressor.h"

namespace
{
constexpr float kMaxGrDb = 20.0f;
constexpr float kCurveMinDb = -48.0f;   // transfer-curve axis range
constexpr float kCurveMaxDb = 6.0f;
}

CompressorEditorComponent::CompressorEditorComponent()
{
    titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    titleLabel.setText("Compressor", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    compOnButton.onClick = [this] { pushToChannel(); };
    addAndMakeVisible(compOnButton);
    autoGainButton.onClick = [this] { pushToChannel(); repaint(); };
    addAndMakeVisible(autoGainButton);

    configureSlider(threshold, thresholdL, "Threshold", -60.0, 0.0, 0.1);
    configureSlider(ratio, ratioL, "Ratio", 1.0, 20.0, 0.1, 4.0);
    configureSlider(attack, attackL, "Attack", 0.1, 200.0, 0.1, 20.0);
    configureSlider(release, releaseL, "Release", 5.0, 1000.0, 1.0, 120.0);
    configureSlider(makeup, makeupL, "Makeup", 0.0, 24.0, 0.1);

    setChannel(nullptr, juce::Colours::teal);
}

void CompressorEditorComponent::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text,
                                                double min, double max, double interval, double skewMidpoint)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 58, 20);
    slider.setRange(min, max, interval);
    if (skewMidpoint > 0.0)
        slider.setSkewFactorFromMidPoint(skewMidpoint);
    slider.onValueChange = [this] { pushToChannel(); repaint(curveArea); };
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::FontOptions(12.0f));
    label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(150, 162, 176));
    addAndMakeVisible(label);
}

void CompressorEditorComponent::setChannel(Channel* channelToEdit, juce::Colour accent)
{
    channel = channelToEdit;
    accentColour = accent;
    displayGrDb = 0.0f;

    const bool has = channel != nullptr;
    titleLabel.setText(has ? channel->name + "  —  Compressor" : "Compressor", juce::dontSendNotification);
    for (auto* c : std::initializer_list<juce::Component*> {
             &compOnButton, &autoGainButton, &threshold, &ratio, &attack, &release, &makeup,
             &thresholdL, &ratioL, &attackL, &releaseL, &makeupL })
        c->setVisible(has);

    if (has)
        pullFromChannel();
    repaint();
}

void CompressorEditorComponent::pullFromChannel()
{
    const auto c = channel->readComp();
    compOnButton.setToggleState(c.compOn, juce::dontSendNotification);
    autoGainButton.setToggleState(c.autoGain, juce::dontSendNotification);
    threshold.setValue(c.thresholdDb, juce::dontSendNotification);
    ratio.setValue(c.ratio, juce::dontSendNotification);
    attack.setValue(c.attackMs, juce::dontSendNotification);
    release.setValue(c.releaseMs, juce::dontSendNotification);
    makeup.setValue(c.makeupDb, juce::dontSendNotification);
}

void CompressorEditorComponent::pushToChannel()
{
    if (channel == nullptr)
        return;
    channel->compOn.store(compOnButton.getToggleState());
    channel->compAutoGain.store(autoGainButton.getToggleState());
    channel->compThresholdDb.store((float) threshold.getValue());
    channel->compRatio.store((float) ratio.getValue());
    channel->compAttackMs.store((float) attack.getValue());
    channel->compReleaseMs.store((float) release.getValue());
    channel->compMakeupDb.store((float) makeup.getValue());
}

void CompressorEditorComponent::refreshMeter()
{
    const float gr = channel != nullptr ? channel->meterGrDb.load() : 0.0f;
    displayGrDb = juce::jmax(gr, displayGrDb * 0.8f);
    repaint(meterArea);
}

void CompressorEditorComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colour::fromRGB(30, 36, 45));
    g.fillRoundedRectangle(bounds, 8.0f);

    if (channel == nullptr)
        return;

    // gain-reduction meter (fills from the right as reduction increases)
    {
        auto area = meterArea.toFloat();
        g.setColour(juce::Colour::fromRGB(22, 26, 32));
        g.fillRoundedRectangle(area, 3.0f);
        const float norm = juce::jlimit(0.0f, 1.0f, displayGrDb / kMaxGrDb);
        if (norm > 0.0f)
        {
            auto fill = area.removeFromRight(area.getWidth() * norm);
            g.setColour(juce::Colour::fromRGB(226, 75, 74));
            g.fillRoundedRectangle(fill, 3.0f);
        }
        g.setColour(juce::Colour::fromRGB(150, 162, 176));
        g.setFont(juce::FontOptions(11.0f));
        g.drawText("GR  -" + juce::String(displayGrDb, 1) + " dB", meterArea, juce::Justification::centredLeft);
    }

    // transfer curve: input dB (x) -> output dB (y)
    {
        auto area = curveArea.toFloat();
        g.setColour(juce::Colour::fromRGB(22, 26, 32));
        g.fillRoundedRectangle(area, 6.0f);

        auto inToX = [&](float db) { return juce::jmap(db, kCurveMinDb, kCurveMaxDb, area.getX(), area.getRight()); };
        auto outToY = [&](float db) { return juce::jmap(db, kCurveMinDb, kCurveMaxDb, area.getBottom(), area.getY()); };

        // 1:1 reference (no compression)
        g.setColour(juce::Colour::fromRGB(48, 56, 68));
        g.drawLine(inToX(kCurveMinDb), outToY(kCurveMinDb), inToX(kCurveMaxDb), outToY(kCurveMaxDb), 1.0f);

        const auto c = channel->readComp();
        const float ratioValue = juce::jmax(1.0f, c.ratio);
        const float makeupTotal = compressorTotalMakeupDb(c);
        juce::Path curve;
        bool started = false;
        for (float in = kCurveMinDb; in <= kCurveMaxDb; in += 0.5f)
        {
            float out = in <= c.thresholdDb ? in : c.thresholdDb + (in - c.thresholdDb) / ratioValue;
            out += makeupTotal;
            const float x = inToX(in);
            const float y = outToY(juce::jlimit(kCurveMinDb, kCurveMaxDb, out));
            if (! started) { curve.startNewSubPath(x, y); started = true; }
            else curve.lineTo(x, y);
        }
        g.setColour(c.compOn ? accentColour : juce::Colour::fromRGB(90, 98, 110));
        g.strokePath(curve, juce::PathStrokeType(2.0f));

        // threshold marker
        g.setColour(juce::Colour::fromRGB(226, 75, 74).withAlpha(0.6f));
        g.drawVerticalLine((int) inToX(c.thresholdDb), area.getY(), area.getBottom());
    }
}

void CompressorEditorComponent::resized()
{
    auto area = getLocalBounds().reduced(14);

    auto header = area.removeFromTop(26);
    titleLabel.setBounds(header.removeFromLeft(230));
    compOnButton.setBounds(header.removeFromRight(96));
    header.removeFromRight(8);
    autoGainButton.setBounds(header.removeFromRight(100));

    meterArea = area.removeFromTop(18);
    area.removeFromTop(8);

    // transfer curve on the left, sliders on the right
    const int curveSize = juce::jmin(area.getHeight(), 150);
    curveArea = area.removeFromLeft(curveSize).withHeight(curveSize);
    area.removeFromLeft(16);

    auto layoutRow = [&area](juce::Label& l, juce::Slider& s)
    {
        auto row = area.removeFromTop(24);
        l.setBounds(row.removeFromLeft(74));
        s.setBounds(row);
    };
    layoutRow(thresholdL, threshold);
    layoutRow(ratioL, ratio);
    layoutRow(attackL, attack);
    layoutRow(releaseL, release);
    layoutRow(makeupL, makeup);
}
