# Firmware tests

`check_adc.c` tests the ADS1115 driver using mocked I2C and delay functions. It covers register byte order, channel selection, signed voltage conversion, invalid arguments, error propagation and the hardware-only settling delay.

In the team's Linux development container, mount the repository at `/workspace` and a writable results directory at `/results`, then run:

```sh
bash verification/run_linux_tests.sh
```

The runner executes both ADC mock variants, builds `hello_world` and `fw_102_103`, and checks eight-second x86 simulations. Logs are written to `/results`.

For interactive simulation from the repository root:

```sh
scons sim --project=hello_world --platform=x86
scons sim --project=fw_102_103 --platform=x86
```

Stop each simulator with Ctrl+C. These checks simulate hardware; physical board operation and wiring require separate validation.
