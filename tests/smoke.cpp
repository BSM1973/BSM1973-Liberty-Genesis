#include "../src/GenesisSynth.h"
#include "../src/GenesisPresets.h"
#include <array>
#include <cassert>
#include <cmath>
int main(){
 liberty::GenesisSynth synth;
 synth.setSampleRate(48000.);
 synth.setWaveforms(liberty::GenesisSynth::Waveform::Saw,liberty::GenesisSynth::Waveform::Square);
 synth.setOscMix(.4f);
 synth.setDetuneCents(12.f);
 synth.setFilter(1500.f,.5f);
 synth.setADSR(.01f,.1f,.7f,.1f);
 synth.setLFO(3.f,.2f);
 std::array<float,512> left{},right{};
 synth.noteOn(69,.9f);
 synth.process(left.data(),right.data(),512);
 bool audible=false;
 for(size_t i=0;i<left.size();++i){assert(std::isfinite(left[i]));assert(left[i]==right[i]);if(std::abs(left[i])>.00001f)audible=true;}
 assert(audible);
 synth.noteOff(69);
 for(int i=0;i<100;++i)synth.process(left.data(),right.data(),512);
 for(float x:left)assert(std::abs(x)<.00001f);
 synth.allNotesOff();
 for(const auto& preset:liberty::factoryPresets){
  liberty::applyPreset(synth,preset);
  synth.noteOn(60,.8f);
  synth.process(left.data(),right.data(),512);
  for(float sample:left)assert(std::isfinite(sample));
  synth.allNotesOff();
 }
}
