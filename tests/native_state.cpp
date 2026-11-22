#include "LibertyGenesis.cpp"
#include <iostream>
#include <cstdlib>
void check(bool ok,const char* msg){if(!ok){std::cerr<<msg<<"\n";std::exit(1);}}
int main(){
 LibertyGenesis a,b;
 check(a.numParams()==129,"parameter count");
 check(a.numPrograms()==100,"program count");
 for(int p=0;p<100;++p){
  a.setProgram(p);
  auto state=a.internalSaveState();
  check(b.internalLoadState(state.data(),state.size()),"load factory state");
  for(int i=0;i<129;++i)check(std::abs(a.getNormalised(i)-b.getNormalised(i))<1e-5f,"factory state mismatch");
 }
 for(int pass=0;pass<3;++pass){
  for(int i=0;i<129;++i)a.setNormalised(i,pass==0?0.f:pass==1?1.f:float((i*37)%101)/100.f);
  auto state=a.internalSaveState();
  check(b.internalLoadState(state.data(),state.size()),"load edited state");
  for(int i=0;i<129;++i)check(std::abs(a.get(i)-b.get(i))<1e-4f,"edited state mismatch");
 }
 std::cout<<"100 factory states and 129 edited parameters round-trip passed\n";
}
