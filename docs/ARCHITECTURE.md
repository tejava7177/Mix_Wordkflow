# Architecture & Implementation Design

How MixMentor is built: the audio engine that mixes multitrack audio in real time,
the per-channel DSP, the metering, and the guidance/template layer that makes it
educational.

## Layered overview

```
UI          JUCE Components — console, channel strips, EQ/comp editors,
            guided-mode panel, meters, transport

Guidance    Stage model, per-control help text, template application
            (the "educational" layer)

Engine      Synced multitrack playback, per-channel DSP chain, master bus,
            metering, offline mixdown/export

Domain      Pure data: Session, Channel, ProcessingChain, EqBand,
            CompressorSettings, Template, Stage
```

## Domain data model

```
Session
 ├─ sampleRate, tempo (optional), master ChannelState
 └─ Channel[]
      ├─ name, colour, source file
      ├─ gain (trim), fader, pan, mute, solo
      └─ ProcessingChain
           ├─ EqBand[]           // freq, gain, Q, type (HP / bell / shelf)
           └─ CompressorSettings // threshold, ratio, attack, release, makeup

Template   // a saved ProcessingChain (+ default fader/pan) for one instrument
Stage      // id, title, help text, which controls it activates (guidance layer)
```

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
