#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace liberty {
class GenesisSynth {
public:
 static constexpr int maxVoices=16;
 enum class Waveform { Sine, Saw, Square, Triangle };
 enum class FilterMode { LowPass, BandPass, HighPass };
 void setSampleRate(double r){sr=std::max(8000.0,r);}
 void setGain(float v){gain=std::clamp(v,0.f,1.f);}
 void setWaveforms(Waveform a,Waveform b){wa=a;wb=b;}
 void setSubLevel(float v){subLevel=std::clamp(v,0.f,1.f);}
 void setVelocityFilter(float v){velocityFilter=std::clamp(v,0.f,1.f);}
 void setAdvanced(float pw,float pwm,float fm,float ring,float noise,bool sync,int octA,int semiA,int octB,int semiB){pulse=std::clamp(pw,.1f,.9f);pwmDepth=std::clamp(pwm,0.f,1.f);fmDepth=std::clamp(fm,0.f,1.f);ringMix=std::clamp(ring,0.f,1.f);noiseMix=std::clamp(noise,0.f,1.f);hardSync=sync;ratioA=std::pow(2.,octA+semiA/12.);ratioB=std::pow(2.,octB+semiB/12.);}
 void setUnison(int count,float cents,float spread,float blend,int phase,bool eco){unison=std::clamp(count,1,7);uniWidth=std::clamp(spread,0.f,1.f);uniBlend=std::clamp(blend,0.f,1.f);uniPhase=std::clamp(phase,0,2);uniEco=eco;for(int j=0;j<6;++j){double p=(j%2?1.:-1.)*(j/2+1)/3.;uniRatio[j]=std::pow(2.,p*std::clamp(cents,0.f,60.f)/1200.);}}
 void setMatrix(int i,int source,int dest,float amount,bool bipolar){if(i>=0&&i<16)routes[i]={source,dest,std::clamp(amount,-1.f,1.f),bipolar};}
 void setModulators(float rate,float a,float d,float s,float r){lfo2Rate=std::clamp(rate,.05f,20.f);ma=std::max(.001f,a);md=std::max(.001f,d);ms=std::clamp(s,0.f,1.f);mr=std::max(.001f,r);}
 void setExpression(float wheel,float pressure){modWheel=std::clamp(wheel,0.f,1.f);aftertouch=std::clamp(pressure,0.f,1.f);}
 float effectMod(int dest) const {return dest>=8&&dest<=10?fxMods[dest-8]:0.f;}
 void setOscMix(float v){mix=std::clamp(v,0.f,1.f);}
 void setDetuneCents(float v){detune=std::clamp(v,-100.f,100.f);detuneRatio=std::pow(2.,detune/1200.);}
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
  *v={};v->active=true;v->note=note;v->freq=440.*std::pow(2.,(note-69)/12.);v->velocity=std::clamp(velocity,0.f,1.f);rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;v->random=double(rng)/4294967296.;for(int j=0;j<6;++j){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;v->up[j]=uniPhase==0?0.:uniPhase==1?(j+1)/7.:double(rng)/4294967296.;}
 }
 void noteOff(int note){for(auto& v:voices)if(v.active&&v.note==note&&!v.releasing){v.releasing=true;v.releaseLevel=v.env;v.modRelease=v.modEnv;v.releaseTime=0.;}}
 void allNotesOff(){for(auto& v:voices)v={};}
 float processSample(){float l,r;processStereo(l,r);return (l+r)*.5f;}
 void processStereo(float& left,float& right){
  constexpr double pi=3.14159265358979323846;
  const double lfo=std::sin(2*pi*lfoPhase);lfoPhase=wrap(lfoPhase+lfoRate/sr);
  fxMods.fill(0.f);int fxVoices=0;
  const double lfo2=std::sin(2*pi*lfo2Phase);lfo2Phase=wrap(lfo2Phase+lfo2Rate/sr);
  const double fc=std::clamp(double(cutoff)*std::pow(2.,lfo*lfoDepth*2.),20.,sr*.45);
  const double g=std::tan(pi*fc/sr),k=2.-1.8*res,a1=1./(1.+g*(g+k)),a2=g*a1,a3=g*a2;
  const double driveGain=1.+filterDrive*7.;
  double sum=0.,side=0.;int active=0;for(const auto& v:voices)if(v.active)++active;const int extras=std::min(unison-1,uniEco?std::max(0,48/std::max(1,active)-1):6);
  for(auto& v:voices){
   if(!v.active)continue;
   if(v.releasing){v.releaseTime+=1./sr;v.env=v.releaseLevel*std::max(0.,1.-v.releaseTime/release);if(v.releaseTime>=release){v.active=false;continue;}}
   else if(v.env<1.&&!v.decaying){v.env=std::min(1.,v.env+1./(attack*sr));if(v.env>=1.)v.decaying=true;}
   else {v.env=std::max(double(sustain),v.env-(1.-sustain)/(decay*sr));}
   if(v.releasing)v.modEnv=v.modRelease*std::max(0.,1.-v.releaseTime/mr);
   else if(!v.modDecay){v.modEnv=std::min(1.,v.modEnv+1./(ma*sr));if(v.modEnv>=1.)v.modDecay=true;}
   else v.modEnv=std::max(double(ms),v.modEnv-(1.-ms)/(md*sr));
   double m[11]={};
   const double src[]={0.,(lfo+1.)*.5,(lfo2+1.)*.5,v.env,v.modEnv,v.velocity,modWheel,aftertouch,v.note/127.,v.random};
   for(const auto& route:routes)if(route.source>0&&route.source<10&&route.dest>0&&route.dest<11&&route.amount!=0){double x=src[route.source];m[route.dest]+=route.amount*(route.bipolar?2*x-1:x);}
   for(int j=0;j<3;++j)fxMods[j]+=float(m[j+8]);++fxVoices;
   const double pitch=m[1]==0.?1.:std::pow(2.,std::clamp(m[1],-1.,1.)*2.);
   const double sa=std::min(.45,v.freq*ratioA*pitch/sr),sb=std::min(.45,v.freq*ratioB*detuneRatio*pitch/sr);
   const double pw=std::clamp(double(pulse)+lfo*pwmDepth*.35+m[4]*.4,.05,.95);
   const bool wrapped=v.pa+sa>=1.;v.pa=wrap(v.pa+sa);
   const double oscA=wave(v.pa,sa,wa,pw);
   const double stepB=std::clamp(sb*(1.+std::clamp(fmDepth+m[5],0.,1.)*oscA*.95),1.e-8,.45);
   const double nextB=wrap(v.pb+stepB);
   if(hardSync&&wrapped){const double after=v.pa/sa;const double resetPhase=after*stepB;v.syncResidual+=wave(nextB,stepB,wb,pw)-wave(resetPhase,stepB,wb,pw);v.pb=resetPhase;}else v.pb=nextB;
   const double oscB=wave(v.pb,stepB,wb,pw)+v.syncResidual;v.syncResidual*=.35;
   double noise=0.;if(noiseMix>0){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;noise=double(rng)/2147483648.-1.;}
   v.ps=wrap(v.ps+sa*.5);
   const double sub=2.*v.ps-1.-blep(v.ps,sa*.5);
   double uMid=0.,uSide=0.;
   for(int j=0;j<extras;++j){const double step=std::min(.45,sa*uniRatio[j]);v.up[j]=wrap(v.up[j]+step);const double w=wave(v.up[j],step,wa,pw);uMid+=w;uSide+=w*(j%2?1.:-1.);}
   const double blend=extras?std::clamp(uniBlend+m[7],0.,1.):0.;const double norm=extras?1./std::sqrt(double(extras)):0.;
   const double input=((1.-blend)*((1.-std::clamp(ringMix+m[6],0.,1.))*((1.-mix)*oscA+mix*oscB)+std::clamp(ringMix+m[6],0.,1.)*oscA*oscB)+blend*uMid*norm+noiseMix*.5*noise+subLevel*.5*sub)*v.env*v.velocity;
   const double voiceVelocityCut=1.-velocityFilter*(1.-v.velocity)*.85;
   const bool modFilter=velocityFilter>0.f||m[2]!=0||m[3]!=0;const double vk=2.-1.8*std::clamp(res+m[3]*.9,0.,.95);const double vg=modFilter?std::tan(pi*std::clamp(fc*voiceVelocityCut*std::pow(2.,std::clamp(m[2],-2.,2.)*4.),20.,sr*.45)/sr):g;
   const double va1=modFilter?1./(1.+vg*(vg+vk)):a1;
   const double va2=modFilter?vg*va1:a2;
   const double va3=modFilter?vg*va2:a3;
   const double driven=filterDrive>0.f ? std::tanh(input*driveGain)/std::tanh(driveGain) : input;
   const double v3=driven-v.ic2,v1=va1*v.ic1+va2*v3,v2=v.ic2+va2*v.ic1+va3*v3;
   v.ic1=2*v1-v.ic1;v.ic2=2*v2-v.ic2;
   const double high=driven-vk*v1-v2;
   sum+=filterMode==FilterMode::LowPass?v2:(filterMode==FilterMode::BandPass?v1:high);
   const double sideIn=uSide*norm*blend*uniWidth*v.env*v.velocity;
   const double s3=sideIn-v.sc2,s1=va1*v.sc1+va2*s3,s2=v.sc2+va2*v.sc1+va3*s3;v.sc1=2*s1-v.sc1;v.sc2=2*s2-v.sc2;
   side+=filterMode==FilterMode::LowPass?s2:(filterMode==FilterMode::BandPass?s1:sideIn-vk*s1-s2);
  }
  if(fxVoices)for(auto& x:fxMods)x=std::clamp(x/fxVoices,-1.f,1.f);
  left=float(std::tanh((sum+side)*gain*.22));right=float(std::tanh((sum-side)*gain*.22));
 }
 void process(float* l,float* r,int n){if(!l||!r||n<=0)return;for(int i=0;i<n;++i)processStereo(l[i],r[i]);}
private:
 struct Voice {bool active=false,releasing=false,decaying=false;int note=-1;double freq=440.,velocity=0.,pa=0.,pb=0.,ps=0.,env=0.,releaseLevel=0.,releaseTime=0.,ic1=0.,ic2=0.,syncResidual=0.,sc1=0.,sc2=0.;std::array<double,6> up{};double modEnv=0,modRelease=0,random=0;bool modDecay=false;};
 static double wrap(double p){return p-std::floor(p);}
 static double blep(double t,double dt){dt=std::clamp(dt,1.e-8,.5);if(t<dt){t/=dt;return t+t-t*t-1.;}if(t>1.-dt){t=(t-1.)/dt;return t*t+t+t+1.;}return 0.;}
 static double wave(double p,double step,Waveform w,double pw=.5){
  switch(w){
   case Waveform::Sine:return std::sin(6.283185307179586*p);
   case Waveform::Saw:return 2*p-1-blep(p,step);
   case Waveform::Square:return (p<pw?1.:-1.)+blep(p,step)-blep(wrap(p-pw),step);
   case Waveform::Triangle:return 1.-4.*std::abs(p-.5);
  }return 0.;
 }
 std::array<Voice,maxVoices> voices{};
 double sr=48000.,lfoPhase=0.;
 float gain=.8f,mix=.5f,detune=7.f,cutoff=8000.f,res=.1f,attack=.008f,decay=.1f,sustain=.8f,release=.18f,lfoRate=.5f,lfoDepth=0.f;
 Waveform wa=Waveform::Saw,wb=Waveform::Square;
 FilterMode filterMode=FilterMode::LowPass;
 float filterDrive=0.f,subLevel=0.f,velocityFilter=0.f;
 float pulse=.5f,pwmDepth=0.f,fmDepth=0.f,ringMix=0.f,noiseMix=0.f;
 bool hardSync=false;double ratioA=1.,ratioB=1.,detuneRatio=std::pow(2.,7./1200.);std::uint32_t rng=0x12345678u;
 int unison=1,uniPhase=0;bool uniEco=true;float uniWidth=1.f,uniBlend=.5f;std::array<double,6> uniRatio{1.,1.,1.,1.,1.,1.};
 struct Route {int source=0,dest=0;float amount=0;bool bipolar=false;};
 std::array<Route,16> routes{};std::array<float,3> fxMods{};
 double lfo2Phase=0.;float lfo2Rate=.3f,ma=.1f,md=.5f,ms=.5f,mr=.5f,modWheel=0,aftertouch=0;
 unsigned steal=0;
};
}
