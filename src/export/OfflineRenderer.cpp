#include "OfflineRenderer.h"

namespace cymbalforge {
namespace {
void copyParameters(const CymbalParameters& source, CymbalParameters& destination)
{
    destination.velocity = source.velocity.load(); destination.position = source.position.load(); destination.variation = source.variation.load();
    destination.brightness = source.brightness.load(); destination.decay = source.decay.load(); destination.choke = source.choke.load(); destination.complexity = source.complexity.load(); destination.shimmer = source.shimmer.load(); destination.noise = source.noise.load(); destination.noiseTone = source.noiseTone.load();
    destination.subLevel = source.subLevel.load(); destination.subTune = source.subTune.load(); destination.subDecay = source.subDecay.load(); destination.subDelay = source.subDelay.load();
    destination.filterCutoff = source.filterCutoff.load(); destination.filterResonance = source.filterResonance.load(); destination.modLfoRate = source.modLfoRate.load(); destination.modLfoDepth = source.modLfoDepth.load(); destination.tempoBpm = source.tempoBpm.load();
    destination.drive = source.drive.load(); destination.transient = source.transient.load(); destination.width = source.width.load(); destination.limiter = source.limiter.load(); destination.reverb = source.reverb.load(); destination.gain = source.gain.load();
    destination.filterMode = source.filterMode.load(); destination.modTarget = source.modTarget.load(); destination.lfoSync = source.lfoSync.load(); destination.modes = source.modes.load(); destination.model = source.model.load();
    for (int slot = 0; slot < 4; ++slot) {
        destination.sampleLevel[(size_t) slot] = source.sampleLevel[(size_t) slot].load(); destination.sampleTune[(size_t) slot] = source.sampleTune[(size_t) slot].load(); destination.sampleAttack[(size_t) slot] = source.sampleAttack[(size_t) slot].load(); destination.sampleDecay[(size_t) slot] = source.sampleDecay[(size_t) slot].load(); destination.sampleSustain[(size_t) slot] = source.sampleSustain[(size_t) slot].load();
        destination.sampleRelease[(size_t) slot] = source.sampleRelease[(size_t) slot].load(); destination.sampleDelay[(size_t) slot] = source.sampleDelay[(size_t) slot].load(); destination.sampleLfoRate[(size_t) slot] = source.sampleLfoRate[(size_t) slot].load(); destination.sampleLfoDepth[(size_t) slot] = source.sampleLfoDepth[(size_t) slot].load();
    }
}
}

bool OfflineRenderer::renderWav(const juce::File& file, const CymbalEngine& source, double seconds)
{
    constexpr double rate = 48000;
    const auto samples = int(rate * seconds);
    juce::AudioBuffer<float> buffer(2, samples);
    CymbalEngine engine;
    copyParameters(source.parameters, engine.parameters);
    engine.prepare(rate, 512);
    for (int slot = 0; slot < 4; ++slot) {
        const auto sample = source.sampleFile(slot);
        if (sample.existsAsFile() && ! engine.loadSampleSlot(slot, sample)) return false;
    }
    engine.strike(engine.parameters.velocity.load(), engine.parameters.position.load());
    for (int at = 0; at < samples; at += 512) engine.render(buffer, at, juce::jmin(512, samples - at));
    file.deleteFile();
    auto stream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream());
    if (! stream) return false;
    juce::WavAudioFormat wav;
    auto writer = std::unique_ptr<juce::AudioFormatWriter>(wav.createWriterFor(stream.release(), rate, 2, 24, {}, 0));
    return writer && writer->writeFromAudioSampleBuffer(buffer, 0, samples);
}
}
