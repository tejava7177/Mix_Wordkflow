#pragma once

#include <juce_core/juce_core.h>

#include "../Domain/Channel.h"
#include "../Domain/StemRole.h"

// A suggested starting point for a track, with a plain-language reason.
struct Recommendation
{
    EqValues eq;
    CompValues comp;
    juce::String reason;
};

// Deterministic, rule-based recommendation from a stem's role and analysis.
// This is the "does the knowing for you" part - expert rules, not AI.
inline Recommendation recommend(StemRole role, const StemAnalysis& a)
{
    Recommendation r;
    juce::StringArray reasons;

    const bool isBass = role == StemRole::Bass;
    const bool isDrums = role == StemRole::Drums;
    const bool isVocal = role == StemRole::LeadVocal || role == StemRole::BackingVocal;

    // High-pass: keep sub-lows only where they belong (bass, kick).
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

    // Reduce mud if there's a low-mid buildup (and it's not the bass).
    if (a.valid && ! isBass && a.lowRatio > 0.5f)
    {
        r.eq.bellFreq = 350.0f;
        r.eq.bellGainDb = -3.0f;
        r.eq.bellQ = 1.0f;
        reasons.add("cut around 350 Hz to reduce muddiness");
    }

    // Add air if the track is dull and would benefit from clarity.
    if (a.valid && a.highRatio < 0.15f && (isVocal || role == StemRole::Guitar || role == StemRole::Keys))
    {
        r.eq.shelfFreq = 8000.0f;
        r.eq.shelfGainDb = 3.0f;
        reasons.add("added a high-shelf lift for air and clarity");
    }

    // Compressor: role-appropriate starting point, auto-gain on.
    r.comp.compOn = true;
    r.comp.autoGain = true;
    switch (role)
    {
        case StemRole::LeadVocal:
        case StemRole::BackingVocal: r.comp = { true, true, -20.0f, 3.0f, 8.0f, 120.0f, 0.0f }; break;
        case StemRole::Bass:         r.comp = { true, true, -18.0f, 3.0f, 15.0f, 100.0f, 0.0f }; break;
        case StemRole::Drums:        r.comp = { true, true, -16.0f, 4.0f, 5.0f, 80.0f, 0.0f }; break;
        default:                     r.comp = { true, true, -18.0f, 2.5f, 15.0f, 150.0f, 0.0f }; break;
    }

    if (a.valid && a.crestDb > 18.0f)
    {
        r.comp.ratio += 1.0f;
        reasons.add("a bit more compression to tame wide dynamics");
    }
    else
    {
        reasons.add("gentle compression to keep it steady");
    }

    r.reason = reasons.isEmpty() ? "Applied a clean starting point."
                                 : "Suggested: " + reasons.joinIntoString("; ") + ".";
    return r;
}
