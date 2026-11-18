#include "../src/GenesisAudioEngine.h"
#include "../src/GenesisPresetBrowser.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
int main(){
 constexpr int block=256;
 std::array<float,block> l{},r{};
 liberty::GenesisAudioEngine engine;
 engine.synth().setSampleRate(48000.);
 liberty::GenesisPresetBrowser browser;
 assert(browser.count()==12);
 for(std::size_t p=0;p<browser.count();++p){
  assert(browser.load(engine.synth(),p));
  engine.synth().allNotesOff();
  for(int note=36;note<=96;note+=12){
   const liberty::MidiEvent events[]={{0,liberty::MidiEvent::Type::NoteOn,static_cast<unsigned char>(note),.8f}};
   engine.process(l.data(),r.data(),block,events,1);
   double peak=0.;
   for(int i=0;i<block;++i){
    assert(std::isfinite(l[i]));
    assert(l[i]==r[i]);
    assert(std::abs(l[i])<=1.f);
    peak=std::max(peak,double(std::abs(l[i])));
   }
   assert(peak>1.e-7);
   engine.synth().allNotesOff();
  }
 }
 // Verify that all-notes-off takes effect at the exact sample boundary.
 engine.synth().allNotesOff();
 const liberty::MidiEvent timed[]={
  {64,liberty::MidiEvent::Type::NoteOn,69,.8f},
  {192,liberty::MidiEvent::Type::AllNotesOff,0,0.f}
 };
 engine.process(l.data(),r.data(),block,timed,2);
 for(int i=0;i<64;++i)assert(l[i]==0.f);
 for(int i=192;i<block;++i)assert(l[i]==0.f);
 // Zero-length and null buffers must not crash.
 engine.process(nullptr,r.data(),block,timed,2);
 engine.process(l.data(),nullptr,block,timed,2);
 engine.process(l.data(),r.data(),0,timed,2);
 std::cout<<"Genesis DSP regression passed: 12 presets, 6 note ranges, MIDI boundaries\n";
}
