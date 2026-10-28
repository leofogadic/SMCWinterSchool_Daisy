<h1 align="center">SMC Winter School</h1>

<p align="center">
  Build sensor-driven instruments on the <a href="https://electro-smith.com/products/daisy-seed">Daisy Seed</a>.<br>
  A ready-to-go project template with drop-in classes for 13 sensors.
</p>

---

## What's in here

| Folder | What it is |
| --- | --- |
| [`SMCProject/`](SMCProject/) | **Your project.** Start here. |
| [`SMCProject/sensors/`](SMCProject/sensors/) | One small, readable class per sensor |
| [`SMCProject/setup/`](SMCProject/setup/) | Starts the hardware, USB serial and audio |
| `libDaisy/`, `DaisySP/` | Daisy hardware library and DSP library (included, no submodules) |
| `ci/`, `cube/`, `dist/`, `utils/`, … | Tooling inherited from [DaisyExamples](https://github.com/electro-smith/DaisyExamples) |

## Getting started

### 1. Install the toolchain

Follow the Daisy guide for your OS: [daisy.audio/tutorials/cpp-dev-env](https://daisy.audio/tutorials/cpp-dev-env/).
You need `arm-none-eabi-gcc`, `make`, and `dfu-util`. VS Code is recommended.

### 2. Get the code and build the libraries (once)

```sh
git clone https://github.com/leofogadic/SMCWinterSchool_Daisy.git
cd SMCWinterSchool_Daisy
./ci/build_libs.sh
```

### 3. Build and flash

Put the Daisy in bootloader mode: hold **BOOT**, tap **RESET**, release **BOOT**. Then:

```sh
cd SMCProject
make
make program-dfu
```

In VS Code you can instead open `SMCProject/` and run the task **build_and_program_dfu**.
No terminal? Flash `SMCProject/build/SMCProject.bin` with the [Daisy Web Programmer](https://electro-smith.github.io/Programmer/).

### 4. See the output

The program waits until a serial monitor is connected over USB, then starts.
Use the VS Code Serial Monitor, the Arduino Serial Monitor, or:

```sh
screen /dev/tty.usbmodem* 115200
```

> If the board seems to do nothing after flashing, it's waiting for the serial monitor.
> To start without one, change `hw.StartLog(true)` to `hw.StartLog(false)` in [`setup/Setup.h`](SMCProject/setup/Setup.h).

## How a project is structured

[`main.cpp`](SMCProject/main.cpp) has two places where your code goes:

```cpp
void AudioCallback(...)   // Runs every 1 ms (48 samples at 48 kHz). Make sound here.
                          // Keep it fast: no sensor reads, no printing, no delays.

int main(void)
{
    Setup(hardware, AudioCallback);
    // 1. Init your sensors
    // 2. StartAnalogInputs(hardware) if you use any analog sensor

    while(1)
    {
        // 3. Read sensors, print, update variables the audio callback uses
    }
}
```

Read sensors in the `while` loop, store the values in global variables,
and use those variables in `AudioCallback` to shape the sound.

### Example: a pot that controls a sine wave

```cpp
#include "daisy_seed.h"
#include "daisysp.h"
#include "setup/Setup.h"
#include "sensors/sensors.h"

using namespace daisy;
using namespace daisysp;

DaisySeed     hardware;
Potentiometer pot;
Oscillator    osc;

volatile float freq = 440.f;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    osc.SetFreq(freq);
    for(size_t i = 0; i < size; i++)
        out[0][i] = out[1][i] = osc.Process();
}

int main(void)
{
    osc.Init(48000.f);
    Setup(hardware, AudioCallback);

    pot.Init(hardware, seed::A0);
    StartAnalogInputs(hardware);

    while(1)
    {
        freq = 100.f + pot.Read() * 900.f; // 100 – 1000 Hz
        hardware.PrintLine("freq: " FLT_FMT3, FLT_VAR3(freq));
        System::Delay(10);
    }
}
```

> `osc.Init()` is called before `Setup()` because `Setup()` starts audio right away.

## Sensors

Include everything with `#include "sensors/sensors.h"`. Pins below are suggestions; any pin of the right type works.

### Analog sensors

All return a value from `0.0` to `1.0` with `Read()`.
**After initializing all of them, call `StartAnalogInputs(hardware)` once.** Up to 12 analog channels.

| Sensor | Class | Init | Extras |
| --- | --- | --- | --- |
| Potentiometer | `Potentiometer` | `pot.Init(hardware, seed::A0)` | |
| Piezo disc | `Piezo` | `piezo.Init(hardware, seed::A1)` | |
| Flex sensor | `FlexSensor` | `flex.Init(hardware, seed::A2)` | |
| Stretch cord | `StretchSensor` | `stretch.Init(hardware, seed::A7)` | |
| Force sensor (FSR) | `ForceSensor` | `force.Init(hardware, seed::A8)` | `IsPressed(threshold)` |
| Joystick (KY-023) | `Joystick` | `joy.Init(hardware, xPin, yPin, swPin)` | `X()`, `Y()` from `-1` to `1`; call `Update()` every loop |

Flex, stretch and force sensors need a voltage divider: `3V3 → sensor → pin → 10 kΩ → GND`.

### Digital

| Sensor | Class | Init | Use |
| --- | --- | --- | --- |
| Button | `Button` | `button.Init(seed::A3)` (add `, true` for internal pull-up) | Call `Update()` every loop |
| RGB LED (common cathode) | `ColorLed` | `led.Init(rPin, gPin, bPin)` | `Red()`, `Cyan()`, `Set(r, g, b)`, `Off()`… |
| Ultrasonic HC-SR04 | `Ultrasonic` | `sonar.Init(seed::D0, seed::D1)` (Trig, Echo) | `Read()` → cm, or `-1` |

`Button` gives you `IsPressed()`, `JustPressed()`, `JustReleased()`, `IsHeld(ms)`, `HeldTime()`, `LastPressTime()`, `Count()` and `Toggled()`. Debouncing is built in.

The ultrasonic sensor measures in the background on a hardware timer, so `Read()` never waits.
Up to 4 can be used; they take turns, so each updates about every 60 ms × number of sensors.
The HC-SR04 Echo pin outputs 5 V; use a voltage divider to bring it down to 3.3 V.

### I²C sensors

All share I²C1: **SCL = D11, SDA = D12**. `Init()` returns `false` if the sensor isn't found,
so check it. Call `Read()` in the loop, then use the values.

| Sensor | Class | Values after `Read()` |
| --- | --- | --- |
| Accelerometer (Adafruit MMA8451) | `Accelerometer` | `x`, `y`, `z` in m/s² (±2 g) |
| Barometer (Adafruit BMP585) | `Barometer` | `temperature` in °C, `pressure` in Pa |
| Capacitive touch (Adafruit MPR121) | `CapacitiveTouch` | `IsTouched(pad)`, `JustTouched(pad)`, `JustReleased(pad)`, `Raw(pad)` for pads 0–11 |

```cpp
Accelerometer accel;

if(!accel.Init())
    hardware.PrintLine("Accelerometer not found, check the wiring");
```

`CapacitiveTouch::Init(touch, release)` takes optional thresholds (default `12`, `6`). Lower is more sensitive.

## Printing values

```cpp
hardware.PrintLine("button count: %d", (int)button.Count());
hardware.PrintLine("x: " FLT_FMT3, FLT_VAR3(accel.x));   // floats need FLT_FMT3 / FLT_VAR3
```

Printing is slow. Never print inside `AudioCallback`, and add a small `System::Delay()` in the loop if you print a lot.

## Troubleshooting

| Problem | Try |
| --- | --- |
| `make: arm-none-eabi-g++: No such file` | Toolchain not installed or not on your `PATH` |
| `cannot find -ldaisy` | Run `./ci/build_libs.sh` from the repo root |
| `make program-dfu` can't find the device | Enter bootloader mode again (hold BOOT, tap RESET) |
| Flashed, but nothing happens | Open a serial monitor (see [step 4](#4-see-the-output)) |
| I²C sensor `Init()` returns `false` | Check SCL/SDA aren't swapped, and the sensor has power and GND |
| Analog value stuck at 0 | Did you call `StartAnalogInputs(hardware)` after all `Init()`s? |
| Clicks or dropouts in audio | Something slow is in `AudioCallback`; move it to the loop |

## Links

- [Daisy documentation](https://daisy.audio/)
- [libDaisy reference](https://daisy.audio/software/libdaisy/) · [DaisySP reference](https://daisy.audio/software/daisysp/)
- [Daisy Seed pinout](https://daisy.audio/hardware/Seed/)
- [Daisy forum](https://forum.electro-smith.com/)

## License

MIT, see [LICENSE](LICENSE). Based on [electro-smith/DaisyExamples](https://github.com/electro-smith/DaisyExamples).
