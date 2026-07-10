# The Mixing Process (Domain Reference)

This is the mixing workflow MixMentor teaches. Each stage lists **what** it does,
**why** it matters, **how** it works technically (which maps to the DSP MixMentor
implements), and a **beginner rule**. The app surfaces this same content as
explain-as-you-go help.

The recommended order:

```
Gain staging → Balance → Panning → EQ → Compression → Reverb & delay → Automation → Master
```

## 1. Gain staging

- **What** — set each track's working level so peaks land around −12 to −6 dBFS,
  leaving headroom.
- **Why** — a clean, consistent starting point; avoids clipping and makes every
  downstream processor behave predictably.
- **How** — a trim/gain multiply at the head of the channel, checked against a
  peak meter.
- **Beginner rule** — don't start too hot. If it's clipping, you've already lost.

## 2. Balance (faders)

- **What** — set the relative loudness of every track using faders alone.
- **Why** — the single most important step; a mix is roughly 80% decided here.
  Processing cannot rescue a bad balance.
- **How** — per-channel gain summed to the master bus.
- **Beginner rule** — get it sounding good with faders before touching any
  processor. Reference the most important element (often vocal, or kick/snare).

## 3. Panning

- **What** — place elements across the left–right stereo field.
- **Why** — creates separation and width, and frees up the center.
- **How** — an equal-power pan law distributes the signal between L and R so
  perceived loudness stays constant across the pan.
- **Beginner rule** — keep kick, bass, lead vocal, and snare centered; spread
  guitars, keys, and overheads.

## 4. EQ

- **What** — shape the frequency balance. Two jobs: corrective (cut problems —
  low rumble via a high-pass, mud around 200–500 Hz, harshness around 2–5 kHz) and
  creative (boost character).
- **Why** — reduces frequency masking so instruments each have their own space.
- **How** — filters (high-pass, bell/peak, shelf) implemented as **IIR biquads**;
  each band has a frequency, gain, and Q.
- **Beginner rule** — cut before you boost; cut narrow, boost wide; high-pass most
  non-bass tracks.

## 5. Compression

- **What** — reduce dynamic range for consistency, punch, and glue.
- **Why** — evens out uneven performances and pushes elements forward.
- **How** — threshold, ratio, attack, release, makeup gain. The detector measures
  level; gain is reduced above the threshold. Attack shapes transients (punch),
  release shapes sustain.
- **Beginner rule** — start gentle (low ratio, a few dB of gain reduction); use
  attack/release to add punch, not just to squash.

## 6. Reverb & delay (space)

- **What** — add depth and ambience; place sounds front-to-back.
- **Why** — turns dry, flat tracks into a believable space.
- **How** — send channels to a shared reverb/delay bus (aux) and blend wet with dry.
  Reverb is many dense reflections; delay is discrete echoes.
- **Beginner rule** — use sends, not inserts; high-pass the reverb return; use less
  than you think.

## 7. Automation

- **What** — change parameters over time (vocal level rides, mutes, build-ups, FX
  throws).
- **Why** — keeps the mix clear and expressive moment to moment.
- **How** — parameter values recorded along the timeline and played back.
- **Beginner rule** — finish a solid static mix first, then automate to refine.

## 8. Master bus

- **What** — glue the whole mix (gentle bus compression and EQ) and control loudness
  (limiter), metered to a target.
- **Why** — cohesion plus a correct delivery level.
- **How** — processing on the summed master signal; loudness measured in **LUFS**.
- **Beginner rule** — a light touch; compare against commercial reference tracks.

## Cross-cutting principles

- Reference commercial tracks often.
- Less is more — subtle moves add up.
- Mix at a low volume; take breaks to rest your ears.
- If a change doesn't clearly help, undo it.
