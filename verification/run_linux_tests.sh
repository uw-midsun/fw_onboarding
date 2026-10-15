#!/usr/bin/env bash
set -euo pipefail
cd /workspace
exec > >(tee /results/test-run.log) 2>&1
trap 'status=$?; if [ "$status" -ne 0 ]; then printf "FAIL: runner exited %s\n" "$status" | tee /results/result.txt; fi' EXIT
echo 'Running real C driver tests with mocked I2C'
gcc -std=gnu11 -Wall -Wextra -Werror -DMS_PLATFORM_X86 \
  -Ilibraries/ms-common/inc -Ilibraries/ms-drivers/inc \
  verification/check_adc.c libraries/ms-drivers/src/ads1115.c -lm -o /tmp/check_adc
/tmp/check_adc
gcc -std=gnu11 -Wall -Wextra -Werror \
  -Ilibraries/ms-common/inc -Ilibraries/ms-drivers/inc \
  verification/check_adc.c libraries/ms-drivers/src/ads1115.c -lm -o /tmp/check_adc_hw
/tmp/check_adc_hw
echo 'Building FW101'
scons --project=hello_world --platform=x86 -j2
echo 'Building FW102/103'
scons --project=fw_102_103 --platform=x86 -j2
capture_simulation() {
  local project="$1"
  set +e
  timeout --signal=TERM --kill-after=2s 8s stdbuf -oL "build/x86/bin/projects/$project" "$project" > "/results/$project.log" 2>&1
  local status=$?
  set -e
  cat "/results/$project.log"
  if [ "$status" -ne 124 ]; then
    echo "Simulation $project exited unexpectedly: $status"
    exit 1
  fi
}
echo 'Running FW101 for eight seconds'
capture_simulation hello_world
echo 'Running FW102/103 for eight seconds'
capture_simulation fw_102_103
python3 - <<'PY'
import pathlib, re
counter_log = pathlib.Path('/results/hello_world.log').read_text()
counter = [int(v) for v in re.findall(r'Hello World: (\d+)', counter_log)]
assert len(counter) >= 5, f'Not enough counter samples: {counter}'
assert all(b == a + 1 for a, b in zip(counter, counter[1:])), counter
sensor_log = pathlib.Path('/results/fw_102_103.log').read_text()
readings = [float(v) for v in re.findall(r'ads1115_reader: (-?\d+\.\d+) V', sensor_log)]
assert len(readings) >= 5, f'Not enough sensor samples: {readings}'
assert all(1.368 <= v <= 1.432 for v in readings), readings
assert 'blinky: ON' in sensor_log and 'blinky: OFF' in sensor_log
assert 'queued reading' in sensor_log
assert not re.search(r'failed|timed out|queue full|FIFO full|out of sync', sensor_log, re.I), sensor_log
print(f'PASS: {len(counter)} consecutive counter values; {len(readings)} sensor readings; LED toggles; no task errors')
PY
printf 'PASS: ADC mock tests, hardware-delay mock test, both builds and both eight-second simulations\n' | tee /results/result.txt
