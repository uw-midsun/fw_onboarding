FW 102 and FW 103

The program toggles PB3 once per second and passes AIN0 voltage readings from the ADC writer to the reader through a five item queue
The sampling interval follows the one second requirement in FW103

Run in the team development container

```sh
scons --project=fw_102_103 --platform=x86
scons sim --project=fw_102_103 --platform=x86
scons test --project=fw_102_103 --platform=x86
```

The simulator supplies values near 1 point 4 V and uses the byte order of the ADS1115
It consumes outgoing I2C bytes so that the simulated transmit buffer does not fill

The ADC uses continuous conversion at 128 samples per second with the default plus or minus 2 point 048 V range
The thresholds are 0 V and 1 point 5 V
The comparator remains disabled as required by keeping its default setting
The optional alert interrupt is not implemented

The driver preserves other configuration bits when choosing one of the four inputs
Register transfers send the high byte first
Each conversion step is 62 point 5 microvolts
Read failures return before a voltage is placed in the queue

The source includes a 20 ms settling delay on ARM after selecting a channel at the configured 128 samples per second
I2C1 uses PB6 for SCL and PB7 for SDA as specified in the STM32L433 datasheet
The FW102 example reverses these pins
Confirm the actual controller board wiring before flashing
No physical board or ADC has been tested

The implementation was prepared with AI assistance
Review the register settings error handling and task priorities before explaining the work to a lead

[FW102 guide](https://uwmidsun.atlassian.net/wiki/spaces/Firmware/pages/1268121797)
[FW103 guide](https://uwmidsun.atlassian.net/wiki/spaces/Firmware/pages/1273464270)
[ADS1115 datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf)
[STM32L433 datasheet](https://www.st.com/resource/en/datasheet/stm32l433cc.pdf)
