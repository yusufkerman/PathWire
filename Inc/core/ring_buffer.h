/**
 * @file ring_buffer.h
 * @brief Generic fixed-size lock-free ring buffer
 *
 * This file defines a simple, allocation-free ring buffer suitable
 * for embedded systems and real-time environments.
 *
 * Design characteristics:
 * - Single producer / single consumer
 * - No dynamic memory allocation
 * - Constant-time push/pop
 * - Overflow-safe
 */
#pragma once

#include <stdint.h>

/**
 * @class ring_buffer
 * @brief Fixed-size circular FIFO buffer
 *
 * @tparam T Type of elements stored in the buffer
 *
 * The buffer uses externally supplied storage and does not perform
 * any memory management internally.
 *
 * @note Not thread-safe without external synchronization.
 */
template<typename T>
class ring_buffer
{
public:
    /**
     * @brief Constructs a ring buffer
     *
     * @param buffer      Pointer to pre-allocated storage array
     * @param bufferSize  Number of elements in the buffer
     *
     * @note bufferSize must be > 1
     */
    ring_buffer(T* buffer, uint16_t bufferSize)
        : buffer(buffer),
          buffer_size(bufferSize),
          prd(0),
          cns(0)
    {
    }

    /**
     * @brief Pushes an item into the buffer
     *
     * @param item Element to push
     * @return true if successful, false if buffer is full
     */
    bool push(const T& item)
    {
        uint16_t next = (prd + 1) % buffer_size;
        if (next == cns)
        {
            return false; // Buffer full
        }

        buffer[prd % buffer_size] = item;
        prd = next;
        return true;
    }

    /**
     * @brief Pops an item from the buffer
     *
     * @param out Reference to receive the popped element
     * @return true if successful, false if buffer is empty
     */
    bool pop(T& out)
    {
        if (cns == prd)
        {
            return false; // Buffer empty
        }

        out = buffer[cns % buffer_size];
        cns = (cns + 1) % buffer_size;
        return true;
    }

private:
    T*       buffer;      ///< Pointer to backing storage
    uint16_t buffer_size; ///< Number of elements in buffer
    uint16_t prd;         ///< Producer index
    uint16_t cns;         ///< Consumer index
};
