#include "../src/GenesisSynth.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>

static void write16(std::ofstream& out, uint16_t n) {
    for (int i = 0; i < 2; ++i) out.put(static_cast<char>((n >> (8 * i)) & 255));
}
static void write32(std::ofstream& out, uint32_t n) {
    for (int i = 0; i < 4; ++i) out.put(static_cast<char>((n >> (8 * i)) & 255));
}
int main() {
    constexpr int rate = 48000, frames = rate * 4, channels = 2;
    std::ofstream out("genesis-demo.wav", std::ios::binary);
    if (!out) return 1;
    out.write("RIFF", 4); write32(out, 36 + frames * channels * 2);
    out.write("WAVEfmt ", 8); write32(out, 16); write16(out, 1);
    write16(out, channels); write32(out, rate); write32(out, rate * channels * 2);
    write16(out, channels * 2); write16(out, 16);
    out.write("data", 4); write32(out, frames * channels * 2);
    liberty::GenesisSynth synth;
    synth.setSampleRate(rate);
    const int notes[] = {60, 64, 67, 72};
    for (int i = 0; i < frames; ++i) {
        if (i % rate == 0) {
            if (i) synth.noteOff(notes[(i / rate - 1) % 4]);
            synth.noteOn(notes[(i / rate) % 4], 0.8f);
        }
        const auto sample = std::clamp(synth.processSample(), -1.0f, 1.0f);
        const auto pcm = static_cast<int16_t>(sample * 32767.0f);
        write16(out, static_cast<uint16_t>(pcm));
        write16(out, static_cast<uint16_t>(pcm));
    }
    if (!out) return 2;
    std::cout << "Wrote genesis-demo.wav\n";
}
