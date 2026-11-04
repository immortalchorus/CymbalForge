#pragma once
#include <JuceHeader.h>

namespace cymbalforge {
struct ModalBank {
    static constexpr int maxModes = 768;
    void prepare(double sampleRate);
    void strike(float velocity, float position, float brightness, float decay, float complexity, float shimmer, int requestedModes);
    std::array<float, 2> process();
    bool active() const noexcept { return energy > 1.0e-7f; }
private:
    struct Mode { float a{}, b{}, z1{}, z2{}, attackGain{}, bloomGain{}, panL{}, panR{}, bloomRate{}; };
    std::array<Mode, maxModes> modes{};
    juce::Random random { 0x43594D42 };
    int count = 256;
    float energy = 0.0f, bloom = 0.0f;
    double rate = 48000.0;
};
}
