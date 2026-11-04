#include "audio/CymbalEngine.h"
#include <iostream>
int main(){ cymbalforge::CymbalEngine engine; engine.prepare(48000,256); engine.strike(1.0f,.6f); juce::AudioBuffer<float> buffer(2,256); float peak=0; for(int i=0;i<30;++i){engine.render(buffer,0,256); peak=juce::jmax(peak,buffer.getMagnitude(0,256));} if(peak<=0.00001f){std::cerr<<"Engine produced silence\n";return 1;} std::cout<<"CymbalEngine smoke test passed\n"; return 0; }
