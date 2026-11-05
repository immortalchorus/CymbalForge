#pragma once
#include <JuceHeader.h>
namespace cymbalforge {
struct NoiseLayer { void prepare(double sampleRate); void strike(float velocity, float brightness, float amount, float tone, float shimmer, int model, int exciter); std::array<float, 2> process(); private: juce::Random random { 0x52455349 }; float contactEnvelope=0, washEnvelope=0, contactDecay=0.99f, washDecay=0.999f, slow=0, medium=0, brightness=0.5f, tone=.5f, washColour=1.0f; };
}
