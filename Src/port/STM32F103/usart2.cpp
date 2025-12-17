#include "port/STM32F103/usart2.h"

static inline uint16_t usart_calc_brr(uint32_t pclk, uint32_t baudrate);

/* -------- RX BUFFER -------- */
uint8_t usart2_rx_storage[512];
ring_buffer<uint8_t> usart2_rx_buffer(
    usart2_rx_storage,
    sizeof(usart2_rx_storage)
);

/* -------- TX BUFFER -------- */
uint8_t usart2_tx_storage[512];
ring_buffer<uint8_t> usart2_tx_buffer(
    usart2_tx_storage,
    sizeof(usart2_tx_storage)
);

usart2::usart2(uint32_t baudrate) : baudrate(baudrate) {}

void usart2::init()
{
    config_gpio();
    config_usart();
    enable_irq();
}

void usart2::config_gpio()
{
    RCC->APB2ENR |= IOPAEN;

    GPIOA->CRL &= ~(
        (0xFU << GPIOA_CRL_PA2_MODE_Pos) |
        (0xFU << GPIOA_CRL_PA3_MODE_Pos)
    );

    GPIOA->CRL |=
        GPIOA_CRL_PA2_MODE_50MHz |
        GPIOA_CRL_PA2_CNF_AF_PP |
        GPIOA_CRL_PA3_MODE_INPUT |
        GPIOA_CRL_PA3_CNF_FLOAT;
}

void usart2::config_usart()
{
    RCC->APB1ENR |= USART2EN;

    USART2->BRR = usart_calc_brr(PCLK1_FREQ, baudrate);

    USART2->CR1 |= USART2_CR1_TE | USART2_CR1_RE;
    USART2->CR1 |= USART2_CR1_RXNEIE;

    USART2->CR1 |= USART2_CR1_UE;
}

void usart2::enable_irq()
{
    NVIC_ClearPendingIRQ(USART2_IRQn);
    NVIC_SetPriority(USART2_IRQn, 9);
    NVIC_EnableIRQ(USART2_IRQn);
}

/* -------- TX KICK (C API) -------- */
extern "C" void usart2_tx_kick(void)
{
	USART2->CR1 |= USART_CR1_TXEIE;
}

/* -------- ISR -------- */
extern "C" void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        uint8_t data = (uint8_t)USART2->DR;
        usart2_rx_buffer.push(data);
    }

    if (USART2->SR & USART_SR_TXE)
    {
        uint8_t byte;
        if (usart2_tx_buffer.pop(byte))
        {
            USART2->DR = byte;
        }
        else
        {
            USART2->CR1 &= ~USART_CR1_TXEIE;
        }
    }
}

static inline uint16_t usart_calc_brr(uint32_t pclk, uint32_t baudrate)
{
    uint32_t usartdiv_times_16 = (pclk + (baudrate / 2U)) / baudrate;
    uint32_t mantissa = usartdiv_times_16 / 16U;
    uint32_t fraction = usartdiv_times_16 % 16U;

    return (uint16_t)((mantissa << 4) | (fraction & 0xFU));
}
