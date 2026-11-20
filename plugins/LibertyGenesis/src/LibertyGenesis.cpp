#include "mpc_plugin.h"
#include "GenesisSynth.h"
#include <algorithm>
#include <array>

class LibertyGenesis final : public mpc::Plugin {
public:
 enum Params { kWaveA, kWaveB, kMix, kDetune, kCutoff, kResonance,
               kVolume, kAttack, kDecay, kSustain, kRelease,
               kLfoRate, kLfoDepth };
 static mpc::PluginInfo makeInfo() {
  mpc::PluginInfo i;
  i.name = "Liberty Genesis";
  i.vendor = "Liberty Music Software";
  i.uniqueId = MPC_FOURCC('L','G','n','s');
  i.version = 100;
  i.isInstrument = true;
  i.numInputs = 0;
  i.numOutputs = 2;
  return i;
 }
 LibertyGenesis() : mpc::Plugin(makeInfo()) {
  addParam(mpc::choiceParam("wavea","Wave A",{"Sine","Saw","Square","Triangle"},1));
  addParam(mpc::choiceParam("waveb","Wave B",{"Sine","Saw","Square","Triangle"},2));
  addParam(mpc::floatParam("mix","Mix",0.f,100.f,50.f,"%",0));
  addParam(mpc::floatParam("detune","Detune",-100.f,100.f,7.f,"ct",0));
  addParam(mpc::logParam("cutoff","Cutoff",20.f,18000.f,8000.f,"Hz",0));
  addParam(mpc::floatParam("reso","Resonance",0.f,95.f,10.f,"%",0));
  addParam(mpc::floatParam("volume","Volume",0.f,100.f,80.f,"%",0));
  addParam(mpc::logParam("attack","Attack",1.f,10000.f,8.f,"ms",0));
  addParam(mpc::logParam("decay","Decay",1.f,10000.f,100.f,"ms",0));
  addParam(mpc::floatParam("sustain","Sustain",0.f,100.f,80.f,"%",0));
  addParam(mpc::logParam("release","Release",1.f,20000.f,180.f,"ms",0));
  addParam(mpc::logParam("lforate","LFO Rate",0.05f,20.f,0.5f,"Hz",2));
  addParam(mpc::floatParam("lfodepth","LFO Depth",0.f,100.f,0.f,"%",0));
 }
 void prepare(double sampleRate, int) override { synth_.setSampleRate(sampleRate); synth_.allNotesOff(); }
 void reset() override { synth_.allNotesOff(); }
 void process(mpc::ProcessContext& ctx) override {
  if (ctx.numOutputs < 1 || !ctx.outputs || !ctx.outputs[0]) return;
  synth_.setWaveforms(static_cast<liberty::GenesisSynth::Waveform>(getInt(kWaveA)),
                      static_cast<liberty::GenesisSynth::Waveform>(getInt(kWaveB)));
  synth_.setOscMix(get(kMix)*.01f);
  synth_.setDetuneCents(get(kDetune));
  synth_.setFilter(get(kCutoff),get(kResonance)*.01f);
  synth_.setGain(get(kVolume)*.01f);
  synth_.setADSR(get(kAttack)*.001f,get(kDecay)*.001f,get(kSustain)*.01f,get(kRelease)*.001f);
  synth_.setLFO(get(kLfoRate),get(kLfoDepth)*.01f);
  float* left = ctx.outputs[0];
  float* right = ctx.numOutputs > 1 ? ctx.outputs[1] : nullptr;
  int pos = 0;
  for(int i=0;i<ctx.numMidiEvents;++i) {
   const auto& ev=ctx.midi[i];
   const int offset=std::clamp(ev.sampleOffset,pos,ctx.numFrames);
   render(left,right,pos,offset);
   pos=offset;
   if(ev.isNoteOn()) synth_.noteOn(ev.note(),ev.velocity()/127.f);
   else if(ev.isNoteOff()) synth_.noteOff(ev.note());
   else if(ev.isAllNotesOff()) synth_.allNotesOff();
  }
  render(left,right,pos,ctx.numFrames);
 }
private:
 void render(float* l,float* r,int begin,int end) {
  for(int i=begin;i<end;++i) { const float s=synth_.processSample(); l[i]=s; if(r)r[i]=s; }
 }
 liberty::GenesisSynth synth_{};
};
MPC_PLUGIN_ENTRY(LibertyGenesis)
