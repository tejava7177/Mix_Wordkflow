# MVP Scope

The MVP is a single vertical slice that proves the core promise: **a beginner opens
a session, follows the guided stages, and exports a mix that clearly sounds better —
understanding each step.**

## Included

- Standalone JUCE desktop application (macOS first, Windows-ready)
- Load a multitrack session (audio stems) and play all tracks in sync
- A bundled demo session so the app is usable with zero setup
- Per-channel strip: gain, fader, pan, mute, solo, level meter
- Per-channel 3-band EQ with a spectrum/curve display
- Per-channel compressor with a gain-reduction meter
- Master bus with peak/RMS metering
- Guided mode across the core stages: gain, balance, pan, EQ, compression, master
- Explain-as-you-go help text on every control
- A small set of starter templates (e.g. vocal, drums, bass, balanced mix)
- Export the finished mix to WAV

## Excluded (for now)

- Reverb / delay send buses (Phase 4+)
- Automation (Phase 4+)
- Loudness (LUFS) metering and a master limiter (post-MVP)
- Large template / content library
- Recording, MIDI, and virtual instruments
- Plugin (VST/AU) build targets

## Success Criteria

- The app builds and runs as a standalone JUCE application
- A first-time user understands it is a tool for *learning to mix*
- This flow works end to end on the demo session:
  1. open the demo session and press play (tracks play in sync)
  2. follow guided mode through gain → balance → pan → EQ → compression → master
  3. read the help and see meters/spectra respond to each change
  4. apply a template to a channel and inspect it
  5. export a WAV mixdown that sounds clearly better than the raw stems
- The code is cleanly layered (Domain / Engine / Guidance / UI) and easy to extend
