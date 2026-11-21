#include "GenesisSynth.h"
#include "GenesisFactory100.h"
#include "GenesisChorus.h"
#include "GenesisDelay.h"
#include "GenesisReverb.h"
#include <cmath>
#include <iostream>
#include <set>
#include <string_view>
#include <cstdlib>
void require(bool ok,const char* msg){if(!ok){std::cerr<<msg<<"\n";std::exit(1);}}
int main(){
 using namespace liberty;
 std::set<std::string_view> names;
 for(const auto& p:genesisFactory100){
  require(names.insert(p.name).second,"Duplicate preset");
  GenesisSynth s;
  s.setWaveforms(static_cast<GenesisSynth::Waveform>(p.waveA),static_cast<GenesisSynth::Waveform>(p.waveB));
  s.setOscMix(p.mix);s.setDetuneCents(p.detune);s.setFilter(p.cutoff,p.resonance);s.setADSR(p.attack,p.decay,p.sustain,p.release);s.setLFO(p.lfoRate,p.lfoDepth);s.setGain(p.gain);s.setFilterMode(static_cast<GenesisSynth::FilterMode>(int(p.filterMode)));s.setFilterDrive(p.filterDrive*.01f);
  s.noteOn(60,.8f);float peak=0;
  for(int i=0;i<48000;++i){float l,r;s.processStereo(l,r);require(std::isfinite(l)&&std::isfinite(r),"Nonfinite factory audio");peak=std::max(peak,std::abs(l));}
  require(peak>1e-7f&&peak<=1.f,"Factory signal level");std::cout<<p.name<<" peak="<<peak<<"\n";
 }
 require(names.size()==100,"Factory count");
 for(int slot=0;slot<16;++slot){
  GenesisSynth a,b;a.setMatrix(slot,5,1,.25f,false);a.noteOn(60,1);b.noteOn(60,1);double diff=0;
  for(int i=0;i<4096;++i)diff+=std::abs(a.processSample()-b.processSample());
  require(diff>.01,"Inactive modulation slot");
 }
 GenesisSynth s;GenesisChorus c;GenesisDelay d;GenesisReverb r;
 c.prepare(48000);d.prepare(48000);r.prepare(48000);
 c.set(.5f,.5f,.5f,1);d.set(.5f,.25,.5f,.5f,.5f);r.set(2,.5f,20,3,.5f,1);
 s.setUnison(7,25,1,.5f,2,true);for(int n=48;n<64;++n)s.noteOn(n,.8f);
 for(int i=0;i<48000*10;++i){float l,h,a,b;s.processStereo(l,h);c.processStereo(l,h,a,b);d.process(a,b);r.process(a,b);require(std::isfinite(a)&&std::isfinite(b),"Nonfinite polyphonic effects");require(std::abs(a)<2&&std::abs(b)<2,"Excessive stress level");}
 std::cout<<"100 base presets, 16 matrix slots, 10s polyphonic stress passed\n";
}
