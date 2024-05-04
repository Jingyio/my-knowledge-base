#include "stm32f10x.h"
#include "ulib_if.h"

void usart_receive_handler(void)
{
    uint8_t data = ulib_uart_receive8(0);
    ulib_uart_send8(0, data);
}

int main(void)
{   
    ulib_uart_init(0, 115200);
//    ulib_uart_set_handler(0, usart_receive_handler);
    
    while (1) {
        ulib_uart_send8(0, ulib_uart_receive8(0));
    }
}
