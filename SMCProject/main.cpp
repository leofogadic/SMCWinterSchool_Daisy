#include "daisy_seed.h"
#include "setup/Setup.h"
#include "sensors/sensors.h"

using namespace daisy;

DaisySeed hardware;

// Declare sensors here


// Runs every block (48 samples = 1 ms at 48 kHz).
// Keep it fast: no sensor reads, no printing, no delays in here.
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = 0.f;
        out[1][i] = 0.f;
    }
}

int main(void)
{
    // Hardware, serial and audio (see setup/Setup.h)
    Setup(hardware, AudioCallback);

    while(1)
    {
        // Read sensors here
    }
}
