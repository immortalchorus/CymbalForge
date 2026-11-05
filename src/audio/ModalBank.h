#pragma once
#include <JuceHeader.h>

namespace cymbalforge {
struct ModalBank {
    static constexpr int maxModes = 768;
    struct Excitation { float velocity, position, brightness, decay, complexity, shimmer, swell, masterTune; int model, requestedModes; };
    void prepare(double sampleRate);
    void strike(const Excitation& excitation);
    std::array<float, 2> process();
    bool active() const noexcept { return energy > 1.0e-7f; }
private:
    // bloom is a one-pole state: avoids an exp() for every mode, every sample.
    struct Mode { float a{}, b{}, z1{}, z2{}, attackGain{}, bloomGain{}, panL{}, panR{}, bloom{}, bloomStep{}; };
    std::array<Mode, maxModes> modes{};
    juce::Random random { 0x43594D42 };
    int count = 256;
    float energy = 0.0f;
    double rate = 48000.0;
};
}
