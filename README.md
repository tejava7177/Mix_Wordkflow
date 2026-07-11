# MixMentor

Take a song from raw stems to a release-ready mix — even if you've never mixed
before. A guided mixing studio that does the knowing for you.

## What it is

MixMentor is a JUCE desktop application with one goal: **let someone who knows
nothing about mixing produce a release-ready mix.** You load your stems, pick the
song's style, and tell it what each track is (drums, bass, vocal, …). MixMentor
analyzes the audio, pre-fills sensible settings, and walks you through the mixing
process one stage at a time — explaining each step and letting you refine with
confidence.

It is a real mixer with real DSP, deliberately shaped as **one well-paved path**
rather than an open-ended toolbox.

## Who it's for

- People who want a good-sounding mix but don't know how to mix
- Hobbyist musicians, bedroom producers, singer-songwriters
- Podcasters and content creators who need a clean, release-ready result

## The idea

Most mixing tools assume you already know what you're doing and hand you thousands of
plugin choices. MixMentor removes both blockers — not knowing *what to do*, and not
knowing *which tool and setting to use*:

```
Pick a genre + label your stems → analyze → guided stages, each pre-filled with a
recommendation you understand and refine → export a release-ready mix
```

What makes a beginner succeed:

- **Genre + roles** — the style sets the targets; labeling each stem's role tells the
  app how to treat it. You cover any material without needing mixing knowledge.
- **Analysis + smart defaults** — it measures your stems (loudness, spectrum, clipping
  …) and pre-fills good starting settings with plain-language reasons. Deterministic
  DSP and expert rules — no AI.
- **One curated toolset** — our own EQ and compressor, one good option per job, so you
  never choose between competing plugins.
- **Explain-as-you-go** — every control says what it does, with live visual feedback.

## Key features (target)

- Load a multitrack session (audio stems) and play them in sync
- Pick a genre/style that sets mixing targets; label each stem's role
- Import analysis (loudness, peak, spectrum, clipping) with warnings and smart defaults
- Per-channel strip: gain, fader, pan, mute/solo, level meter
- Our own EQ and compressor with visual feedback
- Master bus with metering (peak/RMS, loudness)
- Guided stages, each opening with a recommendation you refine
- Role/genre starter templates
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
