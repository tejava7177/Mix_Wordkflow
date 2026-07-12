#pragma once

#include <memory>
#include <vector>

#include <juce_gui_extra/juce_gui_extra.h>

#include "Engine/AudioEngine.h"
#include "Guidance/MixStage.h"
#include "Guidance/Recommendation.h"
#include "UI/ChannelStripComponent.h"
#include "UI/CompressorEditorComponent.h"
#include "UI/EqEditorComponent.h"
#include "UI/GuidedPanelComponent.h"
#include "UI/TimelineComponent.h"

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
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void loadDemoSession();
    void openStems();
    void rebuildStrips();
    void selectChannel(int index);
    juce::String applyRecommendationToChannel(int index);
    void applyRecommendation(int index);
    void suggestAll();
    void timerCallback() override;

    void setGuided(bool shouldBeGuided);
    void updateGuidedPanel();
    void advanceStage(int delta);

    AudioEngine engine;
    int selectedIndex { -1 };
    bool focusGrabbed { false };

    std::vector<MixStage> stages { makeMixStages() };
    bool guidedMode { false };
    int stageIndex { 0 };
    juce::Rectangle<int> stripsRegion;

    juce::Label titleLabel;
    juce::TextButton openButton { "Open" };
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton loopButton { "Loop" };
    juce::TextButton editorsButton { "Editors" };
    bool editorsVisible { true };
    juce::TextButton exportButton { "Export" };
    juce::TextButton suggestAllButton { "Suggest all" };
    juce::TextButton guidedButton { "Guided" };
    juce::Label positionLabel;
    juce::Label emptyLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
    GuidedPanelComponent guidedPanel;
    TimelineComponent timeline;

    juce::OwnedArray<ChannelStripComponent> strips;
    std::unique_ptr<ChannelStripComponent> masterStrip;
    EqEditorComponent eqEditor;
    CompressorEditorComponent compEditor;
    std::vector<float> spectrumBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
