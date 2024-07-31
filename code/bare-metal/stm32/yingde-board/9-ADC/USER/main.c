#include "ulib_if.h"

int main(void)
{ 	
    float value = 0;
    
	ulib_uart_init(0, 115200, ULIB_UART_INTERRUPT_MODE_OFF);
    printf("Hello\n");
    ulib_adc_init(ULIB_GPIO_PORTA, 3);
   

	while(1) {
        value = ulib_adc_get(ULIB_GPIO_PORTA, 3);
        printf("%f\n", value);
        ulib_delay_us(1000000);
    }
}
