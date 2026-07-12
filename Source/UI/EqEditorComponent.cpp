#include "EqEditorComponent.h"

#include "../Engine/EqDsp.h"

namespace
{
constexpr float kMinFreq = 20.0f;
constexpr float kMaxFreq = 20000.0f;
constexpr float kDbRange = 18.0f;
}

EqEditorComponent::EqEditorComponent()
{
    titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    titleLabel.setText("EQ", juce::dontSendNotification);
    addAndMakeVisible(titleLabel);

    suggestButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(58, 66, 52));
    suggestButton.onClick = [this] { if (onSuggest) onSuggest(); };
    addAndMakeVisible(suggestButton);

    reasonLabel.setFont(juce::FontOptions(12.0f));
    reasonLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(180, 190, 200));
    reasonLabel.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(reasonLabel);

    eqOnButton.onClick = [this] { pushToChannel(); };
    addAndMakeVisible(eqOnButton);
    hpOnButton.onClick = [this] { pushToChannel(); };
    addAndMakeVisible(hpOnButton);

    configureSlider(hpFreq, hpFreqL, "HP freq", 20.0, 1000.0, 1.0, 150.0);
    configureSlider(bellFreq, bellFreqL, "Bell freq", 100.0, 12000.0, 1.0, 1000.0);
    configureSlider(bellGain, bellGainL, "Bell gain", -kDbRange, kDbRange, 0.1);
    configureSlider(bellQ, bellQL, "Bell Q", 0.2, 8.0, 0.01, 1.0);
    configureSlider(shelfFreq, shelfFreqL, "Shelf freq", 1000.0, 16000.0, 1.0, 5000.0);
    configureSlider(shelfGain, shelfGainL, "Shelf gain", -kDbRange, kDbRange, 0.1);

    setChannel(nullptr, juce::Colours::teal, 48000.0);
}

void EqEditorComponent::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text,
                                        double min, double max, double interval, double skewMidpoint)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
    slider.setRange(min, max, interval);
    if (skewMidpoint > 0.0)
        slider.setSkewFactorFromMidPoint(skewMidpoint);
    slider.onValueChange = [this] { pushToChannel(); };
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::FontOptions(12.0f));
    label.setColour(juce::Label::textColourId, juce::Colour::fromRGB(150, 162, 176));
    addAndMakeVisible(label);
}

void EqEditorComponent::setChannel(Channel* channelToEdit, juce::Colour accent, double sr)
{
    channel = channelToEdit;
    accentColour = accent;
    sampleRate = sr > 0.0 ? sr : 48000.0;

    const bool has = channel != nullptr;
    titleLabel.setText(has ? channel->name + "  —  EQ" : "EQ", juce::dontSendNotification);
    for (auto* c : std::initializer_list<juce::Component*> {
             &suggestButton, &reasonLabel,
             &eqOnButton, &hpOnButton, &hpFreq, &bellFreq, &bellGain, &bellQ, &shelfFreq, &shelfGain,
             &hpFreqL, &bellFreqL, &bellGainL, &bellQL, &shelfFreqL, &shelfGainL })
        c->setVisible(has);

    if (has)
        pullFromChannel();
    reasonLabel.setText({}, juce::dontSendNotification);
    spectrum.clear();
    repaint();
}

void EqEditorComponent::refresh()
{
    if (channel != nullptr)
        pullFromChannel();
    repaint();
}

void EqEditorComponent::setReason(const juce::String& text)
{
    reasonLabel.setText(text, juce::dontSendNotification);
}

void EqEditorComponent::setSpectrum(const std::vector<float>& magnitudesDb, double analyzerSampleRate)
{
    spectrumSampleRate = analyzerSampleRate > 0.0 ? analyzerSampleRate : 48000.0;
    if (spectrum.size() != magnitudesDb.size())
        spectrum = magnitudesDb;
    else
        for (size_t i = 0; i < magnitudesDb.size(); ++i)
            spectrum[i] = spectrum[i] * 0.7f + magnitudesDb[i] * 0.3f;  // smooth
    repaint();
}

void EqEditorComponent::pullFromChannel()
{
    const auto eq = channel->readEq();
    eqOnButton.setToggleState(eq.eqOn, juce::dontSendNotification);
    hpOnButton.setToggleState(eq.hpOn, juce::dontSendNotification);
    hpFreq.setValue(eq.hpFreq, juce::dontSendNotification);
    bellFreq.setValue(eq.bellFreq, juce::dontSendNotification);
    bellGain.setValue(eq.bellGainDb, juce::dontSendNotification);
    bellQ.setValue(eq.bellQ, juce::dontSendNotification);
    shelfFreq.setValue(eq.shelfFreq, juce::dontSendNotification);
    shelfGain.setValue(eq.shelfGainDb, juce::dontSendNotification);
}

void EqEditorComponent::pushToChannel()
{
    if (channel == nullptr)
        return;
    channel->eqOn.store(eqOnButton.getToggleState());
    channel->hpOn.store(hpOnButton.getToggleState());
    channel->hpFreq.store((float) hpFreq.getValue());
    channel->bellFreq.store((float) bellFreq.getValue());
    channel->bellGainDb.store((float) bellGain.getValue());
    channel->bellQ.store((float) bellQ.getValue());
    channel->shelfFreq.store((float) shelfFreq.getValue());
    channel->shelfGainDb.store((float) shelfGain.getValue());
    repaint();
}

void EqEditorComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colour::fromRGB(30, 36, 45));
    g.fillRoundedRectangle(bounds, 8.0f);

    if (channel == nullptr)
    {
        g.setColour(juce::Colour::fromRGB(120, 132, 146));
        g.setFont(juce::FontOptions(15.0f));
        g.drawText("Select a channel to edit its EQ", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto area = curveArea.toFloat();
    g.setColour(juce::Colour::fromRGB(22, 26, 32));
    g.fillRoundedRectangle(area, 6.0f);

    const float left = area.getX();
    const float right = area.getRight();
    const float top = area.getY();
    const float bottom = area.getBottom();

    auto freqToX = [&](double f)
    {
        return juce::jmap((float) std::log10(f), std::log10(kMinFreq), std::log10(kMaxFreq), left, right);
    };
    auto dbToY = [&](double db)
    {
        return juce::jmap((float) db, kDbRange, -kDbRange, top, bottom);
    };

    // live spectrum of the channel's input, drawn behind everything
    if (! spectrum.empty())
    {
        const int bins = (int) spectrum.size();
        const double fftSize = 2.0 * bins;
        auto specToY = [&](float db)
        {
            return juce::jmap(juce::jlimit(-90.0f, 0.0f, db), -90.0f, 0.0f, bottom, top);
        };
        juce::Path spec;
        bool started = false;
        for (int i = 1; i < bins; ++i)
        {
            const double freq = i * spectrumSampleRate / fftSize;
            if (freq < kMinFreq)
                continue;
            if (freq > kMaxFreq)
                break;
            const float x = freqToX(freq);
            const float y = specToY(spectrum[(size_t) i]);
            if (! started) { spec.startNewSubPath(x, bottom); spec.lineTo(x, y); started = true; }
            else spec.lineTo(x, y);
        }
        if (started)
        {
            spec.lineTo(right, bottom);
            spec.closeSubPath();
            g.setColour(accentColour.withAlpha(0.16f));
            g.fillPath(spec);
        }
    }

    // grid: 0 dB line + freq guides
    g.setColour(juce::Colour::fromRGB(48, 56, 68));
    const float zeroY = dbToY(0.0);
    g.drawHorizontalLine((int) zeroY, left, right);
    for (float f : { 100.0f, 1000.0f, 10000.0f })
    {
        const float x = freqToX(f);
        g.drawVerticalLine((int) x, top, bottom);
    }

    // response curve
    const auto eq = channel->readEq();
    juce::Path path;
    const int steps = juce::jmax(2, (int) area.getWidth());
    for (int i = 0; i <= steps; ++i)
    {
        const float x = left + (area.getWidth() * i) / steps;
        const double frac = (double) i / steps;
        const double freq = std::pow(10.0, juce::jmap(frac, 0.0, 1.0,
                                                      (double) std::log10(kMinFreq), (double) std::log10(kMaxFreq)));
        const double db = juce::Decibels::gainToDecibels(EqDsp::magnitude(sampleRate, eq, freq));
        const float y = dbToY(juce::jlimit(-kDbRange, kDbRange, (float) db));
        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }
    g.setColour(eq.eqOn ? accentColour : juce::Colour::fromRGB(90, 98, 110));
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void EqEditorComponent::resized()
{
    auto area = getLocalBounds().reduced(14);

    auto header = area.removeFromTop(28);
    titleLabel.setBounds(header.removeFromLeft(190));
    eqOnButton.setBounds(header.removeFromRight(84));
    hpOnButton.setBounds(header.removeFromRight(104));
    header.removeFromRight(8);
    suggestButton.setBounds(header.removeFromRight(84).reduced(0, 2));

    area.removeFromTop(4);
    reasonLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(6);

    auto controls = area.removeFromBottom(6 * 26 + 10);
    curveArea = area.reduced(2);

    controls.removeFromTop(10);
    auto layoutRow = [&controls](juce::Label& l, juce::Slider& s)
    {
        auto row = controls.removeFromTop(26);
        l.setBounds(row.removeFromLeft(78));
        s.setBounds(row);
    };
    layoutRow(hpFreqL, hpFreq);
    layoutRow(bellFreqL, bellFreq);
    layoutRow(bellGainL, bellGain);
    layoutRow(bellQL, bellQ);
    layoutRow(shelfFreqL, shelfFreq);
    layoutRow(shelfGainL, shelfGain);
}
