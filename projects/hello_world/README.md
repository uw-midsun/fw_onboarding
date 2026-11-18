# Hello World

FW101 prints `Hello World` and an incrementing counter once per second using the master task scheduler. The counter is incremented by a function through a pointer.

Run from the repository root in the configured Linux development environment:

```sh
scons sim --project=hello_world --platform=x86
```

The program runs continuously. Press Ctrl+C to stop it.
