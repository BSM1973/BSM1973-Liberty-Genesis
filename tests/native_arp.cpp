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
  // Exercise the real process callback at a tempo change, with zero frames
  // so that only transport/parameter handling changes the scheduler.
  for(double sr:{44100.,48000.,96000.})for(double from:{60.,120.,180.})for(double to:{60.,120.,180.}){
   LibertyGenesis p;p.prepare(sr,256);
   p.set(LibertyGenesis::kArpOn,1);p.set(LibertyGenesis::kArpRate,2);
   float dummy=0;float* outputs[]={&dummy,&dummy};
   mpc::ProcessContext ctx{};ctx.outputs=outputs;ctx.numOutputs=2;ctx.numFrames=0;
   ctx.transport.tempoValid=true;ctx.transport.bpm=from;p.process(ctx);
   p.arpNote(60,true);p.arpTick();
   for(int i=0;i<100;++i)p.arpTick();
   const double step=p.untilStep,off=p.untilOff,period=p.arpPeriod;
   ctx.transport.bpm=to;p.process(ctx);
   if(std::abs(p.arpPeriod-sr*60./to*.25)>1e-7 ||
      std::abs(p.untilStep-step*p.arpPeriod/period)>1e-7 ||
      std::abs(p.untilOff-off*p.arpPeriod/period)>1e-7){
    std::cerr<<"Tempo transition phase mismatch\n";std::exit(1);
   }
  }
  std::cout<<"27 tempo transition cases passed; ";
  std::cout<<cases<<" cases, max timing error "<<worst<<" samples\n";
 }
};
int main(){GenesisArpTimingTest::run();}
