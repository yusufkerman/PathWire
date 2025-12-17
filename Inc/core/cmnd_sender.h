/**
 * @file cmnd_sender.h
 * @brief PathWire command frame builder and transmitter helper
 *
 * This file defines the cmnd_sender class, which is responsible for
 * building PathWire command frames and pushing them into a TX ring buffer.
 *
 * The class is designed for embedded systems and supports ISR-safe usage,
 * assuming the underlying ring_buffer implementation is ISR-safe.
 *
 * Frame format:
 *   {p:<path>:d:<data>}
 *
 * - No dynamic memory allocation
 * - No blocking operations
 * - All output is byte-wise pushed to a TX queue
 *
 * @note This component does NOT perform transmission itself.
 *       It only prepares and enqueues bytes for transmission.
 */
#ifndef PATHWIRE_INC_CORE_CMND_SENDER_H_
#define PATHWIRE_INC_CORE_CMND_SENDER_H_


#include <stdint.h>

#include "core/ring_buffer.h"
#include "core/tx_notifier.h"



/**
 * @class cmnd_sender
 * @brief Builds and serializes PathWire command frames
 *
 * cmnd_sender is a lightweight helper class that serializes commands
 * into a predefined textual frame format and pushes them into a
 * transmit ring buffer.
 *
 * Responsibilities:
 * - Frame construction ({p:<path>:d:<data>})
 * - Integer, float, and string serialization
 * - Byte-wise, ordered enqueue into TX buffer
 *
 * Non-responsibilities:
 * - Transport handling (UART, USB, TCP, etc.)
 * - Threading or scheduling
 * - Buffer ownership or lifetime management
 *
 * @note The provided ring_buffer must outlive this object.
 * @note All methods return false on TX buffer overflow.
 */
class cmnd_sender{

public:


	/**
	* @brief Constructs a command sender using an external TX buffer
	*
	* @param tx_buffer Reference to a byte-oriented ring buffer used for output
	*
	* @note The buffer is not owned by cmnd_sender.
	* @note The caller is responsible for ensuring ISR-safety if used in interrupts.
	*/
	cmnd_sender(ring_buffer<uint8_t>& tx_buffer)
	        : tx_queue(tx_buffer)
	    {}


	/**
	 * @brief Sends a command frame with no data payload
	 *
	 * Builds and enqueues a PathWire frame containing only a path
	 * and an empty data section.
	 *
 	 * Frame example:
	 *   {p:system/reset:d:}
	 *
	 * @param path Null-terminated command path string
	 * @return true if the entire frame was successfully enqueued
	 * @return false if the TX buffer overflows during frame construction
	 */
    bool send_trigger(const char* path);


    /**
     * @brief Sends a command frame containing integer data
     *
     * Serializes an array of signed 32-bit integers as a comma-separated
     * list and enqueues the resulting frame.
     *
     * Frame example:
     *   {p:ctrl/set_pid:d:10,20,-5}
     *
     * @param path   Null-terminated command path string
     * @param values Pointer to an array of int32_t values
     * @param count  Number of elements in the values array
     *
     * @return true if the entire frame was successfully enqueued
     * @return false if the TX buffer overflows during frame construction
     */
    bool send_int(
        const char* path,
        const int32_t* values,
        uint16_t count
    );



    /**
     * @brief Sends a command frame containing floating-point data
     *
     * Serializes an array of floating-point values using a fixed
     * decimal format (three fractional digits).
     *
     * Rounding is applied and fractional carry is handled to prevent
     * corrupted output (e.g. 1.999 -> 2.000).
     *
     * Frame example:
     *   {p:sensor/imu:d:1.250,-0.500,0.000}
     *
     * @param path   Null-terminated command path string
     * @param values Pointer to an array of float values
     * @param count  Number of elements in the values array
     *
     * @return true if the entire frame was successfully enqueued
     * @return false if the TX buffer overflows during frame construction
     */
    bool send_float(
        const char* path,
        const float* values,
        uint16_t count
    );



    /**
     * @brief Sends a command frame containing string data
     *
     * Serializes an array of null-terminated strings as a
     * comma-separated list.
     *
     * No escaping is performed. Strings must not contain
     * frame control characters.
     *
     * Frame example:
     *   {p:log/print:d:hello,world}
     *
     * @param path   Null-terminated command path string
     * @param values Pointer to an array of string pointers
     * @param count  Number of strings in the values array
     *
     * @return true if the entire frame was successfully enqueued
     * @return false if the TX buffer overflows during frame construction
     */
    bool send_string(
        const char* path,
        const char* const* values,
        uint16_t count
    );



private:
	ring_buffer<uint8_t>& tx_queue;


	/**
	 * @brief Starts a PathWire command frame
	 *
	 * Writes the fixed frame prefix to the TX buffer:
	 *   {p:<path>:d:
	 *
	 * This function does NOT write the closing '}' character.
	 *
     * @param path Null-terminated command path string
	 *
	 * @return true if all prefix bytes were successfully enqueued
	 * @return false if the TX buffer overflows
	 *
	 * @note Must be paired with end_frame() on success.
	 */
	bool begin_frame(const char* path);


	/**
	 * @brief Finalizes a PathWire command frame
	 *
	 * Writes the closing frame delimiter ('}') to the TX buffer.
	 *
	 * @return true if the delimiter was successfully enqueued
	 * @return false if the TX buffer overflows
	 *
	 * @note This function completes a frame started by begin_frame().
	 */
	bool end_frame();

	/**
	 * @brief Pushes a single character into the TX buffer
	 *
	 * The character is enqueued as a single byte and a TX-ready
	 * notification is issued immediately after a successful push.
	 *
	 * @param c Character to enqueue
	 *
	 * @return true if the byte was successfully enqueued
	 * @return false if the TX buffer is full
	 *
	 * @note This function is the lowest-level output primitive.
	 */
    bool push_char(char c);



    /**
     * @brief Pushes a null-terminated string into the TX buffer
     *
     * Characters are written sequentially using push_char().
     * The terminating null character is NOT transmitted.
     *
     * @param s Pointer to a null-terminated string
     *
     * @return true if all characters were successfully enqueued
     * @return false if the TX buffer overflows
     */
    bool push_string(const char* s);



    /**
     * @brief Serializes and pushes a signed 32-bit integer
     *
     * Converts the integer into its decimal ASCII representation
     * and writes it to the TX buffer without leading zeros.
     *
     * Negative values are prefixed with a minus sign ('-').
     *
     * @param v Integer value to serialize
     *
     * @return true if all characters were successfully enqueued
     * @return false if the TX buffer overflows
     */
    bool push_int(int32_t v);



    /**
     * @brief Serializes and pushes a floating-point value
     *
     * The value is converted to a fixed-point decimal representation
     * with exactly three fractional digits.
     *
     * Algorithm:
     * - Extract integer and fractional parts
     * - Apply rounding to the fractional part
     * - Handle fractional overflow carry (e.g. 1.999 -> 2.000)
     * - Zero-pad fractional digits if required
     *
     * Example outputs:
     *   1.2    -> "1.200"
     *   -0.5   -> "-0.500"
     *   3.9999 -> "4.000"
     *
     * @param v Floating-point value to serialize
     *
     * @return true if all characters were successfully enqueued
     * @return false if the TX buffer overflows
     *
     * @note This implementation avoids sprintf() to remain lightweight
     *       and ISR-friendly.
     */
    bool push_float(float v);


};


#endif
