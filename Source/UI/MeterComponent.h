#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>

// A simple stereo peak meter with decay ballistics. Levels are pushed in from a
// timer (linear peak, 0..1+); the meter maps them to a dB scale for display.
class MeterComponent final : public juce::Component
{
public:
    MeterComponent() = default;

    void setLevels(float peakL, float peakR)
    {
        displayL = juce::jmax(peakL, displayL * decay);
        displayR = juce::jmax(peakR, displayR * decay);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float gap = 3.0f;
        const float barW = (bounds.getWidth() - gap) * 0.5f;
        drawBar(g, bounds.removeFromLeft(barW), displayL);
        bounds.removeFromLeft(gap);
        drawBar(g, bounds, displayR);
    }

private:
    static float levelToNorm(float linearPeak)
    {
        const float db = juce::Decibels::gainToDecibels(linearPeak, -60.0f);
        return juce::jlimit(0.0f, 1.0f, juce::jmap(db, -60.0f, 6.0f, 0.0f, 1.0f));
    }

    void drawBar(juce::Graphics& g, juce::Rectangle<float> area, float linearPeak)
    {
        g.setColour(juce::Colour::fromRGB(24, 28, 34));
        g.fillRoundedRectangle(area, 2.0f);

        const float norm = levelToNorm(linearPeak);
        if (norm <= 0.0f)
            return;

        auto filled = area.withTrimmedTop(area.getHeight() * (1.0f - norm));
        // colour by how hot it is
        juce::Colour colour = juce::Colour::fromRGB(93, 202, 165);          // green
        if (linearPeak > juce::Decibels::decibelsToGain(-3.0f))
            colour = juce::Colour::fromRGB(226, 75, 74);                    // red
        else if (linearPeak > juce::Decibels::decibelsToGain(-12.0f))
            colour = juce::Colour::fromRGB(239, 159, 39);                   // amber

        g.setColour(colour);
        g.fillRoundedRectangle(filled, 2.0f);
    }

    float displayL { 0.0f };
    float displayR { 0.0f };
    static constexpr float decay = 0.85f;
};
