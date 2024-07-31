#include "stm32f10x.h"
#include "ulib_if.h"

uint8_t src[128] = {"Data from src group\0"};
uint8_t dest[128] = {0};

int main(void)
{   
    int i = 0;
    
    ulib_uart_init(0, 115200, ULIB_UART_INTERRUPT_MODE_OFF);
    ulib_dma_memcpy(0, src, dest, 128);
    ulib_dma_wait4done(0);
    
    for (i = 0; i < 19; i++) {
        ulib_uart_send8(0, dest[i]);
    }
    
    while (1) {
        ulib_uart_send8(0, ulib_uart_receive8(0));
    }
}
