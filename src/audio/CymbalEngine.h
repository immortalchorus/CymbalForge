#pragma once
#include "ModalBank.h"
#include "NoiseLayer.h"
#include <atomic>
namespace cymbalforge {
struct CymbalParameters { std::atomic<float> velocity{0.8f}, position{0.65f}, brightness{0.65f}, decay{0.6f}, complexity{0.55f}, shimmer{0.45f}, noise{0.25f}, reverb{0.2f}, gain{0.75f}; std::atomic<int> modes{384}, model{0}; };
class CymbalEngine { public: CymbalParameters parameters; void prepare(double rate, int block); void strike(float velocity, float position); void render(juce::AudioBuffer<float>& buffer, int start, int count); private: ModalBank modal; NoiseLayer noise; juce::dsp::Reverb reverb; };
}
