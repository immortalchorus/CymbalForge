#include "CymbalEngine.h"
namespace cymbalforge {
void CymbalEngine::prepare(double rate, int block) { modal.prepare(rate); noise.prepare(rate); juce::dsp::ProcessSpec s{rate, (juce::uint32) block, 2}; reverb.prepare(s); }
void CymbalEngine::strike(float v, float p) { auto b=parameters.brightness.load(); auto d=parameters.decay.load(); auto c=parameters.complexity.load(); auto s=parameters.shimmer.load(); modal.strike(v,p,b,d,c,s,parameters.modes.load()); noise.strike(v,b,parameters.noise.load(),s); }
void CymbalEngine::render(juce::AudioBuffer<float>& buffer, int start, int count) { buffer.clear(start,count); auto gain=parameters.gain.load(); for(int i=0;i<count;++i) { auto modes=modal.process(); auto residual=noise.process(); buffer.setSample(0,start+i,(modes[0]+residual[0])*gain); buffer.setSample(1,start+i,(modes[1]+residual[1])*gain); } juce::dsp::AudioBlock<float> b(buffer); auto sub=b.getSubBlock((size_t)start,(size_t)count); juce::dsp::ProcessContextReplacing<float> c(sub); juce::dsp::Reverb::Parameters p; p.wetLevel=parameters.reverb.load()*0.35f; p.dryLevel=1.0f; p.roomSize=0.55f; reverb.setParameters(p); reverb.process(c); }
}
