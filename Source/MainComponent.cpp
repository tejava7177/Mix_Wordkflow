#include "MainComponent.h"

#include <algorithm>

#include "Domain/StemRole.h"

namespace
{
juce::String formatTime(double seconds)
{
    const int total = juce::roundToInt(std::floor(seconds));
    return juce::String(total / 60) + ":" + juce::String(total % 60).paddedLeft('0', 2);
}

// Demo session location (development). Later this becomes a bundled resource.
juce::File demoSessionDir()
{
    return juce::File("/Users/simjuheun/Desktop/myProject/MixMentor/demo/indie-rock");
}
} // namespace

MainComponent::MainComponent()
{
    titleLabel.setText("MixMentor", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    playButton.onClick = [this] { engine.togglePlay(); };
    addAndMakeVisible(playButton);

    stopButton.onClick = [this] { engine.stop(); };
    addAndMakeVisible(stopButton);

    loopButton.setClickingTogglesState(true);
    loopButton.setToggleState(engine.isLooping(), juce::dontSendNotification);
    loopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(78, 161, 138));
    loopButton.onClick = [this] { engine.setLooping(loopButton.getToggleState()); };
    addAndMakeVisible(loopButton);

    positionLabel.setJustificationType(juce::Justification::centredRight);
    positionLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(165, 178, 194));
    addAndMakeVisible(positionLabel);

    emptyLabel.setJustificationType(juce::Justification::centred);
    emptyLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(200, 120, 120));
    addChildComponent(emptyLabel);

    loadDemoSession();

    setSize(1200, 760);
    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

void MainComponent::loadDemoSession()
{
    auto dir = demoSessionDir();
    if (! dir.isDirectory())
    {
        emptyLabel.setText("Demo session not found at\n" + dir.getFullPathName(),
                           juce::dontSendNotification);
        emptyLabel.setVisible(true);
        return;
    }

    auto found = dir.findChildFiles(juce::File::findFiles, false, "*.wav");
    std::sort(found.begin(), found.end(),
              [](const juce::File& a, const juce::File& b) { return a.getFileName() < b.getFileName(); });

    if (engine.loadStems(found) == 0)
    {
        emptyLabel.setText("No readable stems in\n" + dir.getFullPathName(),
                           juce::dontSendNotification);
        emptyLabel.setVisible(true);
        return;
    }

    emptyLabel.setVisible(false);
    rebuildStrips();
}

void MainComponent::rebuildStrips()
{
    strips.clear();
    masterStrip.reset();

    auto& session = engine.getSession();
    for (auto& channel : session.channels)
    {
        ChannelStripComponent::Binding b;
        b.name = channel->name;
        b.roleText = toDisplayString(channel->role);
        b.colour = channel->colour;
        b.faderDb = &channel->faderDb;
        b.pan = &channel->pan;
        b.mute = &channel->mute;
        b.solo = &channel->solo;
        b.meterL = &channel->meterPeakL;
        b.meterR = &channel->meterPeakR;

        auto* strip = new ChannelStripComponent(b);
        addAndMakeVisible(strip);
        strips.add(strip);
    }

    ChannelStripComponent::Binding mb;
    mb.name = "Master";
    mb.colour = juce::Colour::fromRGB(78, 161, 138);
    mb.faderDb = &session.masterFaderDb;
    mb.meterL = &session.masterMeterPeakL;
    mb.meterR = &session.masterMeterPeakR;
    masterStrip = std::make_unique<ChannelStripComponent>(mb);
    addAndMakeVisible(*masterStrip);

    resized();
}

void MainComponent::timerCallback()
{
    for (auto* strip : strips)
        strip->refreshMeter();
    if (masterStrip != nullptr)
        masterStrip->refreshMeter();

    positionLabel.setText(formatTime(engine.getPositionSeconds()) + " / "
                              + formatTime(engine.getLengthSeconds()),
                          juce::dontSendNotification);
    playButton.setButtonText(engine.isPlaying() ? "Pause" : "Play");
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(20, 24, 30));

    auto area = getLocalBounds().reduced(16);
    g.setColour(juce::Colour::fromRGB(28, 33, 41));
    g.fillRoundedRectangle(area.toFloat(), 14.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(24);

    auto header = area.removeFromTop(40);
    titleLabel.setBounds(header.removeFromLeft(200));
    positionLabel.setBounds(header.removeFromRight(160));
    header.removeFromLeft(12);
    playButton.setBounds(header.removeFromLeft(80).reduced(0, 4));
    header.removeFromLeft(8);
    stopButton.setBounds(header.removeFromLeft(80).reduced(0, 4));
    header.removeFromLeft(8);
    loopButton.setBounds(header.removeFromLeft(80).reduced(0, 4));

    area.removeFromTop(14);

    emptyLabel.setBounds(area);

    // console: channel strips + master on the right
    const int stripWidth = 96;
    auto console = area;

    if (masterStrip != nullptr)
    {
        auto masterArea = console.removeFromRight(stripWidth);
        masterStrip->setBounds(masterArea);
        console.removeFromRight(10);
    }

    for (auto* strip : strips)
    {
        strip->setBounds(console.removeFromLeft(stripWidth));
        console.removeFromLeft(4);
    }
}
