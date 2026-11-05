#pragma once
#include "ModalBank.h"
#include "NoiseLayer.h"

namespace cymbalforge {
// UI-free, allocation-free per-voice impact renderer. This is intentionally
// shaped for later adoption by SpaceAge's Voice/renderSelectedEngineVoice path.
struct ImpactControls {
    float brightness = .65f, decay = .35f, choke = 0.0f, complexity = .55f, shimmer = .45f, noise = .25f;
    float subLevel = 0.0f, subTune = .35f, subDecay = .35f, filterCutoff = 1.0f, filterMix = 0.0f;
    int modes = 384, model = 0;
};

class ImpactVoiceCore {
public:
    void prepare (double sampleRate);
    void trigger (float velocity, float position, const ImpactControls& controls);
    std::array<float, 2> render();
private:
    ModalBank modal;
    NoiseLayer noise;
    double rate = 48000.0;
    float subPhase = 0, subEnvelope = 0, subDecayCoefficient = .999f;
    float filterStateL = 0, filterStateR = 0, filterCoefficient = 1, filterMix = 0;
    float subHz = 80.0f;
};
}
