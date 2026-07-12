#pragma once

#include <functional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

// A clickable timeline showing a waveform overview of the whole mix and the
// playhead. Click or drag anywhere to seek to that position.
class TimelineComponent final : public juce::Component
{
public:
    TimelineComponent() = default;

    void setOverview(const std::vector<float>& peaks) { overview = peaks; repaint(); }

    void setPosition(double fractionOfLength)
    {
        const double clamped = juce::jlimit(0.0, 1.0, fractionOfLength);
        if (! juce::approximatelyEqual(clamped, positionFraction))
        {
            positionFraction = clamped;
            repaint();
        }
    }

    std::function<void(double fraction)> onSeek;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(juce::Colour::fromRGB(22, 26, 32));
        g.fillRoundedRectangle(bounds, 6.0f);

        const float left = bounds.getX();
        const float right = bounds.getRight();
        const float midY = bounds.getCentreY();
        const float halfH = bounds.getHeight() * 0.44f;
        const float playX = left + bounds.getWidth() * (float) positionFraction;

        if (! overview.empty())
        {
            const int x0 = (int) left;
            const int x1 = (int) right;
            for (int x = x0; x < x1; ++x)
            {
                const float frac = (x - left) / bounds.getWidth();
                const int idx = juce::jlimit(0, (int) overview.size() - 1,
                                             (int) (frac * (float) overview.size()));
                const float amp = juce::jmax(1.0f, overview[(size_t) idx] * halfH);
                g.setColour(x <= (int) playX ? juce::Colour::fromRGB(120, 200, 175)
                                             : juce::Colour::fromRGB(70, 82, 96));
                g.drawVerticalLine(x, midY - amp, midY + amp);
            }
        }

        g.setColour(juce::Colour::fromRGB(240, 244, 250));
        g.drawVerticalLine((int) playX, bounds.getY(), bounds.getBottom());
    }

    void mouseDown(const juce::MouseEvent& e) override { seekFromX(e.x); }
    void mouseDrag(const juce::MouseEvent& e) override { seekFromX(e.x); }

private:
    void seekFromX(int x)
    {
        if (onSeek != nullptr && getWidth() > 0)
            onSeek(juce::jlimit(0.0f, 1.0f, (float) x / (float) getWidth()));
    }

    std::vector<float> overview;
    double positionFraction { 0.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};
