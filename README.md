# MixMentor

Learn to mix by doing it — a guided, template-based mixing studio for beginners.

## What it is

MixMentor is a JUCE desktop application that teaches audio mixing by walking you
through the real mixing process, one stage at a time. You load a multitrack
session, and MixMentor guides you from gain staging to the master bus — explaining
each step in plain language, showing the effect visually, and offering ready-made
templates so you always have a good-sounding starting point.

It is a real mixer with real DSP, wrapped in a learning-first workflow.

## Who it's for

- People learning to mix for the first time
- Hobbyist musicians and bedroom producers
- Students, podcasters, and content creators who want a better-sounding result
  without a steep DAW learning curve

## The idea

Most mixing tools assume you already know what you're doing. MixMentor inverts that:
the interface is organized around the *stages of mixing* and teaches as you go.

```
Gain staging → Balance → Panning → EQ → Compression → (Reverb/Delay) → (Automation) → Master
```

Three things make it beginner-friendly:

- **Guided mode** — follow the stages in a recommended order; only the controls that
  matter right now are in focus.
- **Templates** — per-instrument and whole-mix starting chains, so you hear a good
  result immediately and learn by inspecting it.
- **Explain-as-you-go** — every control says what it does and when to use it, with
  live visual feedback (spectrum, gain-reduction, meters).

## Key features (target)

- Load a multitrack session (audio stems) and play them in sync
- Per-channel strip: gain, fader, pan, mute/solo, level meter
- Per-channel EQ and compression with visual feedback
- Master bus with metering (peak/RMS, loudness)
- Guided stage-by-stage workflow with plain-language help
- Starter templates for common instruments and full mixes
- Export the finished mix to WAV
- A bundled demo session so you can start with zero setup

## Tech stack

- C++17
- JUCE (audio engine, DSP, UI)
- CMake

## Build

JUCE is not vendored in this repository.

```bash
cd MixMentor
cmake -S . -B build -DJUCE_SOURCE_DIR=/path/to/JUCE
cmake --build build
```

If JUCE is missing, CMake stops with a clear error describing the expected setup.

## Documentation

- [docs/PROJECT_DEFINITION.md](docs/PROJECT_DEFINITION.md) — what it is, who it's for, feature list
- [docs/MIXING_STAGES.md](docs/MIXING_STAGES.md) — the mixing process the app teaches (domain reference)
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — audio engine, DSP, guidance/template design
- [docs/WORKFLOW.md](docs/WORKFLOW.md) — guided and free workflows
- [docs/MVP_SCOPE.md](docs/MVP_SCOPE.md) — first milestone scope
- [docs/DEVELOPMENT_ROADMAP.md](docs/DEVELOPMENT_ROADMAP.md) — phased plan

## Status

New project — planning complete, implementation starting with the audible
multitrack mix engine. The application name is a working title and may change.
