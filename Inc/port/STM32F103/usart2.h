#ifndef PATHWIRE_INC_PORT_USART2_H_
#define PATHWIRE_INC_PORT_USART2_H_

#include "stm32f103xb.h"
#include "core/ring_buffer.h"

extern uint8_t usart2_rx_storage[512];
extern ring_buffer<uint8_t> usart2_rx_buffer;

extern uint8_t usart2_tx_storage[512];
extern ring_buffer<uint8_t> usart2_tx_buffer;

#define PCLK1_FREQ 8000000U

#define IOPAEN                   (1U << 2)
#define USART2EN                 (1U << 17)

#define GPIOA_CRL_PA2_MODE_Pos   8U
#define GPIOA_CRL_PA2_CNF_Pos    10U
#define GPIOA_CRL_PA2_MODE_50MHz (0b11 << GPIOA_CRL_PA2_MODE_Pos)
#define GPIOA_CRL_PA2_CNF_AF_PP  (0b10 << GPIOA_CRL_PA2_CNF_Pos)

#define GPIOA_CRL_PA3_MODE_Pos   12U
#define GPIOA_CRL_PA3_CNF_Pos    14U
#define GPIOA_CRL_PA3_MODE_INPUT (0b00 << GPIOA_CRL_PA3_MODE_Pos)
#define GPIOA_CRL_PA3_CNF_FLOAT  (0b01 << GPIOA_CRL_PA3_CNF_Pos)

#define USART2_CR1_UE            (1U << 13)
#define USART2_CR1_TE            (1U << 3)
#define USART2_CR1_RE            (1U << 2)
#define USART2_CR1_RXNEIE        (1U << 5)
#define USART2_CR1_TXEIE         (1U << 7)

class usart2
{
public:
    usart2(uint32_t baudrate);
    void init();

private:
    void config_gpio();
    void config_usart();
    void enable_irq();

    uint32_t baudrate;
};

#ifdef __cplusplus
extern "C" {
#endif

void usart2_tx_kick(void);

#ifdef __cplusplus
}
#endif

#endif
