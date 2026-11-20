#include "../src/GenesisMpcMidiAdapter.h"
#include <array>
#include <cmath>
#include <iostream>
int main() {
    liberty::GenesisMpcMidiAdapter adapter;
    adapter.engine().synth().setSampleRate(48000.0);
    std::array<float, 512> left{}, right{};
    // Deliberately unordered events: the adapter must sort by sample offset.
    const liberty::MpcMidiMessage events[] = {
        {384, 0x80, 60, 0}, {128, 0x90, 60, 100},
        {600, 0x90, 72, 100}, {0, 0xB0, 123, 0}
    };
    adapter.process(left.data(), right.data(), 512, events, 4);
    bool audible = false;
    for (int i = 0; i < 512; ++i) {
        if (!std::isfinite(left[i]) || left[i] != right[i]) return 1;
        if (i < 128 && left[i] != 0.f) return 2;
        if (i >= 128 && i < 384 && std::abs(left[i]) > 0.00001f) audible = true;
    }
    if (!audible) return 3;
    std::cout << "MPC MIDI bridge regression passed\n";
    return 0;
}
