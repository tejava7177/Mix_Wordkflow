# Workflows

MixMentor supports a guided workflow for learners and a free workflow for practice,
sharing one engine.

## Guided workflow (default)

The app walks the user through the mixing stages in order. At each stage the relevant
controls are in focus, with plain-language help and live visual feedback.

1. **Open a session** — load the bundled demo multitrack, or import your own stems
2. **Gain staging** — set healthy input levels with headroom
3. **Balance** — set relative loudness with faders until the song sounds good
4. **Panning** — place elements across the stereo field
5. **EQ** — cut problem frequencies, carve space between instruments
6. **Compression** — control dynamics and add punch
7. **Master** — check the master meter and loudness
8. **Export** — render the mix to a WAV file

At any point the user can step back, replay, or leave guided mode to adjust freely.

## Free workflow

Guided mode off: the full console is available at once for users who already know
what they want. The same explain-as-you-go help and templates remain available.

## Templates

At any stage the user can apply a template to a channel:

- pick a starting chain (e.g. "vocal starter", "drum bus", "acoustic guitar")
- hear the result immediately
- open the channel to inspect exactly what the template set, and tweak it

Templates are a teaching device as much as a shortcut: they show *good defaults* the
user can reverse-engineer.

## UX intent

The interface should feel encouraging and legible, not intimidating. Show one clear
next step at a time in guided mode; make every control explain itself; make the
effect of every action visible through meters and spectra. The user should always
know what to do next and why.

## Design reference

The console layout (transport, channel strips with fader/pan/mute/solo/meter, master
strip) and the guided-stage panel are captured in the project's UI mockups and inform
the JUCE component layout.
