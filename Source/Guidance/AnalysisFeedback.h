#pragma once

#include <vector>

#include <juce_core/juce_core.h>

#include "../Domain/Channel.h"
#include "../Domain/StemRole.h"

// Rule-based, deterministic assessment of a stem's *current* (processed) analysis.
// Turns raw numbers into plain-language feedback: what's good, what could be
// better. No AI - just mixing heuristics a beginner can learn from.

enum class NoteLevel { Good, Info, Warn };

struct AnalysisNote
{
    juce::String text;
    NoteLevel level { NoteLevel::Info };
};

// `cur` is the analysis of the processed signal (post EQ/comp/fader); `source`
// is the raw stem, used to describe what the processing changed.
inline std::vector<AnalysisNote> assessStem(StemRole role,
                                            const StemAnalysis& cur,
                                            const StemAnalysis& source)
{
    std::vector<AnalysisNote> notes;
    if (! cur.valid)
        return notes;

    const bool isBass  = (role == StemRole::Bass);
    const bool isVoice = (role == StemRole::LeadVocal || role == StemRole::BackingVocal);
    const bool wantsHighPass = ! isBass && role != StemRole::Drums;   // low end is mud on most non-bass parts

    // --- Level / headroom (most important, comes first) ---
    if (cur.clipping || cur.peakDb > -0.2f)
        notes.push_back({ "Clipping: peaks at/over 0 dBFS - pull this fader down or it will distort.",
                          NoteLevel::Warn });
    else if (cur.peakDb > -3.0f)
        notes.push_back({ "Very hot: little headroom left. Fine solo, but may push the master into clipping.",
                          NoteLevel::Info });
    else if (cur.loudnessDb < -30.0f)
        notes.push_back({ "Quiet: sits low in the mix - raise the fader if it's getting lost.",
                          NoteLevel::Info });

    // --- Dynamics (crest = peak - RMS) ---
    const bool compressed = source.valid && (source.crestDb - cur.crestDb) > 2.5f;
    if (cur.crestDb < 6.0f)
        notes.push_back({ "Squashed: very low dynamics - ease the compressor for a more natural sound.",
                          NoteLevel::Warn });
    else if (compressed)
        notes.push_back({ "Compressor tightened dynamics by "
                              + juce::String(source.crestDb - cur.crestDb, 1)
                              + " dB - steadier, more consistent level.",
                          NoteLevel::Good });
    else if (cur.crestDb > 20.0f && (role == StemRole::Drums || isVoice))
        notes.push_back({ "Very dynamic: a little compression would even out the loud and quiet moments.",
                          NoteLevel::Info });

    // --- Spectral balance ---
    if (isBass && cur.lowRatio < 0.4f)
        notes.push_back({ "Light on low end for a bass - check the high-pass isn't set too high.",
                          NoteLevel::Info });
    else if (wantsHighPass && cur.lowRatio > 0.45f)
        notes.push_back({ "Lots of low end for this part - a high-pass can clear mud and tighten the mix.",
                          NoteLevel::Info });
    else if (cur.highRatio > 0.55f && ! isVoice)
        notes.push_back({ "Bright: strong high end - great for air, but harsh if overdone.",
                          NoteLevel::Info });

    // Positive fallback so the panel never feels empty when things look healthy.
    if (notes.empty())
        notes.push_back({ "Balanced level and dynamics - this is sitting well.", NoteLevel::Good });

    // Keep it digestible.
    if (notes.size() > 3)
        notes.resize(3);
    return notes;
}
