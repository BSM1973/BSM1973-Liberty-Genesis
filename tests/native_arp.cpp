#include "LibertyGenesis.cpp"
#include <cstdlib>
#include <iostream>
struct GenesisArpTimingTest {
 static void run(){
  double worst=0;int cases=0;
  constexpr double divisions[]={1.,.5,.25,.125,1./3.,1./6.,.75,.375};
  for(double sr:{44100.,48000.,96000.})for(double bpm:{40.,90.,120.,137.,300.})for(double div:divisions)for(double swing:{0.,.35,.75}){
   LibertyGenesis p;p.arpSR=sr;p.arpPeriod=sr*60./bpm*div;p.arpSwing=swing;p.arpGate=.65;p.arpNote(60,true);
   double ideal=0;int steps=0;long long sample=0;
   while(steps<64){
    int before=p.arpStep;p.arpTick();
    if(p.arpStep!=before){
     double error=std::abs(double(sample)-ideal);worst=std::max(worst,error);
     if(error>1.00001){std::cerr<<"Timing error "<<sr<<" "<<bpm<<" "<<div<<" "<<swing<<" "<<error<<"\n";std::exit(1);}
     ideal+=p.arpPeriod*(1.+(steps%2?-1.:1.)*swing*.5);++steps;
    }
    ++sample;
   }
   ++cases;
  }
  std::cout<<cases<<" cases, max timing error "<<worst<<" samples\n";
 }
};
int main(){GenesisArpTimingTest::run();}
