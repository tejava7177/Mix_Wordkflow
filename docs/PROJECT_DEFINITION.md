# Project Definition

## What MixMentor Is

MixMentor is a desktop **educational mixing studio**. It teaches audio mixing by
guiding the user through the real mixing process — from gain staging to the master
bus — with plain-language explanations, live visual feedback, and ready-made
templates. It is a genuine mixer with real DSP, organized around *learning* rather
than around a professional's assumed expertise.

## Why It Exists

Mixing has a steep on-ramp. Full DAWs present hundreds of controls with no guidance
on what to do, in what order, or why. Beginners either give up or copy settings
without understanding them.

MixMentor reframes the tool around the *stages of mixing*. Instead of "here is a
blank console, good luck," it says: "start by setting levels — here's what that
means and why it matters," and only surfaces the controls that matter at each step.
The learner produces a better mix *and* understands how they got there.

## Target Users

- People learning to mix for the first time
- Hobbyist musicians and bedroom producers
- Students of audio / music production
- Podcasters and content creators who want a cleaner result without a DAW

## Product Position

MixMentor **is**:

- an educational, guided mixing application with real audio processing
- a template-driven starting point for common mixing tasks
- a tool that explains *what* each control does and *why*

MixMentor **is not**:

- a full professional DAW or unlimited-track editor
- an automatic "one-click AI mix" product
- a toy — the processing and results are real

## The Teaching Model

MixMentor is built on three pillars, wrapped around the eight-stage mixing process
documented in [MIXING_STAGES.md](MIXING_STAGES.md):

1. **Guided mode** — the stages are presented in a recommended order. At each stage
   the relevant controls are in focus and the rest recede, so the user is never
   overwhelmed. Guided mode is the default; it can be turned off for free mixing.
2. **Templates** — per-instrument and whole-mix starting chains (e.g. "vocal
   starter", "drum bus", "balanced mix"). The user hears a good result immediately
   and learns by inspecting and tweaking the template.
3. **Explain-as-you-go** — every control carries a short plain-language description
   of what it does and when to use it, paired with live visual feedback (spectrum,
   gain-reduction meter, level meters) so the effect is visible, not abstract.

## What You Can Do (Feature List)

### Mix

- Load a multitrack session (audio stems) and play all tracks in sync
- Set per-channel gain, fader, pan, mute, and solo
- Watch per-channel and master level meters
- Shape each channel with EQ and compression, with visual feedback
- Balance to the master bus and monitor loudness
- Export the finished mix to a WAV file

### Learn

- Follow the guided stage-by-stage workflow
- Read plain-language help on every control
- Apply a template and inspect how it is built
- Practice on a bundled demo session with zero setup

## MVP Intent

The first milestone proves the core promise: a beginner opens the bundled demo
session, follows the guided stages (gain, balance, pan, EQ, compression, master),
understands each step, and exports a mix that sounds clearly better than the raw
stems. See [MVP_SCOPE.md](MVP_SCOPE.md).
