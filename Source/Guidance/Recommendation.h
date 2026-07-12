#pragma once

#include <cmath>

#include <juce_core/juce_core.h>

#include "../Domain/Channel.h"
#include "../Domain/StemRole.h"

// A suggested starting point for a track, with a plain-language reason.
struct Recommendation
{
    EqValues eq;
    CompValues comp;
    float faderDb { 0.0f };   // level toward a role-appropriate target
    float panAmount { 0.0f }; // magnitude 0..1; the caller picks the side
    juce::String reason;
};

// Conservative per-track target loudness (dBFS RMS). Deliberately low so that
// several tracks sum with headroom instead of clipping the master.
inline float roleTargetLoudnessDb(StemRole role)
{
    switch (role)
    {
        case StemRole::Drums:        return -25.0f;
        case StemRole::Bass:         return -25.0f;
        case StemRole::LeadVocal:    return -24.0f;
        case StemRole::Guitar:       return -28.0f;
        case StemRole::Keys:         return -29.0f;
        case StemRole::Synth:        return -29.0f;
        case StemRole::BackingVocal: return -30.0f;
        case StemRole::Fx:           return -32.0f;
        default:                     return -28.0f;
    }
}

inline float rolePanAmount(StemRole role)
{
    switch (role)
    {
        case StemRole::Bass:
        case StemRole::Drums:
        case StemRole::LeadVocal:    return 0.0f;
        case StemRole::BackingVocal: return 0.5f;
        case StemRole::Fx:           return 0.4f;
        case StemRole::Other:        return 0.2f;
        default:                     return 0.35f;  // guitar, keys, synth
    }
}

// Deterministic, rule-based recommendation from a stem's role and analysis.
inline Recommendation recommend(StemRole role, const StemAnalysis& a)
{
    Recommendation r;
    juce::StringArray reasons;

    const bool isBass = role == StemRole::Bass;
    const bool isDrums = role == StemRole::Drums;
    const bool isVocal = role == StemRole::LeadVocal || role == StemRole::BackingVocal;

    // Compressor first (its makeup affects the level we then set).
    switch (role)
    {
        case StemRole::LeadVocal:
        case StemRole::BackingVocal: r.comp = { true, true, -20.0f, 3.0f, 8.0f, 120.0f, 0.0f }; break;
        case StemRole::Bass:         r.comp = { true, true, -18.0f, 3.0f, 15.0f, 100.0f, 0.0f }; break;
        case StemRole::Drums:        r.comp = { true, true, -16.0f, 4.0f, 5.0f, 80.0f, 0.0f }; break;
        default:                     r.comp = { true, true, -18.0f, 2.5f, 15.0f, 150.0f, 0.0f }; break;
    }
    const bool veryDynamic = a.valid && a.crestDb > 18.0f;
    if (veryDynamic)
        r.comp.ratio += 1.0f;

    // Auto-gain makeup the compressor will add (so the fader doesn't double-boost).
    const float slope = 1.0f - 1.0f / juce::jmax(1.0f, r.comp.ratio);
    const float autoMakeupDb = r.comp.autoGain ? (0.0f - r.comp.thresholdDb) * slope * 0.6f : 0.0f;

    // Balance: bring the post-compressor level toward a conservative target.
    if (a.valid)
    {
        r.faderDb = juce::jlimit(-24.0f, 12.0f, roleTargetLoudnessDb(role) - a.loudnessDb - autoMakeupDb);
        if (std::abs(r.faderDb) > 1.0f)
            reasons.add(r.faderDb > 0.0f ? "set its level to sit in the mix"
                                         : "eased its level to sit in the mix");
    }

    // Panning.
    r.panAmount = rolePanAmount(role);
    if (r.panAmount > 0.0f)
        reasons.add("panned to the side for width");

    // High-pass.
    r.eq.eqOn = true;
    if (isBass)
    {
        r.eq.hpOn = true;
        r.eq.hpFreq = 30.0f;
    }
    else if (! isDrums)
    {
        r.eq.hpOn = true;
        r.eq.hpFreq = isVocal ? 100.0f : 80.0f;
        reasons.add("high-passed at " + juce::String((int) r.eq.hpFreq) + " Hz to clear low rumble");
    }

    // Mud cut.
    if (a.valid && ! isBass && a.lowRatio > 0.5f)
    {
        r.eq.bellFreq = 350.0f;
        r.eq.bellGainDb = -3.0f;
        r.eq.bellQ = 1.0f;
        reasons.add("cut around 350 Hz to reduce muddiness");
    }

    // Air lift for dull tracks.
    if (a.valid && a.highRatio < 0.15f && (isVocal || role == StemRole::Guitar || role == StemRole::Keys))
    {
        r.eq.shelfFreq = 8000.0f;
        r.eq.shelfGainDb = 3.0f;
        reasons.add("added a high-shelf lift for air");
    }

    reasons.add(veryDynamic ? "a bit more compression to tame wide dynamics"
                            : "gentle compression to keep it steady");

    r.reason = reasons.isEmpty() ? "Applied a clean starting point."
                                 : "Suggested: " + reasons.joinIntoString("; ") + ".";
    return r;
}

// Educational one-liners that tie the measured analysis to the chosen settings,
// so the user learns *why* each suggestion was made (not just what changed).

inline juce::String explainEq(StemRole role, const StemAnalysis& a, const EqValues& eq)
{
    auto pct = [](float r) { return juce::String(juce::roundToInt(r * 100.0f)) + "%"; };

    juce::StringArray moves;
    if (eq.hpOn)
        moves.add("high-pass " + juce::String((int) eq.hpFreq) + " Hz");
    if (eq.bellGainDb < -0.5f)
        moves.add(juce::String(eq.bellGainDb, 1) + " dB dip at " + juce::String((int) eq.bellFreq) + " Hz");
    if (eq.shelfGainDb > 0.5f)
        moves.add("+" + juce::String(eq.shelfGainDb, 1) + " dB shelf at " + juce::String((int) eq.shelfFreq) + " Hz");

    juce::String why;
    if (! a.valid)
        why = "Role-based starting EQ: ";
    else if (role == StemRole::Bass)
        why = "Low energy " + pct(a.lowRatio) + " (bass body): ";
    else if (eq.bellGainDb < -0.5f)
        why = "Lows " + pct(a.lowRatio) + " (muddy): ";
    else if (eq.shelfGainDb > 0.5f)
        why = "Highs only " + pct(a.highRatio) + " (dull): ";
    else
        why = "Balance L/M/H " + pct(a.lowRatio) + " / " + pct(a.midRatio) + " / " + pct(a.highRatio) + ": ";

    if (moves.isEmpty())
        return why + "tone already even, so no cut or boost.";
    return why + moves.joinIntoString(", ") + ".";
}

inline juce::String explainComp(StemRole role, const StemAnalysis& a, const CompValues& comp)
{
    juce::ignoreUnused(role);
    if (! comp.compOn)
        return "Compressor off - this part sits fine without it.";

    const juce::String settings = juce::String(comp.ratio, 1) + ":1 at "
        + juce::String((int) comp.thresholdDb) + " dB, attack " + juce::String((int) comp.attackMs) + " ms";

    juce::String why;
    if (a.valid)
        why = "Crest " + juce::String(a.crestDb, 1) + " dB "
            + (a.crestDb > 18.0f ? "(very dynamic): " : "(fairly steady): ");
    else
        why = "Role-based control: ";

    const juce::String tail = comp.autoGain ? "; auto-gain keeps the level matched." : ".";
    return why + settings + tail;
}
