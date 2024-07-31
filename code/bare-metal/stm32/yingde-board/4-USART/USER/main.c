#include "ulib_if.h"

void usart_receive_handler(void)
{
    uint8_t data = ulib_uart_receive8(0);
    ulib_uart_send8(0, data);
}

int main(void)
{   
    ulib_uart_init(0, 115200, ULIB_UART_INTERRUPT_MODE_RX);
    ulib_uart_set_handler(0, ULIB_UART_INTERRUPT_MODE_RX, usart_receive_handler);
    
    printf("Hello~");
    
    while (1) {

    }
}
