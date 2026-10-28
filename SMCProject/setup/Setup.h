#pragma once

#include "daisy_seed.h"

using namespace daisy;

// Audio block size in samples (48 = 1 ms at 48 kHz)
const size_t kAudioBlockSize = 48;

// Starts the Daisy Seed: hardware, serial over USB and audio.
// Call once at the start of main(), before initializing sensors.
inline void Setup(DaisySeed& hw, AudioHandle::AudioCallback callback)
{
    hw.Configure();
    hw.Init();

    // Serial over USB (true = wait until the serial monitor is open)
    hw.StartLog(true);

    hw.SetAudioBlockSize(kAudioBlockSize);
    hw.StartAudio(callback);
}
