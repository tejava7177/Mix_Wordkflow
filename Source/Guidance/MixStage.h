#pragma once

#include <vector>

#include <juce_core/juce_core.h>

// The area of the console a guided stage focuses on.
enum class StageRegion { Strips, Eq, Compressor, Master, Export };

// One step in the guided mixing walkthrough: a title, a plain-language
// instruction, and the region of the UI it draws attention to.
struct MixStage
{
    juce::String title;
    juce::String instruction;
    StageRegion region;
};

inline std::vector<MixStage> makeMixStages()
{
    return {
        { "Balance",
          "Set each track's volume with its fader until the song sounds balanced. "
          "Start with the most important part - often the drums or the vocal.",
          StageRegion::Strips },
        { "Panning",
          "Place tracks across the stereo field. Keep bass and lead parts centered; "
          "spread guitars and keys left and right for width.",
          StageRegion::Strips },
        { "EQ",
          "Shape each track's tone. Click a track, then press Suggest for a smart "
          "starting EQ and compression - or set it yourself; the spectrum shows "
          "where the energy sits.",
          StageRegion::Eq },
        { "Compression",
          "Even out a track's dynamics. Turn the compressor on and lower the "
          "threshold; keep Auto gain on and watch the gain-reduction meter.",
          StageRegion::Compressor },
        { "Master",
          "Check the overall level on the Master strip. Aim for a healthy level "
          "without the meter reaching the very top.",
          StageRegion::Master },
        { "Export",
          "You're done - nice work! Click Export in the top bar to save your "
          "finished mix as a WAV file.",
          StageRegion::Export },
    };
}
