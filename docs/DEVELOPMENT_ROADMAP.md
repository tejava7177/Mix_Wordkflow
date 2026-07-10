# Development Roadmap

Phased plan from an empty shell to a demoable educational mixer. Each phase ends at
something runnable and showable.

## Phase 1 — Audible multitrack engine

Make it play and balance.

- Domain model: Session → Channel → ProcessingChain
- Audio engine: `AudioDeviceManager` + synced multitrack playback
- Per-channel gain, fader, pan (equal-power), mute, solo, summed to master
- Per-channel and master level meters (atomics + Timer)
- Transport: play, stop, loop; load a demo session

Milestone: open the demo session, press play, and balance tracks with faders/pans.

## Phase 2 — Channel processing

Make it shape sound.

- 3-band EQ per channel (`juce::dsp::IIR`) with a curve/spectrum display
- Compressor per channel (`juce::dsp::Compressor`) with a gain-reduction meter
- Master bus metering
- WAV mixdown export (offline render through the same chain)

Milestone: EQ and compress channels, then export a finished WAV.

## Phase 3 — The educational layer

Make it teach.

- Stage model and guided mode (focus the current stage's controls)
- Explain-as-you-go help on every control, from the domain reference
- Guided flow across gain → balance → pan → EQ → compression → master

Milestone: a first-timer can follow guided mode end to end and understand each step.

## Phase 4 — Templates, content & polish

Make it complete and presentable.

- Starter templates (vocal, drums, bass, balanced mix) with apply + inspect
- Reverb/delay send bus and (optionally) simple automation
- Loudness (LUFS) metering and a master limiter
- Visual design pass toward the target UI skin
- Cross-platform build verification (macOS / Windows)
- README screenshots and a short demo capture

Milestone: a polished, cross-platform build with templates, an audible learning
flow, and an exportable result.
