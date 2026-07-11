#pragma once

#include <atomic>

#include <juce_gui_basics/juce_gui_basics.h>

#include "MeterComponent.h"

// A vertical mixer strip bound to a set of atomic parameters. Works for both a
// channel (with pan, mute, solo) and the master bus (fader + meter only) by
// leaving the optional bindings null.
class ChannelStripComponent final : public juce::Component
{
public:
    struct Binding
    {
        juce::String name;
        juce::String roleText;
        juce::Colour colour { juce::Colours::grey };
        std::atomic<float>* faderDb { nullptr };
        std::atomic<float>* pan { nullptr };    // null => no pan control (master)
        std::atomic<bool>*  mute { nullptr };   // null => no mute
        std::atomic<bool>*  solo { nullptr };   // null => no solo
        std::atomic<float>* meterL { nullptr };
        std::atomic<float>* meterR { nullptr };
    };

    explicit ChannelStripComponent(Binding bindingToUse);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Called from the UI timer to refresh the meter.
    void refreshMeter();

private:
    void updateDbLabel();

    Binding binding;

    juce::Label nameLabel;
    juce::Label roleLabel;
    juce::Slider panSlider;
    juce::TextButton soloButton { "S" };
    juce::TextButton muteButton { "M" };
    juce::Slider faderSlider;
    juce::Label dbLabel;
    MeterComponent meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStripComponent)
};
