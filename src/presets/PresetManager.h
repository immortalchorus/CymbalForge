#pragma once
#include "audio/CymbalEngine.h"
namespace cymbalforge { class PresetManager { public: static bool save(const juce::File&, const CymbalEngine&); static bool load(const juce::File&, CymbalEngine&); }; }
