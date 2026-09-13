# FW101 — Hello World

This project follows the FW101 master-task template. `pre_loop_init()` resets
an integer counter; `run_1hz_cycle()` passes its address to `prv_increment()`
and prints the returned value using `LOG_DEBUG("Hello World %d\n", count)`.
The 100 Hz and 10 Hz callbacks are intentionally empty.

The master tasks provide the infinite loops and timing: the 1 Hz callback
runs once per second. The counter restarts at 1 if it reaches `INT_MAX`,
avoiding signed overflow during indefinite execution.

From the repository root in the Linux development environment:

```bash
scons sim --project=hello_world --platform=x86
```

The output includes:

```text
[0] projects/hello_world/src/main.c:44: Hello World 1
[0] projects/hello_world/src/main.c:44: Hello World 2
[0] projects/hello_world/src/main.c:44: Hello World 3
```

Source line numbers may change when editing. Stop with Ctrl+C. SCons may
report `Build interrupted` when you stop this indefinitely running program.

To compile for the STM32 with the ARM toolchain:

```bash
scons --project=hello_world
```

This produces `build/arm/bin/projects/hello_world.bin`; it does not flash a board.
The `--platform=x86` option selects this repository's Linux desktop simulation
backend. On Apple Silicon, use a Linux VM/container for that command.

The directory was created with `scons new --project=hello_world`. The generated
header and example test are retained; the example test is scaffold, not a test
of the counter. For the FW101 deliverable, capture the running output and show
it to a firmware lead as directed in the lesson.
