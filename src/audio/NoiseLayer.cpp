#include "NoiseLayer.h"
namespace cymbalforge {
void NoiseLayer::prepare(double sampleRate) { coefficient = std::exp(-1.0f / float(sampleRate * 0.32)); }
void NoiseLayer::strike(float velocity, float brightness, float amount) { envelope = velocity * amount; low = brightness; }
float NoiseLayer::process() { auto n = random.nextFloat() * 2.0f - 1.0f; low = low * 0.995f + n * 0.005f; auto high = n - low; auto out = high * envelope; envelope *= coefficient; return out; }
}
