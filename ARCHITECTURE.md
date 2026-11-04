# Architecture

## Real-time path

`MainComponent` owns the JUCE audio callback and `CymbalEngine`. UI controls write atomics in `CymbalParameters`; the audio callback reads them without locks or allocation. Mouse and MIDI events call `strike()`, which initializes resonator state before the following samples are rendered. The engine produces stereo audio and applies a compact feedback reverb.

## Modules

- `audio/ModalBank`: deterministic inharmonic resonator coefficients and sample processing.
- `audio/NoiseLayer`: filtered stochastic strike transient/wash.
- `audio/CymbalEngine`: parameters, voice lifecycle, stereo rendering and reverb.
- `ui/`: JUCE components; no DSP implementation is embedded in UI code.
- `presets/`: human-readable JSON state.
- `export/`: a separate offline engine instance, guaranteeing export cannot interrupt playback.

## Threading and safety

The audio callback never performs disk access, locks, GUI calls, or allocation. Preset and export work occur on the message thread. This MVP supports one cymbal voice; polyphony can be introduced as a lock-free voice pool.

## Extension points

`CymbalParameters` is the stable control boundary. New exciter layers, a two-plate hi-hat model, and convolution belong behind `CymbalEngine`; plugin targets can wrap the same engine in a JUCE `AudioProcessor` without changing DSP.
