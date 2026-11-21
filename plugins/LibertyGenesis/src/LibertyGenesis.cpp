#include "mpc_plugin.h"
#include "GenesisSynth.h"
#include "GenesisFactory100.h"
#include "GenesisChorus.h"
#include "GenesisDelay.h"
#include "GenesisReverb.h"
#include <string_view>
#include <algorithm>
#include <array>

class LibertyGenesis final : public mpc::Plugin {
public:
 enum Params { kWaveA, kWaveB, kMix, kDetune, kCutoff, kResonance,
               kVolume, kAttack, kDecay, kSustain, kRelease,
               kLfoRate, kLfoDepth, kFilterMode, kFilterDrive, kMacroTone, kMacroMotion, kMacroPunch, kSubLevel, kVelocityFilter, kChorusMix, kChorusRate, kChorusDepth, kChorusWidth, kDelayMix, kDelayTime, kDelaySync, kDelayFeedback, kDelayStereo, kDelayTone, kReverbMode, kReverbMix, kReverbPre, kReverbDecay, kReverbDamp, kReverbWidth, kPulse, kPWM, kFM, kRing, kNoise, kSync, kOctA, kSemiA, kOctB, kSemiB, kUniCount, kUniDetune, kUniWidth, kUniBlend, kUniPhase, kUniEco, kArpOn, kArpMode, kArpRate, kArpOct, kArpGate, kArpSwing, kArpVel, kArpHold, kLfo2, kModA, kModD, kModS, kModR, kMatrix };
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
  addParam(mpc::choiceParam("reverbmode","Reverb Mode",{"Room","Plate","Hall","Ambient"},2));
  addParam(mpc::floatParam("reverbmix","Reverb Mix",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("reverbpre","Pre Delay",0.f,200.f,20.f,"ms",0));
  addParam(mpc::logParam("reverbdecay","Decay Time",0.2f,12.f,2.5f,"s",2));
  addParam(mpc::floatParam("reverbdamp","Damping",0.f,100.f,45.f,"%",0));
  addParam(mpc::floatParam("reverbwidth","Reverb Width",0.f,100.f,100.f,"%",0));
  addParam(mpc::floatParam("pulsewidth","Pulse Width",10.f,90.f,50.f,"%",0));
  addParam(mpc::floatParam("pwmdepth","PWM Depth",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("fmdepth","FM Depth",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("ringmix","Ring Mix",0.f,100.f,0.f,"%",0));
  addParam(mpc::floatParam("noiselevel","Noise Level",0.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("oscsync","Osc Sync",{"Off","On"},0));
  addParam(mpc::choiceParam("octavea","Octave A",{"-2","-1","0","1","2"},2));
  addParam(mpc::choiceParam("semia","Semi A",{"-12","-11","-10","-9","-8","-7","-6","-5","-4","-3","-2","-1","0","1","2","3","4","5","6","7","8","9","10","11","12"},12));
  addParam(mpc::choiceParam("octaveb","Octave B",{"-2","-1","0","1","2"},2));
  addParam(mpc::choiceParam("semib","Semi B",{"-12","-11","-10","-9","-8","-7","-6","-5","-4","-3","-2","-1","0","1","2","3","4","5","6","7","8","9","10","11","12"},12));
  addParam(mpc::choiceParam("unicount","Unison Count",{"1","2","3","5","7"},0));
  addParam(mpc::floatParam("unidetune","Uni Detune",0.f,60.f,18.f,"ct",0));
  addParam(mpc::floatParam("uniwidth","Uni Width",0.f,100.f,100.f,"%",0));
  addParam(mpc::floatParam("uniblend","Uni Blend",0.f,100.f,50.f,"%",0));
  addParam(mpc::choiceParam("uniphase","Uni Phase",{"Reset","Spread","Random"},1));
  addParam(mpc::choiceParam("unieco","Uni CPU",{"Full","Eco"},1));
  addParam(mpc::choiceParam("arpon","Arp On",{"Off","On"},0));
  addParam(mpc::choiceParam("arpmode","Arp Mode",{"Up","Down","Up Down","Random","Order"},0));
  addParam(mpc::choiceParam("arprate","Arp Rate",{"1/4","1/8","1/16","1/32","1/8 T","1/16 T","1/8 D","1/16 D"},2));
  addParam(mpc::choiceParam("arpoct","Arp Octaves",{"1","2","3","4"},0));
  addParam(mpc::floatParam("arpgate","Arp Gate",5.f,100.f,65.f,"%",0));
  addParam(mpc::floatParam("arpswing","Arp Swing",0.f,75.f,0.f,"%",0));
  addParam(mpc::floatParam("arpvel","Arp Velocity",1.f,127.f,100.f,"",0));
  addParam(mpc::choiceParam("arphold","Arp Hold",{"Off","On"},0));
  addParam(mpc::logParam("lfo2","LFO 2 Rate",.05f,20.f,.3f,"Hz",2));
  addParam(mpc::logParam("moda","Mod Attack",1.f,10000.f,100.f,"ms",0));
  addParam(mpc::logParam("modd","Mod Decay",1.f,10000.f,500.f,"ms",0));
  addParam(mpc::floatParam("mods","Mod Sustain",0.f,100.f,50.f,"%",0));
  addParam(mpc::logParam("modr","Mod Release",1.f,20000.f,500.f,"ms",0));
  addParam(mpc::choiceParam("mx1s","M1 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx1d","M1 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx1a","M1 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx1p","M1 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx2s","M2 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx2d","M2 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx2a","M2 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx2p","M2 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx3s","M3 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx3d","M3 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx3a","M3 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx3p","M3 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx4s","M4 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx4d","M4 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx4a","M4 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx4p","M4 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx5s","M5 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx5d","M5 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx5a","M5 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx5p","M5 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx6s","M6 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx6d","M6 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx6a","M6 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx6p","M6 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx7s","M7 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx7d","M7 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx7a","M7 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx7p","M7 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx8s","M8 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx8d","M8 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx8a","M8 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx8p","M8 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx9s","M9 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx9d","M9 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx9a","M9 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx9p","M9 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx10s","M10 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx10d","M10 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx10a","M10 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx10p","M10 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx11s","M11 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx11d","M11 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx11a","M11 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx11p","M11 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx12s","M12 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx12d","M12 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx12a","M12 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx12p","M12 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx13s","M13 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx13d","M13 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx13a","M13 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx13p","M13 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx14s","M14 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx14d","M14 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx14a","M14 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx14p","M14 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx15s","M15 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx15d","M15 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx15a","M15 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx15p","M15 Polarity",{"Uni","Bi"},0));
  addParam(mpc::choiceParam("mx16s","M16 Source",{"Off","LFO 1","LFO 2","Amp Env","Mod Env","Velocity","Mod Wheel","Aftertouch","Key Track","Random"},0));
  addParam(mpc::choiceParam("mx16d","M16 Dest",{"Off","Pitch","Cutoff","Resonance","PWM","FM","Ring","Unison","Chorus Mix","Delay Mix","Reverb Mix"},0));
  addParam(mpc::floatParam("mx16a","M16 Amount",-100.f,100.f,0.f,"%",0));
  addParam(mpc::choiceParam("mx16p","M16 Polarity",{"Uni","Bi"},0));
  // MPC native program browser: 100 categorized factory programs.
  int programIndex=0;
  for (const auto& preset : liberty::genesisFactory100) {
   const std::string_view name(preset.name);
   const float chorusMix=name.substr(0,3)=="PAD"?45.f:name.substr(0,4)=="POLY"?30.f:name.substr(0,4)=="KEYS"?20.f:0.f;
   std::vector<float> values = {
    static_cast<float>(preset.waveA), static_cast<float>(preset.waveB),
    preset.mix*100.f, preset.detune, preset.cutoff, preset.resonance*100.f,
    preset.gain*100.f, preset.attack*1000.f, preset.decay*1000.f,
    preset.sustain*100.f, preset.release*1000.f,
    preset.lfoRate, preset.lfoDepth*100.f,
    preset.filterMode, preset.filterDrive, 50.f, 50.f, 50.f, 0.f, 0.f, chorusMix, 0.35f, 45.f, 100.f, 0.f, 350.f, 0.f, 32.f, 75.f, 55.f, 2.f, name.substr(0,3)=="PAD"?25.f:0.f, 20.f, 2.5f, 45.f, 100.f, 50.f, 0.f, 0.f, 0.f, 0.f, 0.f, 2.f, 12.f, 2.f, 12.f, 0.f, 18.f, 100.f, 50.f, 1.f, 1.f
   };
   // Category-specific sound design, with per-program variation.
   const int v=programIndex++%10;
   const bool bass=name.substr(0,4)=="BASS", lead=name.substr(0,4)=="LEAD", pad=name.substr(0,3)=="PAD", keys=name.substr(0,4)=="KEYS", pluck=name.substr(0,5)=="PLUCK", arp=name.substr(0,3)=="ARP", tex=name.substr(0,7)=="TEXTURE", brass=name.substr(0,5)=="BRASS", fx=name.substr(0,2)=="FX", poly=name.substr(0,4)=="POLY";
   values[kSubLevel]=bass?float(25+v*5):lead?12.f:0.f;
   values[kVelocityFilter]=keys||pluck||brass?float(25+v*5):15.f;
   values[kMacroTone]=float(45+v%4*4);
   values[kMacroMotion]=pad||tex?float(55+v*2):50.f;
   values[kMacroPunch]=bass||pluck?float(55+v*2):50.f;
   values[kPulse]=float(30+v*4);
   values[kPWM]=pad||brass||poly?float(15+v*5):0.f;
   values[kFM]=keys||fx||tex?float(5+v*6):lead&&v%3==0?20.f:0.f;
   values[kRing]=fx||tex?float(v*5):0.f;
   values[kNoise]=tex||fx?float(8+v*4):brass?5.f:0.f;
   values[kSync]=lead&&v%3==1?1.f:0.f;
   values[kOctB]=bass&&v%2==0?1.f:2.f;
   values[kSemiB]=tex&&v%3==0?19.f:12.f;
   values[kUniCount]=pad||poly||lead||tex?float(2+v%3):0.f;
   values[kUniDetune]=float(8+v*4);
   values[kUniWidth]=bass?0.f:float(65+v*3);
   values[kUniBlend]=float(30+v*4);
   values[kUniPhase]=pad||tex?2.f:1.f;
   values[kDelayMix]=arp||lead||pluck||tex?float(12+v*3):0.f;
   values[kDelaySync]=float(1+v%6);
   values[kDelayFeedback]=float(20+v*4);
   values[kReverbMode]=pad?2.f:tex||fx?3.f:keys||lead?1.f:0.f;
   values[kReverbMix]=bass?0.f:float((pad||tex?25:8)+v*3);
   values[kReverbDecay]=pad||tex?float(3.+v*.65):float(.5+v*.22);
   if(pad||poly||lead||tex)values[kVolume]*=.8f;
   values.insert(values.end(),{arp?1.f:0.f,float(v%5),float(1+v%7),float(v%3),float(40+v*5),float(v%4*10),100.f,0.f});
   values.insert(values.end(),{.3f+v*.12f,pad?1200.f:30.f,500.f,35.f,600.f});
   for(int i=0;i<16;++i){if(i==0)values.insert(values.end(),{5.f,2.f,12.f,0.f});else if(i==1&&(pad||tex))values.insert(values.end(),{2.f,4.f,20.f,1.f});else if(i==2&&(keys||pluck||bass))values.insert(values.end(),{4.f,2.f,25.f,0.f});else if(i==3&&lead)values.insert(values.end(),{6.f,1.f,3.f,0.f});else if(i==4&&(pad||tex))values.insert(values.end(),{2.f,10.f,12.f,1.f});else if(i==5&&lead)values.insert(values.end(),{7.f,2.f,20.f,0.f});else if(i==6&&arp)values.insert(values.end(),{1.f,9.f,10.f,1.f});else values.insert(values.end(),{0.f,0.f,0.f,0.f});}
   if(values.size()==static_cast<size_t>(kMatrix+64)&&std::all_of(values.begin(),values.end(),[](float x){return std::isfinite(x);}))addProgram(preset.name, std::move(values));
  }
 }
 void prepare(double sampleRate, int) override { arpSR=sampleRate; synth_.setSampleRate(sampleRate); synth_.allNotesOff(); chorus_.prepare(sampleRate); delay_.prepare(sampleRate); reverb_.prepare(sampleRate); }
 void reset() override { clearArp(); synth_.allNotesOff(); chorus_.reset(); delay_.reset(); reverb_.reset(); }
 void process(mpc::ProcessContext& ctx) override {
  if (ctx.numOutputs < 1 || !ctx.outputs || !ctx.outputs[0]) return;
  synth_.setWaveforms(static_cast<liberty::GenesisSynth::Waveform>(getInt(kWaveA)),
                      static_cast<liberty::GenesisSynth::Waveform>(getInt(kWaveB)));
  synth_.setAdvanced(get(kPulse)*.01f,get(kPWM)*.01f,get(kFM)*.01f,get(kRing)*.01f,get(kNoise)*.01f,getInt(kSync)!=0,getInt(kOctA)-2,getInt(kSemiA)-12,getInt(kOctB)-2,getInt(kSemiB)-12);
  constexpr int uniCounts[]={1,2,3,5,7};
  synth_.setUnison(uniCounts[std::clamp(getInt(kUniCount),0,4)],get(kUniDetune),get(kUniWidth)*.01f,get(kUniBlend)*.01f,getInt(kUniPhase),getInt(kUniEco)!=0);
  synth_.setModulators(get(kLfo2),get(kModA)*.001f,get(kModD)*.001f,get(kModS)*.01f,get(kModR)*.001f);
  for(int i=0;i<16;++i){int p=kMatrix+i*4;synth_.setMatrix(i,getInt(p),getInt(p+1),get(p+2)*.01f,getInt(p+3)!=0);}
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
  chorus_.set(std::clamp(get(kChorusMix)*.01f+synth_.effectMod(8),0.f,1.f),get(kChorusRate),get(kChorusDepth)*.01f,get(kChorusWidth)*.01f);
  constexpr double beats[]={0.,.25,.5,.75,1.,1.5,2.};
  const int sync=std::clamp(getInt(kDelaySync),0,6);
  const double bpm=ctx.transport.tempoValid&&std::isfinite(ctx.transport.bpm)?std::clamp(ctx.transport.bpm,40.,300.):120.;
  delay_.set(std::clamp(get(kDelayMix)*.01f+synth_.effectMod(9),0.f,1.f),sync?beats[sync]*60./bpm:get(kDelayTime)*.001,get(kDelayFeedback)*.01f,get(kDelayStereo)*.01f,get(kDelayTone)*.01f);
  reverb_.set(getInt(kReverbMode),std::clamp(get(kReverbMix)*.01f+synth_.effectMod(10),0.f,1.f),get(kReverbPre),get(kReverbDecay),get(kReverbDamp)*.01f,get(kReverbWidth)*.01f);
  const bool nextArp=getInt(kArpOn)!=0;
  if(nextArp!=arpEnabled){synth_.allNotesOff();clearArp();arpEnabled=nextArp;}
  arpMode=getInt(kArpMode);arpOct=1+getInt(kArpOct);
  constexpr double divisions[]={1.,.5,.25,.125,1./3.,1./6.,.75,.375};
  arpPeriod=arpSR*60./bpm*divisions[std::clamp(getInt(kArpRate),0,7)];
  arpGate=get(kArpGate)*.01;arpSwing=get(kArpSwing)*.01;arpVelocity=get(kArpVel)/127.f;
  const bool nextHold=getInt(kArpHold)!=0;
  if(arpHold&&!nextHold){for(int n=0;n<128;++n)if(!physical[n])held[n]=false;rebuildArp();}
  arpHold=nextHold;
  float* left = ctx.outputs[0];
  float* right = ctx.numOutputs > 1 ? ctx.outputs[1] : nullptr;
  int pos = 0;
  for(int i=0;i<ctx.numMidiEvents;++i) {
   const auto& ev=ctx.midi[i];
   const int offset=std::clamp(ev.sampleOffset,pos,ctx.numFrames);
   render(left,right,pos,offset);
   pos=offset;
   if(ev.isNoteOn()){if(arpEnabled)arpNote(ev.note(),true);else synth_.noteOn(ev.note(),ev.velocity()/127.f);}
   else if(ev.isNoteOff()){if(arpEnabled)arpNote(ev.note(),false);else synth_.noteOff(ev.note());}
   else if(ev.isController()&&ev.controller()==1){wheel=ev.controllerValue()/127.f;synth_.setExpression(wheel,pressure);}
   else if(ev.isAftertouch()&&ev.size>=2){pressure=ev.data[1]/127.f;synth_.setExpression(wheel,pressure);}
   else if(ev.status()==0xA0&&ev.size>=3){pressure=ev.data[2]/127.f;synth_.setExpression(wheel,pressure);}
   else if(ev.isAllNotesOff()){clearArp();synth_.allNotesOff();}
  }
  render(left,right,pos,ctx.numFrames);
 }
private:
 float wheel=0.f,pressure=0.f;
 double arpSR=48000.,arpPeriod=6000.,arpGate=.65,arpSwing=0.,untilStep=0.,untilOff=0.;
 bool arpEnabled=false,arpHold=false;
 int arpMode=0,arpOct=1,arpSound=-1,arpStep=0,nHeld=0;
 float arpVelocity=.8f;
 std::array<bool,128> held{},physical{};
 std::array<unsigned,128> order{};
 std::array<int,128> notes{};
 unsigned serial=0,rngArp=1234567;
 void clearArp(){if(arpSound>=0)synth_.noteOff(arpSound);arpSound=-1;held.fill(false);physical.fill(false);nHeld=arpStep=0;untilStep=untilOff=0.;}
 void rebuildArp(){nHeld=0;for(int n=0;n<128;++n)if(held[n])notes[nHeld++]=n;if(!nHeld){if(arpSound>=0)synth_.noteOff(arpSound);arpSound=-1;untilStep=0.;arpStep=0;}}
 void arpNote(int n,bool on){if(n<0||n>=128)return;if(on){if(arpHold&&std::none_of(physical.begin(),physical.end(),[](bool b){return b;}))held.fill(false);physical[n]=held[n]=true;order[n]=++serial;}else{physical[n]=false;if(!arpHold)held[n]=false;}rebuildArp();}
 void arpTick(){
  if(!nHeld)return;
  if(arpSound>=0&&untilOff<=0.){synth_.noteOff(arpSound);arpSound=-1;}
  if(untilStep<=0.){
   if(arpSound>=0)synth_.noteOff(arpSound);
   const int total=nHeld*arpOct;int idx=arpStep%total;
   if(arpMode==1)idx=total-1-idx;
   if(arpMode==2&&total>1){const int cycle=2*total-2;idx=arpStep%cycle;if(idx>=total)idx=cycle-idx;}
   if(arpMode==3){rngArp^=rngArp<<13;rngArp^=rngArp>>17;rngArp^=rngArp<<5;idx=int(rngArp%unsigned(total));}
   int note=notes[idx%nHeld];
   if(arpMode==4){int rank=idx%nHeld;for(int j=0;j<nHeld;++j){int smaller=0;for(int k=0;k<nHeld;++k)if(order[notes[k]]<order[notes[j]])++smaller;if(smaller==rank){note=notes[j];break;}}}
   arpSound=std::clamp(note+12*(idx/nHeld),0,127);
   synth_.noteOn(arpSound,arpVelocity);
   const double duration=std::max(1.,arpPeriod*(1.+(arpStep%2?-1.:1.)*arpSwing*.5));
   untilStep+=duration;untilOff=duration*arpGate;
   arpStep=(arpStep+1)%1000000;
  }
  --untilStep;--untilOff;
 }
 void render(float* l,float* r,int begin,int end) {
  for(int i=begin;i<end;++i) { if(arpEnabled)arpTick(); float dryL,dryR,a,b; synth_.processStereo(dryL,dryR); chorus_.processStereo(dryL,dryR,a,b); delay_.process(a,b); reverb_.process(a,b); l[i]=r?a:(a+b)*.5f; if(r)r[i]=b; }
 }
 liberty::GenesisSynth synth_{};
 liberty::GenesisChorus chorus_{};
 liberty::GenesisDelay delay_{};
 liberty::GenesisReverb reverb_{};
};
MPC_PLUGIN_ENTRY(LibertyGenesis)
