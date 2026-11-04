#pragma once
#include <JuceHeader.h>
namespace cymbalforge {
struct NoiseLayer { void prepare(double sampleRate); void strike(float velocity, float brightness, float amount); float process(); private: juce::Random random; float envelope=0, coefficient=0.99f, low=0; };
}
