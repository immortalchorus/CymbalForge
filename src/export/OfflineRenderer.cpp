#include "OfflineRenderer.h"

namespace cymbalforge {

bool OfflineRenderer::renderWav(const juce::File& file, const CymbalEngine& source, double seconds)
{
    constexpr double rate = 48000;
    const auto samples = int(rate * seconds);
    juce::AudioBuffer<float> buffer(2, samples);
    CymbalEngine engine;
    engine.parameters.copyFrom(source.parameters);
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
