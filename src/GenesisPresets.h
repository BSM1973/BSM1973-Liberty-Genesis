#pragma once
#include "GenesisSynth.h"
#include <array>
#include <string_view>

namespace liberty {
struct GenesisPreset {
 std::string_view name;
 GenesisSynth::Waveform oscillatorA, oscillatorB;
 float mix, detune, cutoff, resonance;
 float attack, decay, sustain, release;
 float lfoRate, lfoDepth, gain;
};
inline constexpr std::array<GenesisPreset,12> factoryPresets{{
 {"Genesis Init",GenesisSynth::Waveform::Saw,GenesisSynth::Waveform::Square,.5f,7.f,8000.f,.1f,.008f,.1f,.8f,.18f,.5f,0.f,.8f},
 {"Deep Foundation",GenesisSynth::Waveform::Sine,GenesisSynth::Waveform::Triangle,.25f,0.f,450.f,.12f,.004f,.18f,.75f,.24f,.4f,0.f,.9f},
 {"Neon Pulse",GenesisSynth::Waveform::Square,GenesisSynth::Waveform::Saw,.3f,9.f,2400.f,.4f,.003f,.16f,.45f,.14f,2.2f,.15f,.75f},
 {"Glass Horizon",GenesisSynth::Waveform::Triangle,GenesisSynth::Waveform::Sine,.55f,3.f,6200.f,.15f,.45f,1.1f,.65f,2.5f,.3f,.2f,.85f},
 {"Copper Bass",GenesisSynth::Waveform::Saw,GenesisSynth::Waveform::Square,.4f,-5.f,600.f,.48f,.002f,.12f,.6f,.16f,1.f,.04f,.85f},
 {"Aurora Pad",GenesisSynth::Waveform::Saw,GenesisSynth::Waveform::Triangle,.5f,12.f,3100.f,.3f,1.2f,1.6f,.8f,3.4f,.15f,.22f,.7f},
 {"Soft Keys",GenesisSynth::Waveform::Sine,GenesisSynth::Waveform::Triangle,.3f,0.f,5200.f,.05f,.006f,.55f,.3f,.6f,.6f,0.f,.9f},
 {"Motion Field",GenesisSynth::Waveform::Saw,GenesisSynth::Waveform::Square,.5f,15.f,1500.f,.5f,.15f,.45f,.7f,.9f,4.f,.7f,.65f},
 {"Vintage Lead",GenesisSynth::Waveform::Saw,GenesisSynth::Waveform::Saw,.5f,6.f,4400.f,.22f,.012f,.16f,.85f,.22f,5.f,.08f,.7f},
 {"Pluck Orbit",GenesisSynth::Waveform::Triangle,GenesisSynth::Waveform::Square,.2f,4.f,2600.f,.2f,.002f,.25f,0.f,.12f,1.f,0.f,.8f},
 {"Submerged",GenesisSynth::Waveform::Sine,GenesisSynth::Waveform::Square,.12f,-12.f,300.f,.3f,.06f,.5f,.5f,1.2f,.12f,.35f,.85f},
 {"Starlight",GenesisSynth::Waveform::Triangle,GenesisSynth::Waveform::Saw,.35f,19.f,7800.f,.25f,.08f,.35f,.45f,1.5f,6.f,.18f,.7f}
}};
inline void applyPreset(GenesisSynth& synth,const GenesisPreset& p){
 synth.setWaveforms(p.oscillatorA,p.oscillatorB);
 synth.setOscMix(p.mix);
 synth.setDetuneCents(p.detune);
 synth.setFilter(p.cutoff,p.resonance);
 synth.setADSR(p.attack,p.decay,p.sustain,p.release);
 synth.setLFO(p.lfoRate,p.lfoDepth);
 synth.setGain(p.gain);
}
}
