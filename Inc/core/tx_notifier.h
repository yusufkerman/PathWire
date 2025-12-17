/**
 * @file tx_notifier.h
 * @brief Transmission-ready notification interface
 *
 * This file defines a lightweight callback mechanism used to notify
 * the system when transmission resources are available.
 *
 * Typical use cases:
 * - UART TX buffer ready
 * - DMA transfer completed
 * - Host communication unblocked
 */
#ifndef PATHWIRE_INC_CORE_TX_NOTIFIER_H_
#define PATHWIRE_INC_CORE_TX_NOTIFIER_H_

/**
 * @typedef tx_notify_fn
 * @brief Transmission notification callback type
 *
 * Called when the system is ready to transmit data.
 *
 * @note Must be non-blocking.
 * @note Should execute quickly.
 */
typedef void (*tx_notify_fn)();

/**
 * @brief Registers a transmission-ready callback
 *
 * @param fn Callback function to register
 *
 * @note Only one notifier is supported.
 * @note Passing nullptr disables notifications.
 */
void register_tx_notifier(tx_notify_fn fn);

/**
 * @brief Notifies that transmission is ready
 *
 * Invokes the registered callback, if any.
 *
 * @note Safe to call from ISR if implementation allows.
 */
void notify_tx_ready();

#endif // PATHWIRE_INC_CORE_TX_NOTIFIER_H_
