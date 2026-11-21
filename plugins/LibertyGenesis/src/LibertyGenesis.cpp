#include "mpc_plugin.h"
#include "GenesisSynth.h"
#include "GenesisFactory100.h"
#include "GenesisChorus.h"
#include "GenesisDelay.h"
#include <string_view>
#include <algorithm>
#include <array>

class LibertyGenesis final : public mpc::Plugin {
public:
 enum Params { kWaveA, kWaveB, kMix, kDetune, kCutoff, kResonance,
               kVolume, kAttack, kDecay, kSustain, kRelease,
               kLfoRate, kLfoDepth, kFilterMode, kFilterDrive, kMacroTone, kMacroMotion, kMacroPunch, kSubLevel, kVelocityFilter, kChorusMix, kChorusRate, kChorusDepth, kChorusWidth, kDelayMix, kDelayTime, kDelaySync, kDelayFeedback, kDelayStereo, kDelayTone };
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
  addParam(mpc::choiceParam("filtermode","Filter Mode",{"Low Pass","Band Pass","High Pass"},0));
  addParam(mpc::floatParam("filterdrive","Filter Drive",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("macrotone","Macro Tone",0.f,100.f,50.f,"%",0));
  addParam(mpc::floatParam("macromotion","Macro Motion",0.f,100.f,50.f,"%",0));
  addParam(mpc::floatParam("macropunch","Macro Punch",0.f,100.f,50.f,"%",0));
  addParam(mpc::floatParam("sublevel","Sub Level",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("velfilter","Velocity Filter",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("chorusmix","Chorus Mix",0.f,100.f,0.f,"%",0));
  addParam(mpc::logParam("chorusrate","Chorus Rate",0.05f,5.f,0.35f,"Hz",2));
  addParam(mpc::floatParam("chorusdepth","Chorus Depth",0.f,100.f,45.f,"%",0));
  addParam(mpc::floatParam("choruswidth","Chorus Width",0.f,100.f,100.f,"%",0));
  addParam(mpc::floatParam("delaymix","Delay Mix",0.f,100.f,0.f,"%",0));
  addParam(mpc::logParam("delaytime","Delay Time",20.f,1800.f,350.f,"ms",0));
  addParam(mpc::choiceParam("delaysync","Delay Sync",{"Free","1/16","1/8","1/8 D","1/4","1/4 D","1/2"},0));
  addParam(mpc::floatParam("delayfeedback","Feedback",0.f,85.f,32.f,"%",0));
  addParam(mpc::floatParam("delaystereo","Cross Feed",0.f,100.f,75.f,"%",0));
  addParam(mpc::floatParam("delaytone","Delay Tone",0.f,100.f,55.f,"%",0));
  // MPC native program browser: 100 categorized factory programs.
  for (const auto& preset : liberty::genesisFactory100) {
   const std::string_view name(preset.name);
   const float chorusMix=name.substr(0,3)=="PAD"?45.f:name.substr(0,4)=="POLY"?30.f:name.substr(0,4)=="KEYS"?20.f:0.f;
   std::vector<float> values = {
    static_cast<float>(preset.waveA), static_cast<float>(preset.waveB),
    preset.mix*100.f, preset.detune, preset.cutoff, preset.resonance*100.f,
    preset.gain*100.f, preset.attack*1000.f, preset.decay*1000.f,
    preset.sustain*100.f, preset.release*1000.f,
    preset.lfoRate, preset.lfoDepth*100.f,
    preset.filterMode, preset.filterDrive, 50.f, 50.f, 50.f, 0.f, 0.f, chorusMix, 0.35f, 45.f, 100.f, 0.f, 350.f, 0.f, 32.f, 75.f, 55.f
   };
   addProgram(preset.name, std::move(values));
  }
 }
 void prepare(double sampleRate, int) override { synth_.setSampleRate(sampleRate); synth_.allNotesOff(); chorus_.prepare(sampleRate); delay_.prepare(sampleRate); }
 void reset() override { synth_.allNotesOff(); chorus_.reset(); delay_.reset(); }
 void process(mpc::ProcessContext& ctx) override {
  if (ctx.numOutputs < 1 || !ctx.outputs || !ctx.outputs[0]) return;
  synth_.setWaveforms(static_cast<liberty::GenesisSynth::Waveform>(getInt(kWaveA)),
                      static_cast<liberty::GenesisSynth::Waveform>(getInt(kWaveB)));
  synth_.setOscMix(get(kMix)*.01f);
  synth_.setSubLevel(get(kSubLevel)*.01f);
  synth_.setVelocityFilter(get(kVelocityFilter)*.01f);
  synth_.setDetuneCents(get(kDetune));
  // Neutral macro values (50%) preserve all legacy preset sounds.
  const float tone=(get(kMacroTone)-50.f)*.01f;
  const float motion=(get(kMacroMotion)-50.f)*.01f;
  const float punch=(get(kMacroPunch)-50.f)*.01f;
  const float cutoff=std::clamp(get(kCutoff)*std::pow(2.f,tone*4.f),20.f,18000.f);
  synth_.setFilter(cutoff,get(kResonance)*.01f);
  synth_.setFilterMode(static_cast<liberty::GenesisSynth::FilterMode>(getInt(kFilterMode)));
  synth_.setFilterDrive(std::clamp(get(kFilterDrive)*.01f+std::max(0.f,punch)*.7f,0.f,1.f));
  synth_.setGain(get(kVolume)*.01f);
  synth_.setADSR(get(kAttack)*.001f*std::pow(2.f,-punch*2.f),get(kDecay)*.001f,get(kSustain)*.01f,get(kRelease)*.001f);
  synth_.setLFO(get(kLfoRate),std::clamp(get(kLfoDepth)*.01f+motion*.5f,0.f,1.f));
  chorus_.set(get(kChorusMix)*.01f,get(kChorusRate),get(kChorusDepth)*.01f,get(kChorusWidth)*.01f);
  constexpr double beats[]={0.,.25,.5,.75,1.,1.5,2.};
  const int sync=std::clamp(getInt(kDelaySync),0,6);
  const double bpm=ctx.transport.tempoValid&&std::isfinite(ctx.transport.bpm)?std::clamp(ctx.transport.bpm,40.,300.):120.;
  delay_.set(get(kDelayMix)*.01f,sync?beats[sync]*60./bpm:get(kDelayTime)*.001,get(kDelayFeedback)*.01f,get(kDelayStereo)*.01f,get(kDelayTone)*.01f);
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
  for(int i=begin;i<end;++i) { const float s=synth_.processSample(); float a,b; chorus_.process(s,a,b); delay_.process(a,b); l[i]=r?a:(a+b)*.5f; if(r)r[i]=b; }
 }
 liberty::GenesisSynth synth_{};
 liberty::GenesisChorus chorus_{};
 liberty::GenesisDelay delay_{};
};
MPC_PLUGIN_ENTRY(LibertyGenesis)
