# SMC Winter School – Project

Your Daisy Seed project. Write your code in [`main.cpp`](main.cpp).

```sh
make                # build
make program-dfu    # flash over USB (hold BOOT, tap RESET first)
```

- [`sensors/`](sensors/): one class per sensor, all included by `sensors/sensors.h`
- [`setup/Setup.h`](setup/Setup.h): starts the hardware, USB serial and audio

For setup, wiring and the full sensor reference, see the [main README](../README.md).
