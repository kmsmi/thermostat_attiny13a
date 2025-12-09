#ifndef SOFT_UART_H_
#define SOFT_UART_H_


// int16_t uart_recieve(uint8_t *rb);

extern void uart_send(uint8_t tb);

extern void uart_write(const uint8_t *buffer, uint8_t size);

extern void uart_init();

#endif // !SOFT_UART_H_
