/**
 * @file cmnd_executer.h
 * @brief PathWire command dispatcher and executor
 *
 * This file defines the cmnd_executer class, which is responsible for
 * matching parsed PathWire command frames against a static path table
 * and invoking the corresponding user-defined handlers.
 *
 * The executer performs lightweight validation, CSV parsing, and
 * type dispatching before calling the registered handler.
 *
 * Design goals:
 * - Deterministic execution
 * - No dynamic memory allocation
 * - No blocking operations
 * - Minimal overhead for embedded systems
 *
 * @note This component assumes that frames are syntactically valid.
 *       Structural validation is performed by cmnd_parser.
 */
#ifndef PATHWIRE_INC_CORE_CMND_EXECUTER_H_
#define PATHWIRE_INC_CORE_CMND_EXECUTER_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "core/ring_buffer.h"
#include "core/cmnd_frame.h"


/**
 * @def MAX_CSV_ITEMS
 * @brief Maximum number of CSV elements parsed per command
 *
 * This limit bounds stack usage and ensures predictable execution time.
 */
#define MAX_CSV_ITEMS 8


/**
 * @enum data_type
 * @brief Supported PathWire data payload types
 *
 * The detected or expected data type determines how the data payload
 * is parsed and passed to the handler.
 */
enum class data_type : uint8_t
{
    NONE,     ///< No data payload (trigger command)
    INT,      ///< Comma-separated signed integers (e.g. "1,-2,3")
    FLOAT,    ///< Comma-separated floats (e.g. "1.25,-0.5")
    STRING    ///< Comma-separated strings (e.g. "foo,bar")
};


/**
 * @typedef path_handler
 * @brief User-defined command handler function type
 *
 * Handlers are invoked when a parsed command path matches an entry
 * in the path table and the data type matches the expected type.
 *
 * @param type  Detected data type
 * @param data  Pointer to parsed data array (type-dependent)
 * @param count Number of elements in the parsed data array
 *
 * @note The data pointer is valid only for the duration of the call.
 */
typedef void (*path_handler)(
    data_type type,
    const void* data,
    uint16_t count
);


/**
 * @struct path_entry
 * @brief Static command dispatch table entry
 *
 * Each entry defines:
 * - A command path string
 * - The expected data type
 * - The handler function to invoke
 *
 * The table is typically defined as a constant array by the user.
 */
struct path_entry
{
    const char*  path;           ///< Null-terminated command path
    data_type    expected_type;  ///< Expected payload data type
    path_handler handler;        ///< Handler function
};


/**
 * @class cmnd_executer
 * @brief Dispatches parsed PathWire commands to user handlers
 *
 * cmnd_executer consumes parsed command frames from a frame queue,
 * matches them against a static path table, and invokes the
 * corresponding handler.
 *
 * Execution flow:
 * 1. Pop a cmnd_frame from the frame queue
 * 2. Match the frame path against the path table
 * 3. Detect and validate the data type
 * 4. Parse CSV data into a temporary buffer
 * 5. Invoke the registered handler
 *
 * Responsibilities:
 * - Path matching
 * - Data type detection and validation
 * - CSV parsing
 * - Handler dispatch
 *
 * Non-responsibilities:
 * - Frame parsing
 * - Memory allocation
 * - Command scheduling or threading
 *
 * @note At most one command is executed per poll() call.
 * @note Commands with mismatched types are silently dropped.
 */
class cmnd_executer
{
public:

    /**
     * @brief Constructs a command executer
     *
     * @param frame_buffer Ring buffer containing parsed command frames
     * @param table        Pointer to a static path_entry table
     * @param table_size   Number of entries in the path table
     *
     * @note The path table and buffers must outlive this object.
     */
    cmnd_executer(ring_buffer<cmnd_frame>& frame_buffer,
                  const path_entry* table,
                  uint16_t table_size);

    /**
     * @brief Executes the next available command
     *
     * Pops a single command frame from the frame queue, matches it
     * against the path table, and invokes the corresponding handler.
     *
     * If no frame is available, this function returns immediately.
     *
     * @note This function never blocks.
     * @note This function executes at most one command per call.
     */
    void poll();

private:
    ring_buffer<cmnd_frame>& frame_queue;

    const path_entry* path_table;
    uint16_t          path_count;
};

#endif // PATHWIRE_INC_CORE_CMND_EXECUTER_H_
