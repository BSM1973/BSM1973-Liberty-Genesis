#include "../src/GenesisSynth.h"
#include <array>
#include <cassert>
#include <cmath>
int main() {
    liberty::GenesisSynth synth;
    synth.setSampleRate(48000.0);
    std::array<float, 512> left{}, right{};
    synth.noteOn(69, 0.9f);
    synth.process(left.data(), right.data(), static_cast<int>(left.size()));
    bool audible = false;
    for (size_t i = 0; i < left.size(); ++i) {
        assert(std::isfinite(left[i]));
        assert(left[i] == right[i]);
        if (std::abs(left[i]) > 0.00001f) audible = true;
    }
    assert(audible);
    synth.noteOff(69);
    for (int i = 0; i < 100; ++i) synth.process(left.data(), right.data(), 512);
    for (float sample : left) assert(std::abs(sample) < 0.00001f);
    synth.allNotesOff();
}
