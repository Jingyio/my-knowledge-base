#include "stm32f10x.h"
#include "ulib_if.h"

int main(void)
{   
    ulib_gpio_init(ULIB_GPIO_PORTB, 0, ULIB_GPIO_DIR_OUT);
    ulib_gpio_init(ULIB_GPIO_PORTB, 1, ULIB_GPIO_DIR_OUT);
    
    while (1) {
        ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);
        ulib_gpio_set(ULIB_GPIO_PORTB, 1, 1);
        ulib_delay_us(1000000);
        ulib_gpio_set(ULIB_GPIO_PORTB, 0, 1);
        ulib_gpio_set(ULIB_GPIO_PORTB, 1, 0);
        ulib_delay_us(1000000);
    }
}
