#include "stm32f10x.h"
#include "ulib_if.h"

int main(void)
{
    uint8_t value = 0;
    
    ULIB_ASSERT(ULIB_S_OK == ulib_gpio_init(ULIB_GPIO_PORTA, 0, ULIB_GPIO_DIR_IN));
    ULIB_ASSERT(ULIB_S_OK == ulib_gpio_init(ULIB_GPIO_PORTA, 1, ULIB_GPIO_DIR_IN));
    ULIB_ASSERT(ULIB_S_OK == ulib_gpio_init(ULIB_GPIO_PORTB, 0, ULIB_GPIO_DIR_OUT));
    ULIB_ASSERT(ULIB_S_OK == ulib_gpio_init(ULIB_GPIO_PORTB, 1, ULIB_GPIO_DIR_OUT));
    
    while (1) {
        ULIB_ASSERT(ulib_gpio_get(ULIB_GPIO_PORTA, 0, &value) == ULIB_S_OK);
        if (value)
            ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);
        else
            ulib_gpio_set(ULIB_GPIO_PORTB, 0, 1);
        
        ULIB_ASSERT(ulib_gpio_get(ULIB_GPIO_PORTA, 1, &value) == ULIB_S_OK);
        if (value)
            ulib_gpio_set(ULIB_GPIO_PORTB, 1, 0);
        else
            ulib_gpio_set(ULIB_GPIO_PORTB, 1, 1);        
    }
}
