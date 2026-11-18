#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace liberty {
class GenesisSynth {
public:
    static constexpr int maxVoices = 16;
    void setSampleRate(double rate) { sampleRate = std::max(8000.0, rate); }
    void setGain(float value) { gain = std::clamp(value, 0.0f, 1.0f); }
    void noteOn(int note, float velocity) {
        if (note < 0 || note > 127) return;
        Voice* target = nullptr;
        for (auto& voice : voices) if (!voice.active) { target = &voice; break; }
        if (!target) { target = &voices[nextVoice++ % maxVoices]; }
        *target = {};
        target->active = true;
        target->note = note;
        target->frequency = 440.0 * std::pow(2.0, (note - 69) / 12.0);
        target->velocity = std::clamp(velocity, 0.0f, 1.0f);
    }
    void noteOff(int note) {
        for (auto& voice : voices) if (voice.active && voice.note == note)
            voice.releasing = true;
    }
    void allNotesOff() { for (auto& voice : voices) voice = {}; }
    float processSample() {
        double mix = 0.0;
        for (auto& voice : voices) {
            if (!voice.active) continue;
            const double attack = 1.0 / (0.008 * sampleRate);
            const double release = 1.0 / (0.18 * sampleRate);
            voice.envelope = voice.releasing
                ? std::max(0.0, voice.envelope - release)
                : std::min(1.0, voice.envelope + attack);
            if (voice.releasing && voice.envelope <= 0.0) { voice.active = false; continue; }
            const double phaseStep = voice.frequency / sampleRate;
            voice.phase += phaseStep;
            voice.phase -= std::floor(voice.phase);
            mix += std::sin(voice.phase * 6.283185307179586) * voice.envelope * voice.velocity;
        }
        return static_cast<float>(std::tanh(mix * gain * 0.22));
    }
    void process(float* left, float* right, int frames) {
        if (!left || !right || frames <= 0) return;
        for (int i = 0; i < frames; ++i) {
            const float sample = processSample();
            left[i] = sample;
            right[i] = sample;
        }
    }
private:
    struct Voice {
        bool active = false, releasing = false;
        int note = -1;
        double phase = 0.0, frequency = 440.0, envelope = 0.0;
        float velocity = 0.0f;
    };
    std::array<Voice, maxVoices> voices {};
    double sampleRate = 48000.0;
    float gain = 0.8f;
    unsigned nextVoice = 0;
};
} // namespace liberty
