# User Flow — From Stems to a Finished Mix

This document walks the complete MixMentor user journey, one step at a time,
starting from the state where the user already has stem files. For each step it
describes what the user does, what the app shows, what happens under the hood, and
how the teaching layer supports the step.

## Starting assumption

The user has a set of **stem files** — separate audio files for each part of one
song (e.g. `drums.wav`, `bass.wav`, `vocal.wav`, `guitar.wav`), ideally the same
length and sample rate. A bundled demo session is also available for users with no
stems of their own.

## The journey at a glance

```
Import stems → label roles → choose genre → analyze → listen to the raw mix
            → gain → balance → pan → EQ → compress → check the master → export
```

Guided mode leads the user through the stages in order, each opening with an
analysis-, genre-, and role-informed recommendation already applied; free mode lets
experienced users roam. The setup steps (roles, genre, analysis) are what let a
beginner reach a release-ready result without prior knowledge.

## Cross-cutting elements (available at every step)

- **Transport** — play, stop, loop a range, scrub the timeline
- **A/B compare** — instantly toggle between the raw import and the current mix
- **Explain-as-you-go** — every control shows a plain-language "what it does / when
  to use it"
- **Templates** — apply a ready-made processing chain to a channel, then inspect it
- **Undo / redo**
- **Meters** — per-channel and master level meters are always visible
- **Recommended defaults** — in guided mode, each stage opens with settings already
  suggested (from genre × role × analysis) and explained; the user refines rather than
  starting blank

---

## Step 1 — Launch and choose a starting point

- **User** — opens MixMentor.
- **Screen** — a start screen with two clear choices: "Open my stems" and "Try the
  demo session." A short line frames the app: "Load your tracks and learn to mix
  them, step by step."
- **Under the hood** — audio device is initialized; no audio is playing yet.
- **Teaching** — the framing sets expectations: this is a guided, learn-by-doing tool.

## Step 2 — Select the stem files

- **User** — clicks "Open my stems" and selects the audio files (multi-select, or a
  folder).
- **Screen** — a native file picker filtered to audio formats (wav, aiff, flac, mp3).
- **Under the hood** — the chosen paths are handed to the engine for import.

## Step 3 — Import and validate

- **User** — waits briefly while stems load.
- **Screen** — each stem becomes a **channel** in the console. Channels are
  auto-named from the filename and auto-colored. A short import summary appears
  (length, sample rate, peak level per stem).
- **Under the hood** — each file is opened with an `AudioFormatReader`; length,
  sample rate, and channel count are read. The engine checks that stems share a
  sample rate and similar length.
- **Guard rails** — if sample rates differ, the app offers to resample; if lengths
  differ, it aligns them from the start and notes the mismatch; mono stems are
  handled as center-panned.
- **Teaching** — a note explains what a "stem" and a "channel" are.

## Step 3a — Label each stem's role

- **User** — confirms or sets each channel's role from a dropdown (drums, bass, lead
  vocal, backing vocal, guitar, keys, synth, FX, other).
- **Screen** — each strip shows a role selector pre-filled with the app's best guess;
  unknown stems default to "other / instrument."
- **Under the hood** — DSP suggests a role from the stem's spectral/temporal
  signature; the user's choice is authoritative and stored as `StemRole`.
- **Teaching** — the user doesn't need mixing knowledge to say "this is a lead vocal";
  the role is how the app knows how to treat the track.
- **Why it matters** — roles are how MixMentor covers the huge variety of possible
  stems: any file is reduced to a known role with role-appropriate handling.

## Step 3b — Choose the genre / style

- **User** — picks the song's style (e.g. pop, rock, ballad, hip-hop, EDM, acoustic).
- **Screen** — a short style picker; each option names what it optimizes for
  (e.g. "modern pop — bright, loud, tight low end").
- **Under the hood** — the choice loads a `GenreProfile` that sets mixing targets
  (tonal balance, loudness, low-end and width guidance) and default moves.
- **Teaching** — explains that mixing choices depend on genre; the app will aim for
  targets appropriate to this style.

## Step 3c — Analyze and prepare recommendations

- **User** — waits a moment while the app inspects the stems.
- **Screen** — a brief analysis summary and any warnings (e.g. "vocal is clipping,"
  "bass and kick overlap in the low end," "this stem is very quiet").
- **Under the hood** — per-stem DSP analysis (loudness, peak, crest, spectrum,
  clip/DC/phase). The recommendation engine combines analysis × genre × role into
  suggested starting settings for every stage, each with a plain-language reason.
- **Teaching** — surfaces problems the beginner couldn't spot, in plain language.

## Step 4 — Land on the console (raw, with recommendations ready)

- **User** — sees the full mixing console for the first time.
- **Screen** — one channel strip per stem (role-labeled, colored), plus a master
  strip. The mix starts at its **raw** state (faders unity, pan center, no processing)
  so the user can hear the untouched material first. A guided-mode panel offers: "Walk
  me through mixing this," and each stage's recommendation is prepared, ready to apply
  and explain as the user reaches it.
- **Under the hood** — the `Session` holds a `Channel` per stem with its `StemRole`,
  `StemAnalysis`, and the recommendation engine's suggested settings staged per stage.
- **Teaching** — guided mode is offered but optional; free mode is one click away.

## Step 5 — Listen to the raw mix first

- **User** — presses play and just listens.
- **Screen** — the transport runs; per-channel and master meters move. This is the
  raw, unbalanced sum of all stems at unity.
- **Under the hood** — a single sample-position clock drives every channel's reader;
  each block is read, summed, and sent to the output. Peak/RMS per channel is
  measured for the meters.
- **Teaching** — "Before changing anything, listen. What's too loud? Too quiet?
  Cluttered? You'll fix these in order." The user can loop a section to focus.

## Step 6 — Stage 1: Gain staging

- **User** — sets each channel's input trim so peaks land in a healthy range.
- **Screen** — guided mode highlights the trim control and peak meter on each strip;
  other controls dim. Meters flag any channel that clips (red).
- **Under the hood** — trim is a gain multiply at the head of the channel chain,
  before everything else.
- **Teaching** — explains headroom and the −12 to −6 dBFS target; warns against
  starting too hot.

## Step 7 — Stage 2: Balance (faders)

- **User** — adjusts faders while the song plays until the balance sounds good,
  using mute/solo to focus on parts.
- **Screen** — guided mode brings the faders and mute/solo into focus; the master
  meter shows the summed level. A suggested order (start from the most important
  element) is offered.
- **Under the hood** — each channel's fader gain feeds the master sum; solo/mute
  gate channels in the mix.
- **Teaching** — "This is the most important step — a mix is ~80% faders. Get it
  sounding good here before any processing."

## Step 8 — Stage 3: Panning

- **User** — pans channels across the stereo field.
- **Screen** — guided mode focuses the pan controls; an optional stereo-field view
  shows where each channel sits.
- **Under the hood** — an equal-power pan law sets left/right gains so perceived
  loudness stays constant across the pan.
- **Teaching** — conventions: kick, bass, lead vocal, snare centered; guitars, keys,
  overheads spread for width and separation.

## Step 9 — Stage 4: EQ

- **User** — opens a channel's EQ and shapes its frequency balance.
- **Screen** — an EQ editor with a live spectrum and an interactive curve; bands can
  be added, dragged (frequency/gain), and adjusted (Q). A gentle default: a
  high-pass on non-bass channels.
- **Under the hood** — each band is a `juce::dsp::IIR` biquad (high-pass, bell,
  shelf); the spectrum comes from an FFT of the channel signal.
- **Teaching** — corrective vs creative EQ; cut before boost; cut narrow, boost wide;
  high-pass most non-bass tracks; typical problem ranges (mud 200–500 Hz, harshness
  2–5 kHz).

## Step 10 — Stage 5: Compression

- **User** — opens a channel's compressor and controls its dynamics.
- **Screen** — threshold, ratio, attack, release, makeup, plus a **gain-reduction
  meter** that shows the compressor working.
- **Under the hood** — `juce::dsp::Compressor` plus a makeup-gain stage in the
  channel chain, after EQ.
- **Teaching** — what each control does; start gentle; use attack/release to add
  punch, not just to squash; templates offer good starting points to inspect.

## Step 11 — Stage 6: Master and metering

- **User** — checks the overall level on the master and applies a light master touch
  if needed.
- **Screen** — the master strip with its meter (peak/RMS, and later loudness); an
  optional gentle master EQ/compression.
- **Under the hood** — the summed signal passes through the master chain before
  output; master metering measures the final signal.
- **Teaching** — headroom and a sensible target level; compare against a commercial
  reference; keep the master processing light.

## Step 12 — A/B compare

- **User** — toggles between the raw import and the finished mix.
- **Screen** — a single A/B control; the difference should be obvious and
  encouraging.
- **Under the hood** — A/B bypasses all channel processing and resets faders/pans to
  neutral for the "raw" state, restoring the mix for "mixed."
- **Teaching** — hearing the before/after is the payoff and reinforces what each
  stage contributed.

## Step 13 — Export the mix

- **User** — clicks export, confirms the file settings, and saves.
- **Screen** — an export dialog (file name, format/bit depth) and a progress
  indicator; on completion, an option to reveal the file.
- **Under the hood** — an offline render seeks all channels to the start and
  processes the whole session block by block through the same channel and master
  chains into one buffer, written with a `juce::AudioFormatWriter` (WAV). Because it
  reuses the live DSP, the file matches what was heard.
- **Teaching** — "You just mixed a song and understand every step you took."

---

## Notes and open questions for planning

- **Guided vs free** — guided mode is the default for first-time users; a "skip to
  free console" is always available. Returning users can default to free.
- **Templates** — where they appear in the flow (a button on each channel strip; a
  gallery on first opening a processor) and which ship first (vocal, drums, bass, a
  balanced starter).
- **Demo session** — what stems ship so a user with no material can complete the
  full flow immediately.
- **Progress** — whether guided mode tracks completed stages (a checklist) to give a
  sense of progress and let users resume.
