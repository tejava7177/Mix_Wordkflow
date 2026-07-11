#pragma once

#include <juce_core/juce_core.h>

// The role a stem plays in the mix. Roles are the taxonomy that lets MixMentor
// cover arbitrary user material: any stem is reduced to a known role with
// role-appropriate handling and templates.
enum class StemRole
{
    Drums,
    Bass,
    LeadVocal,
    BackingVocal,
    Guitar,
    Keys,
    Synth,
    Fx,
    Other
};

inline juce::StringArray stemRoleNames()
{
    return { "Drums", "Bass", "Lead vocal", "Backing vocal",
             "Guitar", "Keys", "Synth", "FX", "Other" };
}

inline juce::String toDisplayString(StemRole role)
{
    return stemRoleNames()[static_cast<int>(role)];
}

// A rough default role guess from a file name, used as the pre-filled suggestion
// the user can confirm or change.
inline StemRole guessRoleFromName(const juce::String& rawName)
{
    auto name = rawName.toLowerCase();
    if (name.contains("drum") || name.contains("kick") || name.contains("snare")
        || name.contains("hat") || name.contains("clap") || name.contains("perc"))
        return StemRole::Drums;
    if (name.contains("bass") || name.contains("sub") || name.contains("808"))
        return StemRole::Bass;
    if (name.contains("lead vox") || name.contains("lead vocal") || name.contains("lead voc"))
        return StemRole::LeadVocal;
    if (name.contains("back") && name.contains("vo"))
        return StemRole::BackingVocal;
    if (name.contains("vocal") || name.contains("vox") || name.contains("voice"))
        return StemRole::LeadVocal;
    if (name.contains("guitar") || name.contains("gtr"))
        return StemRole::Guitar;
    if (name.contains("piano") || name.contains("keys") || name.contains("organ")
        || name.contains("rhodes"))
        return StemRole::Keys;
    if (name.contains("synth") || name.contains("pad") || name.contains("lead"))
        return StemRole::Synth;
    if (name.contains("fx") || name.contains("return") || name.contains("reverb"))
        return StemRole::Fx;
    return StemRole::Other;
}
