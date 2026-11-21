#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
namespace liberty {
class GenesisDelay {
 std::vector<float> a,b;
 unsigned pos=0;
 double sr=48000., delay=12000., targetDelay=12000.;
 float wet=0, targetWet=0, feedback=0, cross=0, damping=.4f, zL=0,zR=0, smooth=.001f;
 float read(const std::vector<float>& v) const {
  const unsigned n=static_cast<unsigned>(v.size()), d=static_cast<unsigned>(delay);
  const float frac=static_cast<float>(delay-d);
  const unsigned p=(pos+n-d)%n, q=p?p-1:n-1;
  return v[p]+(v[q]-v[p])*frac;
 }
public:
 void prepare(double rate){sr=std::clamp(rate,8000.,192000.);a.assign(static_cast<unsigned>(sr*3)+4,0.f);b.assign(a.size(),0.f);smooth=float(1.-std::exp(-1./(.025*sr)));reset();}
 void reset(){std::fill(a.begin(),a.end(),0.f);std::fill(b.begin(),b.end(),0.f);pos=0;wet=zL=zR=0;delay=targetDelay=sr*.25;}
 void set(float mix,double seconds,float fb,float stereo,float tone){
  targetWet=std::clamp(mix,0.f,1.f)*.65f;
  targetDelay=std::clamp(seconds*sr,1.,std::max(1.,double(a.size())-2.));
  feedback=std::clamp(fb,0.f,.85f);cross=std::clamp(stereo,0.f,1.f);damping=.05f+.90f*std::clamp(tone,0.f,1.f);
 }
 void process(float& l,float& r){
  if(a.empty())return;
  wet+=(targetWet-wet)*smooth;delay+=(targetDelay-delay)*smooth;
  const float x=read(a),y=read(b);
  zL+=damping*(x-zL);zR+=damping*(y-zR);
  a[pos]=l+feedback*((1-cross)*zL+cross*zR);
  b[pos]=r+feedback*((1-cross)*zR+cross*zL);
  // Offset the first repeat to create stereo movement even with a mono source.
  l=l*(1-wet)+wet*x;
  r=r*(1-wet)+wet*y*(1-.55f*cross);
  if(++pos>=a.size())pos=0;
 }
};
}
