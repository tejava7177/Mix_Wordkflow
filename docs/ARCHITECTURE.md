# Architecture & Implementation Design

How MixMentor is built: the audio engine that mixes multitrack audio in real time,
the per-channel DSP, the metering, and the guidance/template layer that makes it
educational.

## Layered overview

```
UI          JUCE Components — console, channel strips, EQ/comp editors,
            guided-mode panel, meters, transport

Guidance    Stage model, per-control help text, recommendation engine,
            template application (the layer that gets a novice to a good mix)

Analysis    Per-stem DSP measurement (loudness, peak, crest, spectrum,
            clip/DC/phase) + genre/role-driven recommendations

Engine      Synced multitrack playback, per-channel DSP chain, master bus,
            metering, offline mixdown/export

Domain      Pure data: Session, GenreProfile, Channel, StemRole,
            ProcessingChain, EqBand, CompressorSettings, Template, Stage
```

All "intelligence" is deterministic DSP + expert rules — no AI/ML. Processing is
performed by our own curated processors; MixMentor does not host third-party plugins.

## Domain data model

```
Session
 ├─ sampleRate, master ChannelState
 ├─ GenreProfile          // chosen style → mixing targets & default moves
 └─ Channel[]
      ├─ name, colour, source file
      ├─ StemRole          // drums / bass / lead vocal / guitar / keys / ... / other
      ├─ StemAnalysis      // loudness, peak, crest, spectrum, clip/DC/phase flags
      ├─ gain (trim), fader, pan, mute, solo
      └─ ProcessingChain
           ├─ EqBand[]           // freq, gain, Q, type (HP / bell / shelf)
           └─ CompressorSettings // threshold, ratio, attack, release, makeup

GenreProfile // target tonal balance, loudness, low-end/width guidance per style
StemRole     // taxonomy that maps a stem to role-appropriate defaults & templates
Template     // a saved ProcessingChain (+ default fader/pan) for a role
Stage        // id, title, help text, which controls it activates (guidance layer)
```

`GenreProfile` and `StemRole` are how MixMentor covers the huge variety of user
material without per-file magic: the user reduces any stem to a known *role*, and the
chosen *genre* sets targets — together they select good defaults and templates.

## Import analysis (DSP, not AI)

On import, each stem is measured so the app can pre-fill good defaults and warn about
problems. All measurement is classic DSP — deterministic and explainable:

- **Level** — integrated loudness, true/sample peak, crest factor (dynamics)
- **Spectrum** — long-term average spectral balance (FFT), used to spot excess
  low-end mud, boxiness, or harshness relative to the genre target
- **Integrity** — clipping, DC offset, and stereo phase/correlation checks
- **Role hint** — a rough suggestion of the stem's role from its spectral/temporal
  signature, offered as the default for the user to confirm (the user has final say)

## Recommendation engine (genre × role × analysis → defaults)

A rule-based engine turns `GenreProfile` + `StemRole` + `StemAnalysis` into starting
settings for each stage: a suggested trim, fader level, pan, high-pass frequency, and
gentle EQ/compression moves, plus plain-language reasons. In guided mode each stage
opens with these recommendations already applied; the user understands and refines
them. There is no machine learning — just curated expert rules, so every suggestion
can be explained.

## Curated processors (no plugin hosting)

MixMentor ships its own small set of processors (EQ, compressor, and later a limiter),
each built on `juce::dsp` primitives with one consistent, teachable UI. It does not
host VST/AU plugins. A single, curated toolset is a deliberate product choice: the
beginner never chooses between competing tools, and every control is one we can
explain and pre-configure.

## Audio engine

- A `juce::AudioDeviceManager` drives an audio callback (`AudioIODeviceCallback`,
  or an `AudioAppComponent`).
- **Synced multitrack playback:** one master transport position (in samples) drives
  every channel. Each channel reads its file via an `AudioFormatReaderSource`; per
  block, all channels are read at the same position, processed, and summed. A
  `MixerAudioSource` can host the channel sources, or the mix can be summed manually
  in the callback for full control over per-channel metering and processing.

### Per-channel signal chain

```
file source → trim gain → EQ → compressor → fader gain → pan → sum into master
```

- **EQ** — a `juce::dsp::ProcessorChain` of `juce::dsp::IIR::Filter` biquads, one per
  band, coefficients from `IIR::Coefficients` (high-pass, peak, shelf).
- **Compressor** — `juce::dsp::Compressor` (threshold, ratio, attack, release) plus a
  makeup-gain stage.
- **Pan** — an equal-power law: `leftGain = cos(θ)`, `rightGain = sin(θ)` with
  `θ = (pan + 1) * π/4`.
- **Fader / trim** — sample-wise gain multiply.

### Master bus

The summed signal passes through an optional master chain (glue compression, gentle
EQ, and later a limiter) before the output device. Master metering measures the final
signal.

## Metering

- In the audio callback, each channel and the master track compute peak and RMS for
  the block and store them in `std::atomic` values.
- A UI `juce::Timer` (~30 Hz) reads those atomics and repaints meters, keeping the
  audio thread lock-free.
- Loudness (LUFS) on the master is a later addition using a K-weighted measurement.

## Offline mixdown / export

Export reuses the same processing path offline: seek all channels to the start, then
render the whole session block by block through each channel chain and the master
chain into one buffer, and write it with a `juce::AudioFormatWriter`
(`WavAudioFormat`). Because export shares the live DSP, the exported file matches what
was heard.

## Guidance layer (what makes it educational)

- **Stage model** — an ordered list of `Stage`s (gain, balance, pan, EQ,
  compression, master). Each stage has a title, help text, and the set of controls
  it activates. Guided mode highlights the current stage's controls and de-emphasizes
  the rest; the user can still move freely.
- **Explain-as-you-go** — each control references a short help string surfaced on
  hover/focus, backed by the content in [MIXING_STAGES.md](MIXING_STAGES.md).
- **Templates** — a `Template` is a serialized `ProcessingChain` (+ default fader and
  pan). Applying a template copies its settings onto a channel; the user then
  inspects and tweaks. Templates and sessions serialize to JSON or a JUCE `ValueTree`.

## Threading model

- Audio thread: real-time DSP and metering only; no allocation or locking.
- Message thread: UI, meters (via Timer), and parameter changes pushed to the audio
  thread through atomics / a lock-free queue.

## Build targets

Standalone JUCE application, macOS first, Windows-ready. Modules: `juce_audio_utils`,
`juce_audio_formats`, `juce_dsp`, `juce_gui_extra`.

## Design principles & non-goals

- **One path, not a toolbox** — a single curated processor per job; simplicity is a
  feature.
- **Explainable, deterministic** — DSP + expert rules only; no AI/ML, so every default
  and warning has a reason.
- **Focused mixer, not a DAW** — no recording, MIDI, arrangement timeline, plugin
  hosting, or unlimited tracks. This keeps the product shippable and the goal sharp,
  while still exercising the same real-time audio, DSP, metering, and UI engineering a
  DAW's mixer would.
