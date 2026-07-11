# Development Roadmap

Phased plan from an empty shell to a demoable guided mixer that takes a beginner to a
release-ready mix. Each phase ends at something runnable and showable.

## Phase 1 — Audible multitrack engine

Make it play and balance.

- Domain model: Session → Channel (+ StemRole) → ProcessingChain
- Session setup: load stems, label roles, pick a genre/style
- Audio engine: `AudioDeviceManager` + synced multitrack playback
- Per-channel gain, fader, pan (equal-power), mute, solo, summed to master
- Per-channel and master level meters (atomics + Timer)
- Transport: play, stop, loop; load a demo session

Milestone: open the demo session, press play, and balance tracks with faders/pans.

## Phase 2 — Channel processing

Make it shape sound.

- Our own EQ per channel (`juce::dsp::IIR`) with a curve/spectrum display
- Our own compressor per channel (`juce::dsp::Compressor`) with a gain-reduction meter
- Master bus metering
- WAV mixdown export (offline render through the same chain)

Milestone: EQ and compress channels, then export a finished WAV.

## Phase 3 — Analysis & recommendations

Make it do the knowing for the user.

- Import analysis: per-stem loudness, peak, crest, spectrum, clip/DC/phase
- GenreProfile targets and StemRole defaults
- Recommendation engine: genre × role × analysis → suggested settings per stage,
  each with a plain-language reason
- Import warnings (clipping, masking, level problems)

Milestone: on import, the app flags problems and pre-fills sensible starting settings.

## Phase 4 — The guided layer

Make it teach and lead.

- Stage model and guided mode (focus the current stage's controls)
- Each stage opens with its recommendation applied and explained
- Explain-as-you-go help on every control, from the domain reference
- Guided flow across gain → balance → pan → EQ → compression → master

Milestone: a first-timer follows guided mode end to end and reaches a good mix.

## Phase 5 — Templates, content & polish

Make it complete and presentable.

- Role/genre starter templates with apply + inspect
- Reverb/delay send bus and (optionally) simple automation
- Loudness (LUFS) metering and a master limiter
- Visual design pass toward the target UI skin
- Cross-platform build verification (macOS / Windows)
- README screenshots and a short demo capture

Milestone: a polished, cross-platform build that takes a beginner from stems to a
release-ready, exportable mix.
