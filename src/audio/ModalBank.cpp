#include "ModalBank.h"
#include <cmath>
namespace cymbalforge {
void ModalBank::prepare(double sampleRate) { rate = sampleRate; for (auto& m : modes) m = {}; }
void ModalBank::strike(const Excitation& excitation) {
    const auto [velocity, position, brightness, decay, complexity, shimmer, swell, masterTune, model, requestedModes] = excitation;
    // A cymbal is a dense irregular mode field, not a fundamental with harmonics.
    count = juce::jlimit(64, maxModes, juce::jmax(requestedModes, 128 + int(complexity * float(maxModes - 128))));
    const auto hitRadius = juce::jlimit(0.05f, 0.98f, position);
    const auto modelIndex = juce::jlimit(0, 2, model);
    // Crash: bright/quick; Ride: heavier and more focused; Trash: irregular and broad.
    const float lowFrequencyByModel[] { 120.0f, 78.0f, 105.0f };
    const float topFrequencyByModel[] { 0.45f, 0.39f, 0.46f };
    const float detuneByModel[] { 1.0f, 0.58f, 1.8f };
    const float decayByModel[] { 1.0f, 1.35f, 0.72f };
    const float distributionByModel[] { 0.74f, 0.90f, 0.63f };
    const auto nyquistGuard = float(rate * topFrequencyByModel[modelIndex]);
    const auto pitchRatio = std::pow(2.0f, masterTune / 12.0f);
    const auto lowFrequency = (lowFrequencyByModel[modelIndex] + 105.0f * (1.0f - brightness)) * pitchRatio;
    for (int i = 0; i < count; ++i) {
        const auto u = (float(i) + random.nextFloat()) / float(count);
        auto f = lowFrequency * std::exp(std::log(nyquistGuard / lowFrequency) * std::pow(u, distributionByModel[modelIndex]));
        f = juce::jmin(nyquistGuard, f * (1.0f + (random.nextFloat() * 2.0f - 1.0f) * detuneByModel[modelIndex] * (0.012f + complexity * 0.055f)));
        const auto radialOrder = 1.0f + std::fmod(float(i * 17), 11.0f);
        const auto azimuthOrder = 1.0f + std::fmod(float(i * 7), 13.0f);
        const auto radiusWeight = (0.08f + 0.92f * std::abs(std::sin(juce::MathConstants<float>::pi * radialOrder * hitRadius)) * (0.35f + 0.65f * std::abs(std::sin(azimuthOrder * (0.31f + hitRadius)))));
        const auto spectralTilt = std::pow(1.0f - u, 0.25f + 0.65f * (1.0f - brightness));
        // Exponential range: 12 ms at zero through long cymbal sustains at one.
        auto seconds = decayByModel[modelIndex] * (0.012f * std::pow(850.0f, decay)) / (0.72f + 1.25f * std::pow(u, 0.55f));
        auto r = std::exp(-1.0f / (seconds * float(rate)));
        auto theta = juce::MathConstants<float>::twoPi * f / float(rate);
        auto phase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        auto pan = (random.nextFloat() * 2.0f - 1.0f) * (0.25f + 0.7f * u);
        auto highBand = std::pow(u, 1.7f);
        auto amplitude = velocity * radiusWeight * spectralTilt / std::sqrt(float(count));
        const auto bloomSeconds = .10f + .32f * (1.0f - highBand);
        const auto bloomStep = 1.0f - std::exp(-1.0f / (bloomSeconds * float(rate)));
        modes[i] = { 2.0f * r * std::cos(theta), -r * r, amplitude * std::sin(phase), amplitude * std::sin(phase - theta), 1.0f - .92f * swell, 1.0f + velocity * shimmer * highBand * 2.6f, std::cos((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f), std::sin((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f), 0.0f, bloomStep };
    }
    for (int i = count; i < maxModes; ++i) modes[i] = {};
    energy = velocity;
}
std::array<float, 2> ModalBank::process() {
    std::array<float, 2> output{};
    for (int i = 0; i < count; ++i) { auto& m = modes[i]; auto y = m.a*m.z1 + m.b*m.z2; m.z2=m.z1; m.z1=y; m.bloom += m.bloomStep * (1.0f - m.bloom); auto gain = m.attackGain + (m.bloomGain - m.attackGain) * m.bloom; output[0] += y*gain*m.panL; output[1] += y*gain*m.panR; }
    energy *= 0.99986f; return { output[0] * 0.16f, output[1] * 0.16f };
}
}
