# MVP Scope

The MVP is a single vertical slice that proves the core promise: **a beginner opens
a session, follows the guided stages, and exports a mix that clearly sounds better —
understanding each step.**

## Included

- Standalone JUCE desktop application (macOS first, Windows-ready)
- Load a multitrack session (audio stems) and play all tracks in sync
- A bundled demo session so the app is usable with zero setup
- Per-stem role labeling (with a DSP-suggested guess)
- Genre/style selection that sets mixing targets (a few styles)
- Import analysis (loudness, peak, crest, spectrum, clip/DC/phase) with warnings
- Recommendation engine: genre × role × analysis → suggested defaults per stage
- Per-channel strip: gain, fader, pan, mute, solo, level meter
- Per-channel EQ with a spectrum/curve display (our own processor)
- Per-channel compressor with a gain-reduction meter (our own processor)
- Master bus with peak/RMS metering
- Guided mode across the core stages, each opening with recommendations applied
- Explain-as-you-go help text on every control
- A small set of role/genre starter templates (vocal, drums, bass, balanced mix)
- Export the finished mix to WAV

## Excluded (for now / non-goals)

- Reverb / delay send buses (Phase 4+)
- Automation (Phase 4+)
- Loudness (LUFS) metering and a master limiter (post-MVP)
- Large template / content library
- Recording, MIDI, and virtual instruments
- **Full-DAW features** — arrangement timeline, unlimited tracks (out of scope by design)
- **Third-party plugin hosting** (VST/AU) — MixMentor ships its own processors
- **AI / machine learning** — all assistance is deterministic DSP + expert rules

## Success Criteria

- The app builds and runs as a standalone JUCE application
- A first-time user understands it takes them from raw stems to a finished mix
- This flow works end to end on the demo session:
  1. open the demo session, label roles, pick a genre, and review the analysis
  2. press play (tracks play in sync) and hear the raw mix
  3. follow guided mode through gain → balance → pan → EQ → compression → master,
     each stage opening with a recommendation the user understands and refines
  4. see meters/spectra respond and read the help at each step
  5. export a WAV mixdown that sounds clearly better — approaching release quality
- The code is cleanly layered (Domain / Analysis / Engine / Guidance / UI) and easy
  to extend
