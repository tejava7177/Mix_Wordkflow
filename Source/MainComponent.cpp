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

    openButton.onClick = [this] { openStems(); };
    addAndMakeVisible(openButton);

    playButton.onClick = [this] { engine.togglePlay(); };
    addAndMakeVisible(playButton);

    stopButton.onClick = [this] { engine.stop(); };
    addAndMakeVisible(stopButton);

    loopButton.setClickingTogglesState(true);
    loopButton.setToggleState(engine.isLooping(), juce::dontSendNotification);
    loopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(78, 161, 138));
    loopButton.onClick = [this] { engine.setLooping(loopButton.getToggleState()); };
    addAndMakeVisible(loopButton);

    exportButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Export mix as WAV",
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("mixmentor-mix.wav"),
            "*.wav");
        const auto flags = juce::FileBrowserComponent::saveMode
                         | juce::FileBrowserComponent::canSelectFiles
                         | juce::FileBrowserComponent::warnAboutOverwriting;
        fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File {})
                return;
            if (! file.hasFileExtension("wav"))
                file = file.withFileExtension("wav");
            const bool ok = engine.exportMixdown(file);
            juce::NativeMessageBox::showMessageBoxAsync(
                juce::MessageBoxIconType::InfoIcon, "MixMentor",
                ok ? ("Exported mix to\n" + file.getFullPathName()) : "Export failed.");
        });
    };
    addAndMakeVisible(exportButton);

    suggestAllButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(58, 66, 52));
    suggestAllButton.onClick = [this] { suggestAll(); };
    addAndMakeVisible(suggestAllButton);

    guidedButton.setClickingTogglesState(true);
    guidedButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(232, 176, 75));
    guidedButton.onClick = [this] { setGuided(guidedButton.getToggleState()); };
    addAndMakeVisible(guidedButton);

    guidedPanel.onBack = [this] { advanceStage(-1); };
    guidedPanel.onNext = [this] { advanceStage(1); };
    guidedPanel.onExit = [this] { setGuided(false); };
    addChildComponent(guidedPanel);

    editorsButton.setClickingTogglesState(true);
    editorsButton.setToggleState(true, juce::dontSendNotification);
    editorsButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(78, 90, 110));
    editorsButton.onClick = [this]
    {
        editorsVisible = editorsButton.getToggleState();
        eqEditor.setVisible(editorsVisible);
        compEditor.setVisible(editorsVisible);
        resized();
    };
    addAndMakeVisible(editorsButton);

    positionLabel.setJustificationType(juce::Justification::centredRight);
    positionLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(165, 178, 194));
    addAndMakeVisible(positionLabel);

    emptyLabel.setJustificationType(juce::Justification::centred);
    emptyLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(200, 120, 120));
    addChildComponent(emptyLabel);

    addAndMakeVisible(eqEditor);
    addAndMakeVisible(compEditor);
    eqEditor.onSuggest = [this] { applyRecommendation(selectedIndex); };

    timeline.onSeek = [this](double fraction) { engine.seekSeconds(fraction * engine.getLengthSeconds()); };
    addAndMakeVisible(timeline);

    // Keep keyboard shortcuts working: transport buttons don't hold focus, the
    // main component does, so space/enter/arrows reach keyPressed().
    for (auto* b : { &openButton, &playButton, &stopButton, &loopButton, &exportButton,
                     &suggestAllButton, &guidedButton, &editorsButton })
        b->setWantsKeyboardFocus(false);
    setWantsKeyboardFocus(true);

    loadDemoSession();

    setSize(1240, 900);
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

void MainComponent::openStems()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Open stems - select audio files or a folder",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff;*.flac;*.mp3");
    const auto flags = juce::FileBrowserComponent::openMode
                     | juce::FileBrowserComponent::canSelectFiles
                     | juce::FileBrowserComponent::canSelectMultipleItems
                     | juce::FileBrowserComponent::canSelectDirectories;
    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
    {
        const auto results = fc.getResults();
        if (results.isEmpty())
            return;

        juce::Array<juce::File> files;
        for (const auto& item : results)
        {
            if (item.isDirectory())
                files.addArray(item.findChildFiles(juce::File::findFiles, false, "*"));
            else
                files.add(item);
        }
        std::sort(files.begin(), files.end(),
                  [](const juce::File& a, const juce::File& b) { return a.getFileName() < b.getFileName(); });

        const int loaded = engine.loadStems(files);
        if (loaded == 0)
        {
            juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                "MixMentor", "No readable audio files were found.");
            return;
        }
        selectedIndex = -1;
        emptyLabel.setVisible(false);
        rebuildStrips();
        if (loaded >= AudioEngine::kMaxTracks)
            juce::NativeMessageBox::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                "MixMentor", "Loaded " + juce::String(AudioEngine::kMaxTracks) + " tracks (the maximum).");
    });
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

        const int index = strips.size();
        auto* strip = new ChannelStripComponent(b);
        strip->onSelect = [this, index] { selectChannel(index); };
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

    timeline.setOverview(engine.getOverview());
    selectChannel(session.numChannels() > 0 ? 0 : -1);
    resized();
}

void MainComponent::selectChannel(int index)
{
    selectedIndex = index;
    for (int i = 0; i < strips.size(); ++i)
        strips[i]->setSelected(i == index);

    engine.setAnalyzedChannel(index);

    auto& session = engine.getSession();
    if (index >= 0 && index < session.numChannels())
    {
        auto* ch = session.channels[static_cast<size_t>(index)].get();
        eqEditor.setChannel(ch, ch->colour, session.sampleRate);
        compEditor.setChannel(ch, ch->colour);

        // Surface an import warning, or the role hint, under the EQ title.
        if (ch->analysis.clipping)
            eqEditor.setReason("Note: this track peaks very hot - lower its fader or it may distort.");
        else if (ch->analysis.valid && ch->analysis.loudnessDb < -32.0f)
            eqEditor.setReason("Note: this track is quite quiet - you may need to raise its fader.");
        else
            eqEditor.setReason("Click Suggest for a starting EQ and compression for this " + toDisplayString(ch->role).toLowerCase() + ".");
    }
    else
    {
        eqEditor.setChannel(nullptr, juce::Colours::teal, session.sampleRate);
        compEditor.setChannel(nullptr, juce::Colours::teal);
    }
}

juce::String MainComponent::applyRecommendationToChannel(int index)
{
    auto& session = engine.getSession();
    if (index < 0 || index >= session.numChannels())
        return {};

    auto* ch = session.channels[static_cast<size_t>(index)].get();
    const Recommendation rec = recommend(ch->role, ch->analysis);

    ch->eqOn.store(rec.eq.eqOn);
    ch->hpOn.store(rec.eq.hpOn);
    ch->hpFreq.store(rec.eq.hpFreq);
    ch->bellFreq.store(rec.eq.bellFreq);
    ch->bellGainDb.store(rec.eq.bellGainDb);
    ch->bellQ.store(rec.eq.bellQ);
    ch->shelfFreq.store(rec.eq.shelfFreq);
    ch->shelfGainDb.store(rec.eq.shelfGainDb);

    ch->compOn.store(rec.comp.compOn);
    ch->compAutoGain.store(rec.comp.autoGain);
    ch->compThresholdDb.store(rec.comp.thresholdDb);
    ch->compRatio.store(rec.comp.ratio);
    ch->compAttackMs.store(rec.comp.attackMs);
    ch->compReleaseMs.store(rec.comp.releaseMs);
    ch->compMakeupDb.store(rec.comp.makeupDb);

    // Balance and panning. Same-role instruments alternate sides so, e.g., two
    // guitars spread left and right instead of stacking.
    ch->faderDb.store(rec.faderDb);
    int sameRoleBefore = 0;
    for (int j = 0; j < index; ++j)
        if (session.channels[static_cast<size_t>(j)]->role == ch->role)
            ++sameRoleBefore;
    const float side = (sameRoleBefore % 2 == 0) ? -1.0f : 1.0f;
    ch->pan.store(rec.panAmount * side);
    if (index < strips.size())
        strips[index]->refreshControls();

    return rec.reason;
}

void MainComponent::applyRecommendation(int index)
{
    const auto reason = applyRecommendationToChannel(index);
    if (reason.isNotEmpty())
    {
        eqEditor.refresh();
        compEditor.refresh();
        eqEditor.setReason(reason);
    }
}

void MainComponent::suggestAll()
{
    auto& session = engine.getSession();
    for (int i = 0; i < session.numChannels(); ++i)
        applyRecommendationToChannel(i);

    eqEditor.refresh();
    compEditor.refresh();
    eqEditor.setReason("Suggested a full starting mix for every track - play it, then refine from here.");
}

void MainComponent::setGuided(bool shouldBeGuided)
{
    guidedMode = shouldBeGuided;
    guidedButton.setToggleState(shouldBeGuided, juce::dontSendNotification);
    guidedPanel.setVisible(shouldBeGuided);
    if (shouldBeGuided)
    {
        stageIndex = 0;
        updateGuidedPanel();
    }
    resized();
    repaint();
}

void MainComponent::updateGuidedPanel()
{
    if (stages.empty())
        return;
    stageIndex = juce::jlimit(0, static_cast<int>(stages.size()) - 1, stageIndex);
    guidedPanel.setStage(stageIndex, static_cast<int>(stages.size()), stages[static_cast<size_t>(stageIndex)]);

    const auto region = stages[static_cast<size_t>(stageIndex)].region;
    if ((region == StageRegion::Eq || region == StageRegion::Compressor)
        && selectedIndex < 0 && engine.getSession().numChannels() > 0)
        selectChannel(0);

    repaint();
}

void MainComponent::advanceStage(int delta)
{
    const int next = stageIndex + delta;
    if (next >= static_cast<int>(stages.size()))
    {
        setGuided(false);   // finished the walkthrough
        return;
    }
    stageIndex = juce::jmax(0, next);
    updateGuidedPanel();
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        engine.togglePlay();
        return true;
    }
    if (key == juce::KeyPress::returnKey)
    {
        engine.stop();
        engine.seekSeconds(0.0);
        return true;
    }
    if (key == juce::KeyPress::leftKey)
    {
        engine.seekSeconds(engine.getPositionSeconds() - 5.0);
        return true;
    }
    if (key == juce::KeyPress::rightKey)
    {
        engine.seekSeconds(engine.getPositionSeconds() + 5.0);
        return true;
    }
    if (juce::CharacterFunctions::toLowerCase(key.getTextCharacter()) == 'l')
    {
        const bool loop = ! engine.isLooping();
        engine.setLooping(loop);
        loopButton.setToggleState(loop, juce::dontSendNotification);
        return true;
    }
    return false;
}

void MainComponent::timerCallback()
{
    if (! focusGrabbed && isShowing())
    {
        grabKeyboardFocus();
        focusGrabbed = true;
    }

    for (auto* strip : strips)
        strip->refreshMeter();
    if (masterStrip != nullptr)
        masterStrip->refreshMeter();

    if (engine.renderSpectrum(spectrumBuffer))
        eqEditor.setSpectrum(spectrumBuffer, engine.getSampleRate());
    compEditor.refreshMeter();

    const double lengthSeconds = engine.getLengthSeconds();
    timeline.setPosition(lengthSeconds > 0.0 ? engine.getPositionSeconds() / lengthSeconds : 0.0);
    positionLabel.setText(formatTime(engine.getPositionSeconds()) + " / "
                              + formatTime(lengthSeconds),
                          juce::dontSendNotification);
    playButton.setButtonText(engine.isPlaying() ? "Pause" : "Play");
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(20, 24, 30));

    auto area = getLocalBounds().reduced(16);
    g.setColour(juce::Colour::fromRGB(28, 33, 41));
    g.fillRoundedRectangle(area.toFloat(), 14.0f);

    // guided mode: highlight the region for the current stage
    if (guidedMode && ! stages.empty())
    {
        juce::Rectangle<int> region;
        switch (stages[static_cast<size_t>(stageIndex)].region)
        {
            case StageRegion::Strips:     region = stripsRegion; break;
            case StageRegion::Eq:         region = eqEditor.getBounds(); break;
            case StageRegion::Compressor: region = compEditor.getBounds(); break;
            case StageRegion::Master:     if (masterStrip != nullptr) region = masterStrip->getBounds(); break;
            case StageRegion::Export:     region = exportButton.getBounds(); break;
        }
        if (! region.isEmpty())
        {
            g.setColour(juce::Colour::fromRGB(232, 176, 75));
            g.drawRoundedRectangle(region.toFloat().expanded(3.0f), 9.0f, 2.5f);
        }
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(24);

    auto header = area.removeFromTop(40);
    titleLabel.setBounds(header.removeFromLeft(130));
    positionLabel.setBounds(header.removeFromRight(140));
    header.removeFromLeft(8);

    auto placeButton = [&header](juce::Button& b, int w)
    {
        b.setBounds(header.removeFromLeft(w).reduced(0, 4));
        header.removeFromLeft(6);
    };
    placeButton(openButton, 64);
    header.removeFromLeft(8);
    placeButton(playButton, 72);
    placeButton(stopButton, 60);
    placeButton(loopButton, 60);
    header.removeFromLeft(8);
    placeButton(exportButton, 72);
    placeButton(suggestAllButton, 96);
    placeButton(guidedButton, 74);
    placeButton(editorsButton, 74);

    area.removeFromTop(14);

    timeline.setBounds(area.removeFromTop(56));
    area.removeFromTop(12);

    if (guidedMode)
    {
        guidedPanel.setBounds(area.removeFromTop(66));
        area.removeFromTop(12);
    }

    emptyLabel.setBounds(area);

    auto console = area;

    if (masterStrip != nullptr)
    {
        masterStrip->setBounds(console.removeFromRight(96));
        console.removeFromRight(10);
    }

    // EQ + compressor editors occupy a right block when shown; hiding them frees
    // the full width for the channel strips (no horizontal scroll).
    if (editorsVisible)
    {
        auto editors = console.removeFromRight(640);
        console.removeFromRight(12);
        auto compArea = editors.removeFromBottom(210);
        editors.removeFromBottom(8);
        eqEditor.setBounds(editors);
        compEditor.setBounds(compArea);
    }

    // Strips are centered in the remaining area; their width adapts to the track
    // count (a little wider when the editors are hidden), so there's never an
    // awkward empty gap on one side.
    const int gap = 4;
    const int n = strips.size();
    stripsRegion = {};
    if (n > 0)
    {
        const int maxW = editorsVisible ? 96 : 120;
        const int stripW = juce::jlimit(44, maxW, (console.getWidth() - gap * (n - 1)) / n);
        const int blockW = stripW * n + gap * (n - 1);
        int x = console.getX() + juce::jmax(0, (console.getWidth() - blockW) / 2);
        for (auto* strip : strips)
        {
            const juce::Rectangle<int> bounds(x, console.getY(), stripW, console.getHeight());
            strip->setBounds(bounds);
            stripsRegion = stripsRegion.isEmpty() ? bounds : stripsRegion.getUnion(bounds);
            x += stripW + gap;
        }
    }
}
