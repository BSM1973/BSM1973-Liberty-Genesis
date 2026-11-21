#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace liberty {
class GenesisChorus {
 static constexpr unsigned capacity=8192, mask=capacity-1;
 std::array<float,capacity> buffer{};
 unsigned write=0;
 double sr=48000., phase=0., sine=0., cosine=1., ds=0., dc=1.;
 float wet=0.f, target=0.f, depth=0.f, width=0.f, smooth=0.002f;
 float tap(double delay) const {
  delay=std::clamp(delay,1.,double(capacity-2));
  const unsigned whole=static_cast<unsigned>(delay);
  const float frac=static_cast<float>(delay-whole);
  const float a=buffer[(write-whole)&mask], b=buffer[(write-whole-1)&mask];
  return a+(b-a)*frac;
 }
public:
 void prepare(double rate){sr=std::clamp(rate,8000.,192000.);smooth=float(1.-std::exp(-1./(.015*sr)));reset();}
 void reset(){buffer.fill(0.f);write=0;phase=0.;sine=0.;cosine=1.;wet=0.f;}
 void set(float mix,float rate,float amount,float spread){
  target=std::clamp(mix,0.f,1.f)*.65f;
  depth=std::clamp(amount,0.f,1.f);
  width=std::clamp(spread,0.f,1.f);
  const double angle=6.283185307179586*std::clamp(rate,.05f,5.f)/sr;
  ds=std::sin(angle);dc=std::cos(angle);
 }
 void process(float input,float& l,float& r){processStereo(input,input,l,r);}
 void processStereo(float inL,float inR,float& l,float& r){
  const float input=(inL+inR)*.5f;
  buffer[write]=input;
  wet+=(target-wet)*smooth;
  if(target==0.f && wet<1.e-6f){wet=0.f;l=inL;r=inR;}
  else{
   const double base=.016*sr, excursion=.007*sr*depth;
   const float a=tap(base+excursion*sine);
   const float b=tap(base+excursion*(sine*(1.-width)+cosine*width));
   l=inL*(1.f-wet)+a*wet;r=inR*(1.f-wet)+b*wet;
   const double next=sine*dc+cosine*ds;
   cosine=cosine*dc-sine*ds;sine=next;
   // Correct floating point drift without trigonometry.
   const double norm=1.5-.5*(sine*sine+cosine*cosine);
   sine*=norm;cosine*=norm;
  }
  write=(write+1)&mask;
 }
};
}
