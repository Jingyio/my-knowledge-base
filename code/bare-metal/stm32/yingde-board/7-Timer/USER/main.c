#include "ulib_if.h"

void timer_handler(void)
{
    static int i = 0;
    uint8_t value = 0;
    
    if (i >= 1000) {
        ulib_gpio_get(ULIB_GPIO_PORTB, 1, &value);
        ulib_gpio_set(ULIB_GPIO_PORTB, 1, !value);
        i = 0;
    }
    
    i++;
}

int main(void)
{
    ulib_gpio_init(ULIB_GPIO_PORTB, 1, ULIB_GPIO_DIR_OUT);
    ulib_timer_start(1, 1000, timer_handler);
    
    for ( ;; ) {
    
    }
}
