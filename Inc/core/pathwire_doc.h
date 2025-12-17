/**
 * @mainpage PathWire
 *
 * @section intro Introduction
 *
 * **PathWire** is a lightweight, text-based command and telemetry protocol
 * designed for embedded systems.
 *
 * Design goals:
 * - Low RAM footprint
 * - No dynamic memory allocation
 * - ISR and RTOS friendly
 * - Platform-independent core
 *
 * @section architecture Architecture
 *
 * PathWire consists of three core components:
 *
 * - **cmnd_parser**   → RX byte stream → frames
 * - **cmnd_executer** → frame → handler dispatch
 * - **cmnd_sender**   → telemetry & command TX
 *
 * Platform-specific code is isolated under `/port`
 * and kept strictly separate from the core.
 *
 * @section frame_format Frame Format
 *
 * ```
 * {p:<path>:d:<csv_data>}
 * ```
 *
 * Examples:
 * - `{p:ctrl/arm:d:1}`
 * - `{p:sens/IMU/gyro:d:0.01,0.02,0.03}`
 * - `{p:system/reset:d:}`
 *
 * @section threading Threading Model
 *
 * PathWire is not inherently thread-safe.
 * External synchronization (e.g. mutex) is required
 * when used in multi-task environments.
 *
 * @section usage Typical Usage
 *
 * - UART RX → `ring_buffer<uint8_t>`
 * - `cmnd_parser.poll()`
 * - `cmnd_executer.poll()`
 *
 * @section future Future Work
 *
 * - Binary transport
 * - CRC support
 * - Streaming frames
 * - C# / PC host library
 */
