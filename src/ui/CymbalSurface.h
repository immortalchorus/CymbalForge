#pragma once
#include "audio/CymbalEngine.h"
namespace cymbalforge { class CymbalSurface : public juce::Component { public: explicit CymbalSurface(CymbalEngine&); juce::String getTooltip() override; void paint(juce::Graphics&) override; void mouseDown(const juce::MouseEvent&) override; private: CymbalEngine& engine; }; }
