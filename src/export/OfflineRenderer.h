#pragma once
#include "audio/CymbalEngine.h"
namespace cymbalforge { class OfflineRenderer { public: static bool renderWav(const juce::File&, CymbalParameters&, double seconds = 8.0); }; }
