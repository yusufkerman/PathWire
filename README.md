# PathWire

Lightweight, text-based command and telemetry protocol for embedded systems.

PathWire is designed for resource-constrained microcontrollers where simplicity,
determinism, and zero dynamic memory usage are critical. It provides a clean and
human-readable way to send commands and telemetry data between embedded targets
and host systems (PC, SBC, simulator, etc.).

---

## Features

- Text-based, human-readable protocol
- No dynamic memory allocation
- Low RAM footprint
- ISR- and RTOS-friendly design
- Platform-independent core
- Simple path-based command routing
- Suitable for UART, USB CDC, or similar byte-stream transports

---

## Design Goals

PathWire was built with the following goals in mind:

- Predictable memory usage
- Easy debugging over serial terminals
- Clear separation between protocol core and hardware ports
- Minimal dependencies
- Straightforward integration into existing embedded projects

---

## Architecture Overview

PathWire consists of three main core components:

1. **Command Parser**
   - Consumes a byte stream
   - Parses incoming data into command frames

2. **Command Executer**
   - Matches parsed frames against a path table
   - Dispatches data to user-defined handlers

3. **Command Sender**
   - Formats and transmits telemetry or responses
   - Uses the same frame format as incoming commands

Platform- or MCU-specific code is isolated under the `port/` directory and does
not leak into the core logic.

---

## Frame Format

PathWire uses a simple text-based frame structure:
{p:<path>:d:<csv_data>}

Examples:
{p:ctrl/arm:d:1}
{p:sens/IMU/gyro:d:0.01,0.02,0.03}
{p:system/reset:d:}

- `path` identifies the command or data endpoint
- `data` is a comma-separated list of values
- Empty data fields are allowed

---

## Threading Model

PathWire itself is **not thread-safe**.

In RTOS-based systems:
- `parser.poll()` and `executer.poll()` should be called from a single task
- Any shared transport (UART, USB, etc.) must be protected externally
  (e.g. mutex or critical section)

This design keeps the core lightweight and avoids unnecessary synchronization
overhead.

---

## Typical Usage Flow

1. Receive bytes from transport (UART, USB, etc.)
2. Push bytes into an RX ring buffer
3. Periodically call:
   - `cmnd_parser.poll()`
   - `cmnd_executer.poll()`
4. Use `cmnd_sender` to transmit telemetry or responses

---

## Intended Use Cases

- Flight controllers
- Robotics and motion control systems
- Embedded telemetry links
- Hardware-in-the-loop (HIL) simulations
- Debug and control interfaces for MCUs

---

## Limitations

- No built-in CRC or checksum (yet)
- Text-based format (not optimal for high-bandwidth streaming)
- No internal synchronization

These trade-offs are intentional to keep the protocol simple and predictable.

---

## Future Work

Planned or possible extensions include:

- Binary transport layer
- Optional CRC support
- Streaming frame support
- PC / host-side libraries (C++, Python, C#)
- Documentation examples and protocol tooling

---

## License

PathWire is released under the MIT License.

You are free to use, modify, and distribute the code, including for commercial
purposes, as long as the original copyright and license notice are preserved.

---

## Author

Developed by Yusuf Kerman.
