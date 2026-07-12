#pragma once

#include <memory>
#include <vector>

#include <juce_gui_extra/juce_gui_extra.h>

#include "Engine/AudioEngine.h"
#include "Guidance/MixStage.h"
#include "Guidance/Recommendation.h"
#include "UI/AnalysisPanelComponent.h"
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
    enum class EditorView { None, Eq, Comp };

    void loadDemoSession();
    void openStems();
    void rebuildStrips();
    void selectChannel(int index);
    void setEditorView(EditorView view);
    void updateAnalysisPanel(int index);
    juce::String applyRecommendationToChannel(int index);
    void applyEqRecommendation(int index);
    void applyCompRecommendation(int index);
    void suggestAll();
    void timerCallback() override;

    void setGuided(bool shouldBeGuided);
    void updateGuidedPanel();
    void advanceStage(int delta);

    AudioEngine engine;
    int selectedIndex { -1 };
    bool focusGrabbed { false };

    // Debounce for re-measuring the selected stem after the user changes settings.
    EqValues lastPanelEq;
    CompValues lastPanelComp;
    float lastPanelFader { 0.0f };
    int analyzedPanelChannel { -1 };
    bool panelDirty { false };
    int panelSettleTicks { 0 };

    std::vector<MixStage> stages { makeMixStages() };
    bool guidedMode { false };
    int stageIndex { 0 };
    juce::Rectangle<int> stripsRegion;

    juce::Label titleLabel;
    juce::TextButton openButton { "Open" };
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton loopButton { "Loop" };
    juce::TextButton eqButton { "EQ" };
    juce::TextButton compButton { "Comp" };
    EditorView editorView { EditorView::Eq };
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
    AnalysisPanelComponent analysisPanel;
    EqEditorComponent eqEditor;
    CompressorEditorComponent compEditor;
    std::vector<float> spectrumBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
