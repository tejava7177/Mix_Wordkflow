#pragma once

#include <memory>
#include <vector>

#include <juce_gui_extra/juce_gui_extra.h>

#include "Engine/AudioEngine.h"
#include "UI/ChannelStripComponent.h"
#include "UI/EqEditorComponent.h"

// Phase 1 console: loads a demo session, plays all stems in sync, and exposes a
// channel strip (fader / pan / mute / solo / meter) per stem plus a master strip.
class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void loadDemoSession();
    void rebuildStrips();
    void selectChannel(int index);
    void timerCallback() override;

    AudioEngine engine;
    int selectedIndex { -1 };

    juce::Label titleLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton loopButton { "Loop" };
    juce::Label positionLabel;
    juce::Label emptyLabel;

    juce::OwnedArray<ChannelStripComponent> strips;
    std::unique_ptr<ChannelStripComponent> masterStrip;
    EqEditorComponent eqEditor;
    std::vector<float> spectrumBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
