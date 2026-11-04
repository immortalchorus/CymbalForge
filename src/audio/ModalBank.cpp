#include "ModalBank.h"
#include <cmath>
namespace cymbalforge {
void ModalBank::prepare(double sampleRate) { rate = sampleRate; for (auto& m : modes) m = {}; }
void ModalBank::strike(float velocity, float position, float brightness, float decay, int requestedModes) {
    count = juce::jlimit(8, maxModes, requestedModes);
    // Deterministic, deliberately inharmonic plate-inspired partial distribution.
    for (int i = 0; i < count; ++i) {
        auto n = float(i + 1); auto f = 150.0f * (0.72f * n + 0.085f * n * n) * (0.7f + brightness * 0.6f);
        f = juce::jmin(f, float(rate * 0.45));
        auto radiusWeight = 0.35f + 0.65f * std::abs(std::sin(n * (0.8f + position * 2.1f)));
        auto seconds = (0.12f + 7.0f * decay) / (1.0f + n * 0.035f);
        auto r = std::exp(-1.0f / (seconds * float(rate)));
        auto theta = juce::MathConstants<float>::twoPi * f / float(rate);
        modes[i] = { 2.0f * r * std::cos(theta), -r * r, velocity * radiusWeight / std::sqrt(n), 0.0f, 1.0f };
    }
    for (int i = count; i < maxModes; ++i) modes[i] = {};
    energy = velocity;
}
float ModalBank::process() {
    float output = 0.0f;
    for (int i = 0; i < count; ++i) { auto& m = modes[i]; auto y = m.a*m.z1 + m.b*m.z2; m.z2=m.z1; m.z1=y; output += y*m.gain; }
    energy *= 0.9998f; return output * 0.12f;
}
}
