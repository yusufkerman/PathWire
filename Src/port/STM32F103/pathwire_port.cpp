#include "port/STM32F103/pathwire_port.h"
#include "core/tx_notifier.h"

extern "C" void usart2_tx_kick(void);

static void tx_ready_handler()
{
    usart2_tx_kick();
}

void pathwire_port_init()
{
    register_tx_notifier(tx_ready_handler);
}
