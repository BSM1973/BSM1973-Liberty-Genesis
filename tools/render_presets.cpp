#include "../src/GenesisPresetBrowser.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

static void u16(std::ofstream& f, uint16_t n) {
 for(int i=0;i<2;++i) f.put(static_cast<char>((n>>(8*i))&255));
}
static void u32(std::ofstream& f, uint32_t n) {
 for(int i=0;i<4;++i) f.put(static_cast<char>((n>>(8*i))&255));
}
int main(int argc,char** argv){
 liberty::GenesisPresetBrowser browser;
 if(argc==2 && std::string(argv[1])=="--list"){
  for(std::size_t i=0;i<browser.count();++i) std::cout<<i<<": "<<browser.at(i)->name<<"\n";
  return 0;
 }
 if(argc!=4 || std::string(argv[1])!="--render"){
  std::cerr<<"Usage: genesis_presets --list | --render INDEX OUTPUT.wav\n";
  return 1;
 }
 std::size_t index=0;
 try {
  const std::string input(argv[2]);
  std::size_t used=0;
  index=std::stoul(input,&used);
  if(used!=input.size())return 2;
 } catch(...) {return 2;}
 if(index>=browser.count())return 2;
 constexpr int rate=48000, seconds=5, frames=rate*seconds;
 std::ofstream out(argv[3],std::ios::binary);
 if(!out)return 3;
 out.write("RIFF",4);u32(out,36+frames*4);
 out.write("WAVEfmt ",8);u32(out,16);u16(out,1);u16(out,2);
 u32(out,rate);u32(out,rate*4);u16(out,4);u16(out,16);
 out.write("data",4);u32(out,frames*4);
 liberty::GenesisSynth synth;
 synth.setSampleRate(rate);
 browser.load(synth,index);
 const int chord[]={48,55,60,64};
 for(int i=0;i<frames;++i){
  if(i==0)for(int note:chord)synth.noteOn(note,.55f);
  if(i==rate*3)for(int note:chord)synth.noteOff(note);
  float sample=std::clamp(synth.processSample(),-1.f,1.f);
  auto pcm=static_cast<uint16_t>(static_cast<int16_t>(sample*32767.f));
  u16(out,pcm);u16(out,pcm);
 }
 if(!out)return 4;
 std::cout<<"Rendered "<<browser.at(index)->name<<" to "<<argv[3]<<"\n";
 return 0;
}
