#pragma once
#include "GenesisPresets.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
namespace liberty {
class GenesisPresetBrowser {
public:
 std::size_t count() const { return factoryPresets.size(); }
 const GenesisPreset* at(std::size_t index) const {
  return index < factoryPresets.size() ? &factoryPresets[index] : nullptr;
 }
 const GenesisPreset* findByName(std::string_view name) const {
  for(const auto& preset:factoryPresets) if(preset.name==name) return &preset;
  return nullptr;
 }
 std::vector<std::size_t> search(std::string_view query) const {
  std::vector<std::size_t> matches;
  std::string needle=lower(query);
  for(std::size_t i=0;i<factoryPresets.size();++i)
   if(lower(factoryPresets[i].name).find(needle)!=std::string::npos) matches.push_back(i);
  return matches;
 }
 bool load(GenesisSynth& synth,std::size_t index) const {
  auto* preset=at(index);
  if(!preset)return false;
  applyPreset(synth,*preset);
  return true;
 }
private:
 static std::string lower(std::string_view input) {
  std::string result(input);
  std::transform(result.begin(),result.end(),result.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
  return result;
 }
};
}
