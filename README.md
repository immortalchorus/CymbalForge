# CymbalForge

CymbalForge is a native Windows-first cymbal synthesizer written in C++20 and JUCE. It combines an inharmonic modal bank with a velocity-sensitive noise exciter for playable crash and ride sounds.

## Build

Install CMake 3.24+, a C++20 compiler (Visual Studio 2022 recommended), and Git. JUCE is fetched on configure.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release
```

The app executable is under `build/CymbalForge_artefacts/Release`. The first build requires internet access solely to obtain JUCE.

## Current features

- Native JUCE desktop UI with 2D bell/bow/edge audition surface
- Real-time audio through JUCE (WASAPI/ASIO capabilities depend on the installed driver)
- 32–128 modal resonators, strike position, velocity, brightness, decay, noise and reverb
- MIDI note input and JSON preset persistence
- 24-bit WAV single-hit export
- Deterministic offline rendering and DSP smoke tests

## Roadmap

Hi-hat interaction, MIDI learn, nonlinear coupling, convolution, multi-velocity sample maps, and VST3 are deliberately separate milestones. See [ARCHITECTURE.md](ARCHITECTURE.md).
