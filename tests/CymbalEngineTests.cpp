#include "audio/CymbalEngine.h"
#include "audio/ImpactVoiceCore.h"
#include "presets/PresetManager.h"
#include <cmath>
#include <iostream>

namespace {
struct Metrics { float peak{}, left{}, right{}; };

Metrics renderHit(int model, float position)
{
    cymbalforge::CymbalEngine engine;
    engine.prepare(48000, 256);
    engine.parameters.model.store(model); engine.parameters.complexity.store(.85f); engine.parameters.shimmer.store(.75f);
    engine.strike(.95f, position);
    juce::AudioBuffer<float> buffer(2, 256);
    Metrics result;
    for (int block = 0; block < 80; ++block) {
        engine.render(buffer, 0, 256);
        for (int channel = 0; channel < 2; ++channel) for (int sample = 0; sample < 256; ++sample) {
            const auto value = buffer.getSample(channel, sample);
            if (! std::isfinite(value)) return {};
            result.peak = juce::jmax(result.peak, std::abs(value));
            (channel == 0 ? result.left : result.right) += value * value;
        }
    }
    return result;
}

bool testCymbalFamilies()
{
    const auto crashBell = renderHit(0, .15f), crashEdge = renderHit(0, .88f), darkRide = renderHit(1, .60f), trash = renderHit(2, .60f);
    const auto valid = [] (const Metrics& m) { return m.peak > .00001f && m.peak < 8.0f && m.left > 0 && m.right > 0; };
    if (! valid(crashBell) || ! valid(crashEdge) || ! valid(darkRide) || ! valid(trash)) { std::cerr << "Engine produced invalid audio\n"; return false; }
    if (std::abs(crashBell.left - crashEdge.left) < .001f) { std::cerr << "Strike position did not change response\n"; return false; }
    if (std::abs(darkRide.left - trash.left) < .001f) { std::cerr << "Cymbal family did not change response\n"; return false; }
    return true;
}

bool testSampleLoopAndRelease()
{
    cymbalforge::ImpactVoiceCore core;
    core.prepare(48000);
    auto sample = std::make_shared<cymbalforge::ImpactSampleData>();
    sample->audio.setSize(2, 64);
    for (int i = 0; i < 64; ++i) { sample->audio.setSample(0, i, .4f); sample->audio.setSample(1, i, .4f); }
    core.setSample(0, sample);
    cymbalforge::ImpactControls controls;
    controls.decay = 0; controls.noise = .6f; controls.noiseDecay = .8f; controls.sub2Level = .5f; controls.sub2Decay = 0;
    controls.filterMode = 3; controls.filterCutoff = .7f; controls.filterEnvAmount = .7f; controls.filterEnvDecay = .3f;
    controls.samples[0].level = 1.0f; controls.samples[0].loop = true; controls.samples[0].release = .02f;
    core.trigger(1.0f, .5f, controls);
    for (int i = 0; i < 120000; ++i) { const auto rendered = core.render(); if (! std::isfinite(rendered[0]) || ! std::isfinite(rendered[1])) { std::cerr << "Impact core produced invalid expanded output\n"; return false; } }
    if (! core.samplesActive()) { std::cerr << "Looping sample did not sustain\n"; return false; }
    core.release();
    for (int i = 0; i < 120000; ++i) core.render();
    if (core.samplesActive()) { std::cerr << "Released looping sample remained active\n"; return false; }
    return true;
}

bool testPresetRoundTrip()
{
    cymbalforge::CymbalEngine source, restored;
    source.parameters.swell = .61f; source.parameters.noiseDecay = .72f; source.parameters.sub2Level = .48f;
    source.parameters.filterEnvAmount = .63f; source.parameters.filterEnvDecay = .27f; source.parameters.exciter = 3;
    source.parameters.sampleLoop[2] = 1; source.parameters.sampleRelease[2] = .42f;
    const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("cymbalforge-preset-regression.json");
    const auto saved = cymbalforge::PresetManager::save(file, source);
    const auto loaded = saved && cymbalforge::PresetManager::load(file, restored);
    file.deleteFile();
    const auto same = [] (float a, float b) { return std::abs(a - b) < .0001f; };
    if (! loaded || ! same(restored.parameters.swell.load(), .61f) || ! same(restored.parameters.noiseDecay.load(), .72f) || ! same(restored.parameters.sub2Level.load(), .48f) || ! same(restored.parameters.filterEnvAmount.load(), .63f) || restored.parameters.exciter.load() != 3 || restored.parameters.sampleLoop[2].load() != 1 || ! same(restored.parameters.sampleRelease[2].load(), .42f)) { std::cerr << "Preset round-trip lost impact controls\n"; return false; }
    return true;
}
}

int main()
{
    if (! testCymbalFamilies() || ! testSampleLoopAndRelease() || ! testPresetRoundTrip()) return 1;
    std::cout << "CymbalForge impact regression tests passed\n";
    return 0;
}
