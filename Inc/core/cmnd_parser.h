/**
 * @file cmnd_parser.h
 * @brief PathWire command frame parser
 *
 * This file defines the cmnd_parser class, responsible for parsing
 * incoming PathWire byte streams into structured command frames.
 *
 * The parser operates as a non-blocking, byte-oriented state machine
 * suitable for embedded systems and interrupt-driven RX paths.
 *
 * Frame format:
 *   {p:<path>:d:<data>}
 *
 * Example:
 *   {p:sensor/imu:d:1.0,2.0,3.0}
 *
 * Design goals:
 * - No dynamic memory allocation
 * - No blocking operations
 * - Deterministic, linear-time parsing
 * - ISR-safe when used with an ISR-safe ring_buffer
 *
 * @note Parsed frames reference slices of the provided work buffer.
 *       The consumer must process frames before the buffer is reused.
 */
#ifndef PATHWIRE_INC_CORE_CMND_PARSER_H_
#define PATHWIRE_INC_CORE_CMND_PARSER_H_

#include <stdint.h>
#include "core/ring_buffer.h"
#include "core/cmnd_frame.h"

/**
 * @class cmnd_parser
 * @brief Incremental PathWire frame parser
 *
 * cmnd_parser consumes raw bytes from an RX ring buffer and incrementally
 * parses them into complete PathWire command frames.
 *
 * Parsing is performed using an explicit finite state machine (FSM),
 * allowing the parser to:
 * - Resume parsing across multiple poll() calls
 * - Recover from malformed input
 * - Operate safely in low-latency embedded environments
 *
 * Responsibilities:
 * - Byte-wise parsing of PathWire frames
 * - Validation of frame structure
 * - Extraction of path and data substrings
 * - Emission of cmnd_frame objects into a frame queue
 *
 * Non-responsibilities:
 * - Command execution
 * - Data type validation or conversion
 * - Buffer ownership or memory management
 *
 * @note This class does NOT copy path or data strings.
 *       Pointers inside cmnd_frame refer to the shared work buffer.
 */
class cmnd_parser {

public:

    /**
     * @brief Constructs a PathWire command parser
     *
     * @param rx_buffer        RX ring buffer providing incoming bytes
     * @param frame_buffer     Output ring buffer for parsed command frames
     * @param work_buffer      Scratch buffer used to assemble frames
     * @param work_buffer_size Size of the scratch buffer in bytes
     *
     * @note All buffers must outlive the cmnd_parser instance.
     * @note The work buffer must be large enough to hold the largest
     *       expected PathWire frame.
     */
    cmnd_parser(ring_buffer<uint8_t>& rx_buffer,
                ring_buffer<cmnd_frame>& frame_buffer,
                char* work_buffer,
                uint16_t work_buffer_size);

    /**
     * @brief Resets the parser to its initial state
     *
     * Clears internal indices, pointers, and state machine state.
     *
     * This function is automatically invoked on:
     * - Frame completion
     * - Buffer overflow detection
     * - Parser error recovery
     */
    void reset();

    /**
     * @brief Processes available RX data
     *
     * Consumes bytes from the RX ring buffer and advances the internal
     * state machine accordingly.
     *
     * This function:
     * - May be called periodically from the main loop
     * - May be called after RX interrupts
     * - Never blocks
     *
     * @note At most one frame is emitted per successful parse sequence.
     * @note Malformed frames are silently discarded.
     */
    void poll();

private:
    ring_buffer<uint8_t>& rx_queue;
    ring_buffer<cmnd_frame>& frame_queue;

    /**
     * @brief Internal parser state machine states
     *
     * The parser advances deterministically through these states
     * while consuming the input stream.
     */
    enum class state_t : uint8_t {
        WAIT_START,     ///< Waiting for '{'
        WAIT_P,         ///< Expecting 'p'
        WAIT_P_COLON,   ///< Expecting ':'
        READ_PATH,      ///< Reading path string
        WAIT_D,         ///< Expecting 'd'
        WAIT_D_COLON,   ///< Expecting ':'
        READ_DATA,      ///< Reading CSV data payload
        WAIT_END,       ///< Waiting for '}'
        ERROR           ///< Error recovery state
    };

    // ------------------------------------------------------------------
    // Working buffer state
    // ------------------------------------------------------------------

    char*    workBuffer;      ///< Scratch buffer used to assemble frames
    uint16_t work_buf_size;   ///< Total size of the scratch buffer
    uint16_t idx;             ///< Current write index into the buffer

    // ------------------------------------------------------------------
    // Frame field pointers
    // ------------------------------------------------------------------

    const char* path_ptr;     ///< Pointer to parsed path string
    uint16_t    path_len;     ///< Length of the parsed path

    const char* data_ptr;     ///< Pointer to parsed data payload
    uint16_t    data_len;     ///< Length of the parsed data

    state_t state;            ///< Current parser FSM state
};

#endif // PATHWIRE_INC_CORE_CMND_PARSER_H_
