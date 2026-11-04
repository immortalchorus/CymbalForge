#pragma once
#include "audio/CymbalEngine.h"
namespace cymbalforge { class PresetManager { public: static bool save(const juce::File&, const CymbalParameters&); static bool load(const juce::File&, CymbalParameters&); }; }
