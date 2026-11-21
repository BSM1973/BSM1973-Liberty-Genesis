#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
namespace liberty {
class GenesisReverb {
 struct Line {std::vector<float> b; unsigned p=0,n=1; float lp=0;};
 std::array<Line,8> lines;
 std::vector<float> preL,preR;
 unsigned pos=0,pre=0;
 double sr=48000.;
 float wet=0,target=0,smooth=.001f,width=1,damp=.5f;
 std::array<float,8> gains{};
 int mode=-1;
public:
 void prepare(double rate){
 sr=std::clamp(rate,8000.,192000.);
 for(auto& l:lines)l.b.assign(unsigned(sr*.15)+8,0.f);
 preL.assign(unsigned(sr*.25)+4,0.f);preR.assign(preL.size(),0.f);
 smooth=float(1.-std::exp(-1./(.025*sr)));mode=-1;reset();
 }
 void reset(){for(auto& l:lines){std::fill(l.b.begin(),l.b.end(),0.f);l.p=0;l.lp=0;}std::fill(preL.begin(),preL.end(),0.f);std::fill(preR.begin(),preR.end(),0.f);pos=0;wet=0;}
 void set(int character,float mix,float predelay,float decay,float damping,float spread){
 if(preL.empty())return;
 character=std::clamp(character,0,3);
 constexpr float scale[]={.65f,.9f,1.25f,1.9f};
 constexpr float ms[]={29.7f,37.1f,41.1f,43.7f,53.1f,59.3f,67.7f,71.9f};
 for(unsigned i=0;i<8;++i){
 auto& l=lines[i];const unsigned n=std::clamp(unsigned(sr*.001*ms[i]*scale[character]),1u,unsigned(l.b.size()));
 if(mode!=character){l.n=n;l.p%=n;}
 gains[i]=float(std::pow(.001,double(n)/(sr*std::clamp(decay,.2f,12.f))));
 }
 mode=character;target=std::clamp(mix,0.f,1.f)*.65f;
 pre=std::min(unsigned(sr*.001*std::clamp(predelay,0.f,200.f)),unsigned(preL.size()-1));
 width=std::clamp(spread,0.f,1.f);damp=.05f+.9f*(1.f-std::clamp(damping,0.f,1.f));
 }
 void process(float& l,float& r){
 if(preL.empty())return;
 wet+=(target-wet)*smooth;
 const unsigned q=(pos+unsigned(preL.size())-pre)%unsigned(preL.size());
 preL[pos]=l;preR[pos]=r;const float x=preL[q],y=preR[q];
 if(++pos>=preL.size())pos=0;
 float sum=0;std::array<float,8> v{};
 for(unsigned i=0;i<8;++i){auto& d=lines[i];d.lp+=damp*(d.b[d.p]-d.lp);v[i]=d.lp;sum+=v[i];}
 float a=0,b=0;
 for(unsigned i=0;i<8;++i){auto& d=lines[i];d.b[d.p]=.25f*(i%2?y:x)+gains[i]*(v[i]-.25f*sum);if(++d.p>=d.n)d.p=0; a+=v[i]*(i%2?1.f:-1.f);b+=v[i]*(i%4<2?1.f:-1.f);}
 a*=.25f;b*=.25f;const float mid=(a+b)*.5f;
 l=l*(1-wet)+wet*(mid+(a-mid)*width);r=r*(1-wet)+wet*(mid+(b-mid)*width);
 }
};
}
