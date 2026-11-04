# CymbalForge research basis

## What changed

The original prototype used a small, regular inharmonic bank plus white-noise
layer. That produces an obvious pitch centre and a static hiss: the "gamelan
bells over white noise" failure mode.

The engine is now a **deterministic-plus-stochastic hybrid**:

- A 128–768 voice non-harmonic modal field, distributed densely toward the
  high end rather than as multiples of a nominal fundamental.
- Independent phase, detune, decay, and stereo position for every mode.
- Radial and azimuthal nodal-like strike weights, so bell, bow, and edge hits
  excite a different spectrum instead of merely changing one EQ parameter.
- A delayed, upper-band modal bloom that rises with velocity and *Shimmer*.
- A filtered residual with distinct short stick-contact and longer metallic-wash
  envelopes, rather than raw white noise.

This is a performant real-time perceptual model. It is deliberately not called
a finite-element simulation or an exact model of any branded cymbal.

## Evidence translated into controls

| Finding | Implementation |
| --- | --- |
| Cymbals need very many modes; a DAFx crash-cymbal implementation allocated more than 2,000 modes per instrument. | `Complexity` scales a dense CPU-safe 128–768 mode field. |
| Strong hits drive nonlinear behaviour and a fast increase in high-frequency energy. | `Shimmer` produces a velocity-dependent delayed upper-band bloom and residual wash. |
| Stick/mallet and central support dynamics change timbre. | Strike position changes modal excitation weights, while the contact residual is separate from the ringing modes. |
| Spectral modelling separates time-varying sinusoidal and filtered-noise parts. | Modal field and filtered residual each have their own envelopes. |
| Ogun's useful idea is broad timbre mapping, not manual editing of thousands of partials. | The UI exposes musically meaningful macro controls: Brightness, Complexity, Shimmer, Decay, and Noise. |

## Sources

- Image-Line, [Ogun tutorial](https://ww2.image-line.com/fl-studio-learning/fl-studio-online-manual/html/plugins/Ogun_tutorial.htm) and [Pad Harmonics Tool](https://ww2.image-line.com/fl-studio-learning/fl-studio-online-manual/html/plugins/Ogun_PadHarmonics.htm).
- Skare and Abel, [Real-Time Modal Synthesis of Crash Cymbals with Nonlinear Approximations](https://www.dafx.de/paper-archive/2019/DAFx2019_paper_48.pdf), DAFx 2019.
- Ducceschi and Touzé, [Modal approach for nonlinear vibrations of damped impacted plates](https://cris.unibo.it/handle/11585/836437), *Journal of Sound and Vibration*, 2015.
- Samejima, [Nonlinear physical modeling sound synthesis of cymbals involving dynamics of washers and sticks/mallets](https://www.jstage.jst.go.jp/article/ast/42/6/42_E2087/_article/-char/en), 2021.
- Serra and Smith, [Spectral Modeling Synthesis](https://mtg.upf.edu/node/251), 1990.
