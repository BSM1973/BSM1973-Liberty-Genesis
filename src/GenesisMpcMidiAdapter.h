#pragma once
#include "GenesisAudioEngine.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace liberty {
// Host-neutral bridge for VST-style 3-byte MIDI messages. No SDK ABI is
// assumed here: the MPC plugin entry point must translate SDK events into
// these messages. All operations are allocation-free.
struct MpcMidiMessage {
    int sampleOffset = 0;
    std::uint8_t status = 0;
    std::uint8_t data1 = 0;
    std::uint8_t data2 = 0;
};
class GenesisMpcMidiAdapter {
public:
    static constexpr std::size_t kMaxEvents = 256;
    GenesisAudioEngine& engine() noexcept { return engine_; }
    void process(float* left, float* right, int frames,
                 const MpcMidiMessage* messages, std::size_t count) noexcept {
        std::array<MidiEvent, kMaxEvents> translated{};
        std::size_t n = 0;
        if (!messages) count = 0;
        // Insertion sort into a bounded fixed array, preserving equal-offset order.
        for (std::size_t i = 0; i < count && n < kMaxEvents; ++i) {
            const auto& m = messages[i];
            if (m.sampleOffset < 0 || m.sampleOffset >= frames) continue;
            const auto op = m.status & 0xF0;
            MidiEvent event{};
            event.sampleOffset = m.sampleOffset;
            event.note = static_cast<std::uint8_t>(m.data1 & 0x7F);
            event.velocity = static_cast<float>(m.data2 & 0x7F) / 127.f;
            if (op == 0x90) event.type = event.velocity > 0.f ? MidiEvent::Type::NoteOn : MidiEvent::Type::NoteOff;
            else if (op == 0x80) event.type = MidiEvent::Type::NoteOff;
            else if (op == 0xB0 && m.data1 == 123) event.type = MidiEvent::Type::AllNotesOff;
            else continue;
            std::size_t j = n++;
            while (j > 0 && translated[j - 1].sampleOffset > event.sampleOffset) {
                translated[j] = translated[j - 1];
                --j;
            }
            translated[j] = event;
        }
        engine_.process(left, right, frames, translated.data(), n);
    }
private:
    GenesisAudioEngine engine_{};
};
}
