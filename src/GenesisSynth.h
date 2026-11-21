#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace liberty {
class GenesisSynth {
public:
 static constexpr int maxVoices=16;
 enum class Waveform { Sine, Saw, Square, Triangle };
 enum class FilterMode { LowPass, BandPass, HighPass };
 void setSampleRate(double r){sr=std::max(8000.0,r);}
 void setGain(float v){gain=std::clamp(v,0.f,1.f);}
 void setWaveforms(Waveform a,Waveform b){wa=a;wb=b;}
 void setOscMix(float v){mix=std::clamp(v,0.f,1.f);}
 void setDetuneCents(float v){detune=std::clamp(v,-100.f,100.f);}
 void setFilterMode(FilterMode mode){filterMode=mode;}
 void setFilterDrive(float amount){filterDrive=std::clamp(amount,0.f,1.f);}
 void setFilter(float hz,float resonance){cutoff=std::clamp(hz,20.f,18000.f);res=std::clamp(resonance,0.f,.95f);}
 void setADSR(float a,float d,float s,float r){attack=std::clamp(a,.001f,10.f);decay=std::clamp(d,.001f,10.f);sustain=std::clamp(s,0.f,1.f);release=std::clamp(r,.001f,20.f);}
 void setLFO(float rate,float depth){lfoRate=std::clamp(rate,0.f,20.f);lfoDepth=std::clamp(depth,0.f,1.f);}
 void noteOn(int note,float velocity){
  if(note<0||note>127)return;
  Voice* v=nullptr;
  for(auto& x:voices)if(!x.active){v=&x;break;}
  if(!v)v=&voices[steal++%maxVoices];
  *v={};v->active=true;v->note=note;v->freq=440.*std::pow(2.,(note-69)/12.);v->velocity=std::clamp(velocity,0.f,1.f);
 }
 void noteOff(int note){for(auto& v:voices)if(v.active&&v.note==note&&!v.releasing){v.releasing=true;v.releaseLevel=v.env;v.releaseTime=0.;}}
 void allNotesOff(){for(auto& v:voices)v={};}
 float processSample(){
  constexpr double pi=3.14159265358979323846;
  const double lfo=std::sin(2*pi*lfoPhase);lfoPhase=wrap(lfoPhase+lfoRate/sr);
  const double fc=std::clamp(double(cutoff)*std::pow(2.,lfo*lfoDepth*2.),20.,sr*.45);
  const double g=std::tan(pi*fc/sr),k=2.-1.8*res,a1=1./(1.+g*(g+k)),a2=g*a1,a3=g*a2;
  const double driveGain=1.+filterDrive*7.;
  double sum=0.;
  for(auto& v:voices){
   if(!v.active)continue;
   if(v.releasing){v.releaseTime+=1./sr;v.env=v.releaseLevel*std::max(0.,1.-v.releaseTime/release);if(v.releaseTime>=release){v.active=false;continue;}}
   else if(v.env<1.&&!v.decaying){v.env=std::min(1.,v.env+1./(attack*sr));if(v.env>=1.)v.decaying=true;}
   else {v.env=std::max(double(sustain),v.env-(1.-sustain)/(decay*sr));}
   const double sa=v.freq/sr,sb=v.freq*std::pow(2.,detune/1200.)/sr;
   v.pa=wrap(v.pa+sa);v.pb=wrap(v.pb+sb);
   const double input=((1.-mix)*wave(v.pa,sa,wa)+mix*wave(v.pb,sb,wb))*v.env*v.velocity;
   const double driven=filterDrive>0.f ? std::tanh(input*driveGain)/std::tanh(driveGain) : input;
   const double v3=driven-v.ic2,v1=a1*v.ic1+a2*v3,v2=v.ic2+a2*v.ic1+a3*v3;
   v.ic1=2*v1-v.ic1;v.ic2=2*v2-v.ic2;
   const double high=driven-k*v1-v2;
   sum+=filterMode==FilterMode::LowPass?v2:(filterMode==FilterMode::BandPass?v1:high);
  }
  return float(std::tanh(sum*gain*.22));
 }
 void process(float* l,float* r,int n){if(!l||!r||n<=0)return;for(int i=0;i<n;++i)l[i]=r[i]=processSample();}
private:
 struct Voice {bool active=false,releasing=false,decaying=false;int note=-1;double freq=440.,velocity=0.,pa=0.,pb=0.,env=0.,releaseLevel=0.,releaseTime=0.,ic1=0.,ic2=0.;};
 static double wrap(double p){return p-std::floor(p);}
 static double blep(double t,double dt){dt=std::clamp(dt,1.e-8,.5);if(t<dt){t/=dt;return t+t-t*t-1.;}if(t>1.-dt){t=(t-1.)/dt;return t*t+t+t+1.;}return 0.;}
 static double wave(double p,double step,Waveform w){
  switch(w){
   case Waveform::Sine:return std::sin(6.283185307179586*p);
   case Waveform::Saw:return 2*p-1-blep(p,step);
   case Waveform::Square:return (p<.5?1.:-1.)+blep(p,step)-blep(wrap(p+.5),step);
   case Waveform::Triangle:return 1.-4.*std::abs(p-.5);
  }return 0.;
 }
 std::array<Voice,maxVoices> voices{};
 double sr=48000.,lfoPhase=0.;
 float gain=.8f,mix=.5f,detune=7.f,cutoff=8000.f,res=.1f,attack=.008f,decay=.1f,sustain=.8f,release=.18f,lfoRate=.5f,lfoDepth=0.f;
 Waveform wa=Waveform::Saw,wb=Waveform::Square;
 FilterMode filterMode=FilterMode::LowPass;
 float filterDrive=0.f;
 unsigned steal=0;
};
}
