/**
 * @file cmnd_frame.h
 * @brief Lightweight container for parsed PathWire command frames
 *
 * This file defines the cmnd_frame structure, which represents a
 * fully parsed PathWire command without owning any memory.
 *
 * The frame contains references (pointers + lengths) to the original
 * parsing buffer. No copying or allocation is performed.
 *
 * Example command:
 *   {p:/motor/set,d:1200}
 *
 * Parsed frame:
 *   path = "/motor/set"
 *   data = "1200"
 */
#ifndef PATHWIRE_INC_CORE_CMND_FRAME_H_
#define PATHWIRE_INC_CORE_CMND_FRAME_H_

#include <stdint.h>

/**
 * @struct cmnd_frame
 * @brief Parsed PathWire command representation
 *
 * A cmnd_frame holds non-owning references to a command's path and data
 * sections. The memory backing these pointers must remain valid while
 * the frame is in use.
 *
 * @note This structure performs no validation.
 * @note Lifetime is managed externally by the parser buffer.
 */
struct cmnd_frame
{
    const char* path;     ///< Pointer to command path string
    uint16_t    path_len; ///< Length of the path string (excluding null terminator)

    const char* data;     ///< Pointer to data payload (CSV or raw)
    uint16_t    data_len; ///< Length of data payload
};

#endif // PATHWIRE_INC_CORE_CMND_FRAME_H_
