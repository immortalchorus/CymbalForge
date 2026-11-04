#pragma once
#include <JuceHeader.h>

namespace cymbalforge {
struct ModalBank {
    static constexpr int maxModes = 128;
    void prepare(double sampleRate);
    void strike(float velocity, float position, float brightness, float decay, int requestedModes);
    float process();
    bool active() const noexcept { return energy > 1.0e-7f; }
private:
    struct Mode { float a{}, b{}, z1{}, z2{}, gain{}; };
    std::array<Mode, maxModes> modes{};
    int count = 64;
    float energy = 0.0f;
    double rate = 48000.0;
};
}
