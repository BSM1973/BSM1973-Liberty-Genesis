# Platform compatibility — verification required

This repository currently contains a portable C++17 sound engine and a desktop smoke test.

It is NOT an MPC 3, MPC Key 37, or Force plugin yet. No Akai-specific binary, SDK integration, packaging, signing, UI, or hardware validation has been implemented.

Before targeting any device:
1. Confirm current official Akai developer documentation, SDK access and supported plugin format for that specific device/firmware.
2. Check CPU architecture, real-time audio API, MIDI API, display constraints and distribution rules.
3. Build a minimal supported host integration and test on hardware.
4. Only then mark a device compatible.
