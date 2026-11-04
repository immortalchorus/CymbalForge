#include "ModalBank.h"
#include <cmath>
namespace cymbalforge {
void ModalBank::prepare(double sampleRate) { rate = sampleRate; for (auto& m : modes) m = {}; }
void ModalBank::strike(float velocity, float position, float brightness, float decay, float complexity, float shimmer, int requestedModes) {
    // A cymbal is a dense irregular mode field, not a fundamental with harmonics.
    count = juce::jlimit(64, maxModes, juce::jmax(requestedModes, 128 + int(complexity * float(maxModes - 128))));
    const auto hitRadius = juce::jlimit(0.05f, 0.98f, position);
    const auto nyquistGuard = float(rate * 0.43);
    const auto lowFrequency = 90.0f + 120.0f * (1.0f - brightness);
    for (int i = 0; i < count; ++i) {
        const auto u = (float(i) + random.nextFloat()) / float(count);
        auto f = lowFrequency * std::exp(std::log(nyquistGuard / lowFrequency) * std::pow(u, 0.74f));
        f = juce::jmin(nyquistGuard, f * (1.0f + (random.nextFloat() * 2.0f - 1.0f) * (0.012f + complexity * 0.055f)));
        const auto radialOrder = 1.0f + std::fmod(float(i * 17), 11.0f);
        const auto azimuthOrder = 1.0f + std::fmod(float(i * 7), 13.0f);
        const auto radiusWeight = (0.08f + 0.92f * std::abs(std::sin(juce::MathConstants<float>::pi * radialOrder * hitRadius)) * (0.35f + 0.65f * std::abs(std::sin(azimuthOrder * (0.31f + hitRadius)))));
        const auto spectralTilt = std::pow(1.0f - u, 0.25f + 0.65f * (1.0f - brightness));
        auto seconds = (0.25f + 8.0f * decay) / (0.72f + 1.25f * std::pow(u, 0.55f));
        auto r = std::exp(-1.0f / (seconds * float(rate)));
        auto theta = juce::MathConstants<float>::twoPi * f / float(rate);
        auto phase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        auto pan = (random.nextFloat() * 2.0f - 1.0f) * (0.25f + 0.7f * u);
        auto highBand = std::pow(u, 1.7f);
        auto amplitude = velocity * radiusWeight * spectralTilt / std::sqrt(float(count));
        modes[i] = { 2.0f * r * std::cos(theta), -r * r, amplitude * std::sin(phase), amplitude * std::sin(phase - theta), 1.0f, 1.0f + velocity * shimmer * highBand * 2.6f, std::cos((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f), std::sin((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f), 0.000015f + highBand * (0.00018f + shimmer * 0.00032f) };
    }
    for (int i = count; i < maxModes; ++i) modes[i] = {};
    bloom = 0.0f; energy = velocity;
}
std::array<float, 2> ModalBank::process() {
    bloom = juce::jmin(1.0f, bloom + 0.00004f);
    std::array<float, 2> output{};
    for (int i = 0; i < count; ++i) { auto& m = modes[i]; auto y = m.a*m.z1 + m.b*m.z2; m.z2=m.z1; m.z1=y; auto modeBloom = 1.0f - std::exp(-bloom * m.bloomRate * 10000.0f); auto gain = m.attackGain + (m.bloomGain - m.attackGain) * modeBloom; output[0] += y*gain*m.panL; output[1] += y*gain*m.panR; }
    energy *= 0.99986f; return { output[0] * 0.16f, output[1] * 0.16f };
}
}
