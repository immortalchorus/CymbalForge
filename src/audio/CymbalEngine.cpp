#include "CymbalEngine.h"
namespace cymbalforge {
void CymbalEngine::prepare(double rate, int block) { modal.prepare(rate); noise.prepare(rate); juce::dsp::ProcessSpec s{rate, (juce::uint32) block, 2}; reverb.prepare(s); }
void CymbalEngine::strike(float v, float p) { auto b=parameters.brightness.load(); auto d=parameters.decay.load(); modal.strike(v,p,b,d,parameters.modes.load()); noise.strike(v,b,parameters.noise.load()); }
void CymbalEngine::render(juce::AudioBuffer<float>& buffer, int start, int count) { buffer.clear(start,count); auto gain=parameters.gain.load(); for(int i=0;i<count;++i) { auto dry=(modal.process()+noise.process())*gain; buffer.setSample(0,start+i,dry); buffer.setSample(1,start+i,dry*(0.96f+0.04f*std::sin(float(i)))); } juce::dsp::AudioBlock<float> b(buffer); auto sub=b.getSubBlock((size_t)start,(size_t)count); juce::dsp::ProcessContextReplacing<float> c(sub); juce::dsp::Reverb::Parameters p; p.wetLevel=parameters.reverb.load()*0.35f; p.dryLevel=1.0f; p.roomSize=0.55f; reverb.setParameters(p); reverb.process(c); }
}
