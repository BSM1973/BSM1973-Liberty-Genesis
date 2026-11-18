#pragma once
#include "GenesisSynth.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace liberty {
// Host-independent MIDI event representation. The host must provide sorted
// sample offsets in [0, frames). No allocations or locks in process().
struct MidiEvent {
 enum class Type : uint8_t { NoteOn, NoteOff, AllNotesOff };
 int sampleOffset=0;
 Type type=Type::NoteOff;
 uint8_t note=0;
 float velocity=0.f;
};
class GenesisAudioEngine {
public:
 GenesisSynth& synth(){return instrument;}
 const GenesisSynth& synth()const{return instrument;}
 // Audio buffers must not alias. Events must be sorted by sampleOffset.
 // Invalid event offsets are ignored; events with equal offsets keep input order.
 void process(float* left,float* right,int frames,const MidiEvent* events,std::size_t count){
  if(!left||!right||frames<=0)return;
  if(!events)count=0;
  std::size_t next=0;
  for(int sample=0;sample<frames;++sample){
   while(next<count && events[next].sampleOffset<sample)++next;
   while(next<count && events[next].sampleOffset==sample){
    const auto& e=events[next++];
    switch(e.type){
     case MidiEvent::Type::NoteOn:
      if(e.velocity>0.f)instrument.noteOn(e.note,e.velocity);
      else instrument.noteOff(e.note);
      break;
     case MidiEvent::Type::NoteOff:instrument.noteOff(e.note);break;
     case MidiEvent::Type::AllNotesOff:instrument.allNotesOff();break;
    }
   }
   left[sample]=right[sample]=instrument.processSample();
  }
 }
private:
 GenesisSynth instrument;
};
}
