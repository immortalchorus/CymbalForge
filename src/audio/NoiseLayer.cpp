#include "NoiseLayer.h"
namespace cymbalforge {
void NoiseLayer::prepare(double sampleRate) { contactDecay = std::exp(-1.0f / float(sampleRate * 0.018)); washDecay = std::exp(-1.0f / float(sampleRate * 0.65)); }
void NoiseLayer::strike(float velocity, float newBrightness, float amount, float shimmer, int model) { brightness = newBrightness; const float washByModel[] { 1.0f, 0.55f, 1.55f }; washColour = washByModel[juce::jlimit(0, 2, model)]; contactEnvelope = juce::jmax(contactEnvelope, velocity * amount * (0.3f + 0.7f * newBrightness)); washEnvelope = juce::jmax(washEnvelope, velocity * amount * washColour * (0.08f + shimmer * 0.42f)); }
std::array<float, 2> NoiseLayer::process() { auto white = random.nextFloat() * 2.0f - 1.0f; slow += 0.012f * (white - slow); medium += (0.07f + brightness * 0.12f) * (white - medium); auto metallicBand = medium - slow; auto air = white - medium; auto mono = air * contactEnvelope + metallicBand * washEnvelope; contactEnvelope *= contactDecay; washEnvelope *= washDecay; auto side = (random.nextFloat() * 2.0f - 1.0f) * washEnvelope * 0.10f * washColour; return { mono + side, mono - side }; }
}
