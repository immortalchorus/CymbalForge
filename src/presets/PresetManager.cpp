#include "PresetManager.h"

namespace cymbalforge {
namespace {
template <typename Callback>
void visitFloatParameters(CymbalParameters& parameters, Callback&& visit)
{
    visit("velocity", parameters.velocity); visit("position", parameters.position); visit("variation", parameters.variation); visit("velocityTone", parameters.velocityTone);
    visit("brightness", parameters.brightness); visit("decay", parameters.decay); visit("choke", parameters.choke); visit("complexity", parameters.complexity); visit("shimmer", parameters.shimmer); visit("swell", parameters.swell); visit("noise", parameters.noise); visit("noiseTone", parameters.noiseTone); visit("noiseDecay", parameters.noiseDecay);
    visit("subLevel", parameters.subLevel); visit("subTune", parameters.subTune); visit("subDecay", parameters.subDecay); visit("subDelay", parameters.subDelay); visit("sub2Level", parameters.sub2Level); visit("sub2Tune", parameters.sub2Tune); visit("sub2Decay", parameters.sub2Decay); visit("sub2Delay", parameters.sub2Delay);
    visit("filterCutoff", parameters.filterCutoff); visit("filterResonance", parameters.filterResonance); visit("filterEnvAmount", parameters.filterEnvAmount); visit("filterEnvDecay", parameters.filterEnvDecay); visit("modLfoRate", parameters.modLfoRate); visit("modLfoDepth", parameters.modLfoDepth); visit("tempoBpm", parameters.tempoBpm);
    visit("transient", parameters.transient); visit("drive", parameters.drive); visit("width", parameters.width); visit("limiter", parameters.limiter); visit("reverb", parameters.reverb); visit("gain", parameters.gain);
}

template <typename Callback>
void visitFloatParameters(const CymbalParameters& parameters, Callback&& visit)
{
    visit("velocity", parameters.velocity); visit("position", parameters.position); visit("variation", parameters.variation); visit("velocityTone", parameters.velocityTone);
    visit("brightness", parameters.brightness); visit("decay", parameters.decay); visit("choke", parameters.choke); visit("complexity", parameters.complexity); visit("shimmer", parameters.shimmer); visit("swell", parameters.swell); visit("noise", parameters.noise); visit("noiseTone", parameters.noiseTone); visit("noiseDecay", parameters.noiseDecay);
    visit("subLevel", parameters.subLevel); visit("subTune", parameters.subTune); visit("subDecay", parameters.subDecay); visit("sub2Level", parameters.sub2Level); visit("sub2Tune", parameters.sub2Tune); visit("sub2Decay", parameters.sub2Decay); visit("sub2Delay", parameters.sub2Delay); visit("subDelay", parameters.subDelay);
    visit("filterCutoff", parameters.filterCutoff); visit("filterResonance", parameters.filterResonance); visit("filterEnvAmount", parameters.filterEnvAmount); visit("filterEnvDecay", parameters.filterEnvDecay); visit("modLfoRate", parameters.modLfoRate); visit("modLfoDepth", parameters.modLfoDepth); visit("tempoBpm", parameters.tempoBpm);
    visit("transient", parameters.transient); visit("drive", parameters.drive); visit("width", parameters.width); visit("limiter", parameters.limiter); visit("reverb", parameters.reverb); visit("gain", parameters.gain);
}

juce::String sampleKey(int slot, const char* suffix) { return "sample" + juce::String(slot + 1) + suffix; }
}

bool PresetManager::save(const juce::File& file, const CymbalEngine& engine)
{
    const auto& parameters = engine.parameters;
    auto object = new juce::DynamicObject;
    visitFloatParameters(parameters, [object] (const char* key, const std::atomic<float>& value) { object->setProperty(key, value.load()); });
    for (int slot = 0; slot < 4; ++slot) {
        object->setProperty(sampleKey(slot, "Level"), parameters.sampleLevel[(size_t) slot].load()); object->setProperty(sampleKey(slot, "Tune"), parameters.sampleTune[(size_t) slot].load()); object->setProperty(sampleKey(slot, "Start"), parameters.sampleStart[(size_t) slot].load()); object->setProperty(sampleKey(slot, "Pan"), parameters.samplePan[(size_t) slot].load());
        object->setProperty(sampleKey(slot, "Attack"), parameters.sampleAttack[(size_t) slot].load()); object->setProperty(sampleKey(slot, "Decay"), parameters.sampleDecay[(size_t) slot].load()); object->setProperty(sampleKey(slot, "Sustain"), parameters.sampleSustain[(size_t) slot].load()); object->setProperty(sampleKey(slot, "Release"), parameters.sampleRelease[(size_t) slot].load());
        object->setProperty(sampleKey(slot, "Delay"), parameters.sampleDelay[(size_t) slot].load()); object->setProperty(sampleKey(slot, "LfoRate"), parameters.sampleLfoRate[(size_t) slot].load()); object->setProperty(sampleKey(slot, "LfoDepth"), parameters.sampleLfoDepth[(size_t) slot].load());
        object->setProperty(sampleKey(slot, "Loop"), parameters.sampleLoop[(size_t) slot].load());
        object->setProperty(sampleKey(slot, "Reverse"), parameters.sampleReverse[(size_t) slot].load());
        object->setProperty(sampleKey(slot, "Path"), engine.sampleFile(slot).getFullPathName());
    }
    object->setProperty("modes", parameters.modes.load()); object->setProperty("voiceLimit", parameters.voiceLimit.load()); object->setProperty("model", parameters.model.load()); object->setProperty("filterMode", parameters.filterMode.load()); object->setProperty("modTarget", parameters.modTarget.load()); object->setProperty("lfoSync", parameters.lfoSync.load()); object->setProperty("exciter", parameters.exciter.load()); object->setProperty("midiPositionMap", parameters.midiPositionMap.load());
    return file.replaceWithText(juce::JSON::toString(juce::var(object), true));
}

bool PresetManager::load(const juce::File& file, CymbalEngine& engine)
{
    const auto value = juce::JSON::parse(file);
    if (! value.isObject()) return false;
    const auto* object = value.getDynamicObject();
    auto& parameters = engine.parameters;
    visitFloatParameters(parameters, [object] (const char* key, std::atomic<float>& target) { if (object->hasProperty(key)) target.store(float(object->getProperty(key))); });
    for (int slot = 0; slot < 4; ++slot) {
        auto get = [object, slot] (const char* suffix, std::atomic<float>& target) { const auto key = sampleKey(slot, suffix); if (object->hasProperty(key)) target.store(float(object->getProperty(key))); };
        get("Level", parameters.sampleLevel[(size_t) slot]); get("Tune", parameters.sampleTune[(size_t) slot]); get("Start", parameters.sampleStart[(size_t) slot]); get("Pan", parameters.samplePan[(size_t) slot]); get("Attack", parameters.sampleAttack[(size_t) slot]); get("Decay", parameters.sampleDecay[(size_t) slot]); get("Sustain", parameters.sampleSustain[(size_t) slot]); get("Release", parameters.sampleRelease[(size_t) slot]); get("Delay", parameters.sampleDelay[(size_t) slot]); get("LfoRate", parameters.sampleLfoRate[(size_t) slot]); get("LfoDepth", parameters.sampleLfoDepth[(size_t) slot]);
        const auto loopKey = sampleKey(slot, "Loop"); if (object->hasProperty(loopKey)) parameters.sampleLoop[(size_t) slot].store(int(object->getProperty(loopKey)));
        const auto reverseKey = sampleKey(slot, "Reverse"); if (object->hasProperty(reverseKey)) parameters.sampleReverse[(size_t) slot].store(int(object->getProperty(reverseKey)));
        const auto path = object->getProperty(sampleKey(slot, "Path")).toString();
        if (path.isNotEmpty()) engine.loadSampleSlot(slot, juce::File(path)); else engine.clearSampleSlot(slot);
    }
    if (object->hasProperty("modes")) parameters.modes.store(int(object->getProperty("modes"))); if (object->hasProperty("voiceLimit")) parameters.voiceLimit.store(int(object->getProperty("voiceLimit"))); if (object->hasProperty("model")) parameters.model.store(int(object->getProperty("model"))); if (object->hasProperty("filterMode")) parameters.filterMode.store(int(object->getProperty("filterMode"))); if (object->hasProperty("modTarget")) parameters.modTarget.store(int(object->getProperty("modTarget"))); if (object->hasProperty("lfoSync")) parameters.lfoSync.store(int(object->getProperty("lfoSync"))); if (object->hasProperty("exciter")) parameters.exciter.store(int(object->getProperty("exciter"))); if (object->hasProperty("midiPositionMap")) parameters.midiPositionMap.store(int(object->getProperty("midiPositionMap")));
    return true;
}
}
