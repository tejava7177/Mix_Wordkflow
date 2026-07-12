#pragma once

#include <utility>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../Domain/Channel.h"
#include "../Guidance/AnalysisFeedback.h"

// Read-only panel that shows the measured DSP analysis of the selected stem, so
// the user can verify the raw numbers the recommendation engine works from
// (level, dynamics, spectral balance, clipping). No reasoning yet - just facts.
class AnalysisPanelComponent final : public juce::Component
{
public:
    AnalysisPanelComponent() = default;

    void setAnalysis(const StemAnalysis& a, const juce::String& name,
                     const juce::String& role, juce::Colour c)
    {
        analysis = a;
        stemName = name;
        roleText = role;
        colour = c;
        hasStem = true;
        repaint();
    }

    void setNotes(std::vector<AnalysisNote> newNotes)
    {
        notes = std::move(newNotes);
        repaint();
    }

    void clear()
    {
        hasStem = false;
        notes.clear();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto full = getLocalBounds().toFloat();
        g.setColour(juce::Colour::fromRGB(24, 29, 37));
        g.fillRoundedRectangle(full, 8.0f);

        auto area = getLocalBounds().reduced(16, 12);

        // Title (build with String on the left so the em-dash decodes as UTF-8)
        juce::String title("Analysis");
        if (hasStem)
            title << "  \xe2\x80\x94  " << stemName;
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.drawText(title, area.removeFromTop(18), juce::Justification::centredLeft);

        if (! hasStem || ! analysis.valid)
        {
            g.setColour(juce::Colour::fromRGB(150, 162, 176));
            g.setFont(juce::FontOptions(12.0f));
            g.drawText(hasStem ? "No analysis available for this stem."
                               : "Select a track to see its measured analysis.",
                       area, juce::Justification::centredLeft);
            return;
        }

        // Role subtitle
        g.setColour(colour);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(roleText.toUpperCase(), area.removeFromTop(15), juce::Justification::centredLeft);
        area.removeFromTop(4);

        // Four headline metrics, side by side.
        auto metrics = area.removeFromTop(40);
        const int cellW = metrics.getWidth() / 4;
        auto drawMetric = [&g](juce::Rectangle<int> cell, const juce::String& label,
                               const juce::String& value, juce::Colour valueColour)
        {
            g.setColour(juce::Colour::fromRGB(140, 152, 166));
            g.setFont(juce::FontOptions(10.5f));
            g.drawText(label, cell.removeFromTop(15), juce::Justification::centredLeft);
            g.setColour(valueColour);
            g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
            g.drawText(value, cell, juce::Justification::centredLeft);
        };
        drawMetric(metrics.removeFromLeft(cellW), "Loudness (RMS)",
                   juce::String(analysis.loudnessDb, 1) + " dB", juce::Colours::white);
        drawMetric(metrics.removeFromLeft(cellW), "Peak",
                   juce::String(analysis.peakDb, 1) + " dB", juce::Colours::white);
        drawMetric(metrics.removeFromLeft(cellW), "Dynamics (crest)",
                   juce::String(analysis.crestDb, 1) + " dB", juce::Colours::white);
        drawMetric(metrics.removeFromLeft(cellW), "Clipping",
                   analysis.clipping ? "Yes" : "No",
                   analysis.clipping ? juce::Colour::fromRGB(224, 96, 96) : juce::Colours::white);

        area.removeFromTop(8);

        // Spectral balance: a 3-segment bar (low / mid / high) plus a legend.
        g.setColour(juce::Colour::fromRGB(140, 152, 166));
        g.setFont(juce::FontOptions(10.5f));
        g.drawText("Spectral balance", area.removeFromTop(14), juce::Justification::centredLeft);
        area.removeFromTop(2);

        const auto lowC = juce::Colour::fromRGB(90, 130, 200);
        const auto midC = juce::Colour::fromRGB(90, 180, 140);
        const auto highC = juce::Colour::fromRGB(214, 170, 84);

        auto bar = area.removeFromTop(14);
        const float total = juce::jmax(0.0001f, analysis.lowRatio + analysis.midRatio + analysis.highRatio);
        const int w = bar.getWidth();
        const int lowW = juce::roundToInt(w * analysis.lowRatio / total);
        const int midW = juce::roundToInt(w * analysis.midRatio / total);
        const int highW = w - lowW - midW;
        auto seg = bar;
        g.setColour(lowC);  g.fillRect(seg.removeFromLeft(lowW));
        g.setColour(midC);  g.fillRect(seg.removeFromLeft(midW));
        g.setColour(highC); g.fillRect(seg.removeFromLeft(highW));

        area.removeFromTop(4);
        auto legend = area.removeFromTop(15);
        auto pct = [](float r) { return juce::String(juce::roundToInt(r * 100.0f)) + "%"; };
        const int third = legend.getWidth() / 3;
        g.setFont(juce::FontOptions(10.5f));
        g.setColour(lowC);
        g.drawText("Low <250Hz  " + pct(analysis.lowRatio),
                   legend.removeFromLeft(third), juce::Justification::centredLeft);
        g.setColour(midC);
        g.drawText("Mid 250Hz-4kHz  " + pct(analysis.midRatio),
                   legend.removeFromLeft(third), juce::Justification::centredLeft);
        g.setColour(highC);
        g.drawText("High >4kHz  " + pct(analysis.highRatio),
                   legend, juce::Justification::centredLeft);

        // Feedback: plain-language assessment of the current processed sound.
        if (notes.empty())
            return;
        area.removeFromTop(8);
        g.setColour(juce::Colour::fromRGB(58, 66, 78));
        g.fillRect(area.removeFromTop(1));
        area.removeFromTop(5);
        g.setColour(juce::Colour::fromRGB(140, 152, 166));
        g.setFont(juce::FontOptions(10.5f));
        g.drawText("Feedback", area.removeFromTop(14), juce::Justification::centredLeft);
        area.removeFromTop(1);

        for (const auto& note : notes)
        {
            auto row = area.removeFromTop(18);
            if (row.getHeight() < 12)
                break;
            auto dot = row.removeFromLeft(15);
            const juce::Colour c = note.level == NoteLevel::Good ? juce::Colour::fromRGB(96, 194, 132)
                                 : note.level == NoteLevel::Warn ? juce::Colour::fromRGB(226, 148, 84)
                                                                 : juce::Colour::fromRGB(120, 158, 210);
            g.setColour(c);
            g.fillEllipse((float) dot.getX() + 1.0f, (float) dot.getCentreY() - 3.0f, 6.0f, 6.0f);
            g.setColour(juce::Colour::fromRGB(206, 214, 224));
            g.setFont(juce::FontOptions(11.5f));
            g.drawFittedText(note.text, row, juce::Justification::centredLeft, 1);
        }
    }

private:
    StemAnalysis analysis;
    juce::String stemName;
    juce::String roleText;
    juce::Colour colour { juce::Colours::grey };
    bool hasStem { false };
    std::vector<AnalysisNote> notes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalysisPanelComponent)
};
