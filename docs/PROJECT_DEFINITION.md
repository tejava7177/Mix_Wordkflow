# Project Definition

## Goal (North Star)

**Let someone who knows nothing about mixing produce a release-ready mix, just by
using MixMentor.** Education is the vehicle; a finished, release-quality song is the
outcome. Every design decision serves this goal.

## What MixMentor Is

MixMentor is a desktop **guided mixing studio** that takes a beginner from raw stems
to a release-ready mix. The user picks the song's style, labels each stem's role, and
MixMentor analyzes the audio, pre-fills sensible starting settings, and walks the user
through the mixing process one stage at a time — explaining each step and letting them
refine with confidence. It is a real mixer with real DSP, deliberately shaped as **one
well-paved path** rather than an open-ended toolbox.

## Why It Exists

Mixing has a steep on-ramp. Full DAWs present hundreds of controls and thousands of
plugin choices with no guidance on what to do, in what order, or why. Beginners either
give up or copy settings blindly. MixMentor removes the two things that block a
novice: not knowing *what to do*, and not knowing *which tool and setting to use*. It
supplies both — genre-aware targets, role-appropriate processing, analysis-driven
defaults, and a guided order — so a first-timer can reach a professional result.

## Target Users

- People who want a good-sounding mix but don't know how to mix
- Hobbyist musicians, bedroom producers, singer-songwriters
- Podcasters and content creators who need a clean, release-ready result

## Product Position

MixMentor **is**:

- a guided path that takes a beginner to a release-ready mix
- genre-aware and analysis-assisted (it measures the audio and pre-fills good defaults)
- built on a single, curated, high-quality toolset with plain-language guidance

MixMentor **is not**:

- a full DAW (no recording, MIDI, arrangement timeline, or unlimited tracks)
- a third-party plugin host (VST/AU) — it ships its own curated processors
- an AI / machine-learning auto-mixer — its intelligence is deterministic DSP and
  expert rules, so it is explainable and teachable
- an open-ended toolbox — simplicity and a single good path are the point

## How It Gets a Beginner to a Release-Ready Mix

1. **Genre / style profile** — the user picks the song's style; this sets targets
   (tonal balance, loudness, low-end, width) and default moves appropriate to the genre.
2. **Stem-role labeling** — the user tags each stem by role (drums, bass, lead vocal,
   guitar, keys, …), with a DSP-suggested guess. A beginner can identify a role even
   without mixing knowledge, and the role tells MixMentor how to treat that track.
3. **Import analysis (DSP, not AI)** — each stem is measured (loudness, peak, crest
   factor, spectral balance, clipping / DC / phase). Results drive smart starting
   settings and clear warnings.
4. **Guided stages** — the mixing process (gain → balance → pan → EQ → compression →
   master) is presented in order. Each stage arrives with an analysis-, genre-, and
   role-informed **recommendation already in place**; the user understands it and
   refines rather than starting from a blank page.
5. **Curated toolset + templates** — one well-designed processor per job and
   role-based starting chains, so the user never has to choose between competing tools.
6. **Explain-as-you-go** — every control says what it does and when to use it, with
   live visual feedback (spectrum, gain-reduction, meters).

## What You Can Do (Feature List)

- Load a multitrack session (audio stems) and play all tracks in sync
- Pick a genre/style that sets mixing targets
- Label each stem's role; get analysis-based warnings and smart defaults
- Set per-channel gain, fader, pan, mute, solo; watch per-channel and master meters
- Shape each channel with our own EQ and compressor, with visual feedback
- Follow the guided stages with recommendations pre-filled
- Apply role/genre templates and inspect how they are built
- Compare the raw import against the finished mix (A/B)
- Export a release-ready mix to WAV
- Practice on a bundled demo session with zero setup

## MVP Intent

The first milestone proves the core promise on a bundled demo session: a beginner
picks a style, labels the stems, follows the guided stages with recommendations
pre-filled, and exports a mix that sounds clearly release-worthy — understanding each
step. See [MVP_SCOPE.md](MVP_SCOPE.md).
