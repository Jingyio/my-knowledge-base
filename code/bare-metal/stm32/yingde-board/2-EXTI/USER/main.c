#include "stm32f10x.h"
#include "ulib_if.h"

void key1_handler(void)
{
    static int count = 0;
    
    if (count % 2) {
        ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);
        ulib_gpio_set(ULIB_GPIO_PORTB, 1, 1);
    } else {
        ulib_gpio_set(ULIB_GPIO_PORTB, 0, 1);
        ulib_gpio_set(ULIB_GPIO_PORTB, 1, 0);    
    }
    
    count++;
}

int main(void)
{
    ulib_gpio_init(ULIB_GPIO_PORTB, 0, ULIB_GPIO_DIR_OUT);
    ulib_gpio_init(ULIB_GPIO_PORTB, 1, ULIB_GPIO_DIR_OUT);
    
    ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);

    ulib_exti_init(ULIB_GPIO_PORTA, 0, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTA, 0, key1_handler);
    
    ulib_exti_init(ULIB_GPIO_PORTA, 1, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTA, 1, key1_handler);

    ulib_exti_init(ULIB_GPIO_PORTB, 3, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 3, key1_handler);

    ulib_exti_init(ULIB_GPIO_PORTB, 4, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 4, key1_handler);

    ulib_exti_init(ULIB_GPIO_PORTB, 5, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 5, key1_handler);

    ulib_exti_init(ULIB_GPIO_PORTB, 6, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 6, key1_handler);

    ulib_exti_init(ULIB_GPIO_PORTB, 7, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 7, key1_handler);

    ulib_exti_init(ULIB_GPIO_PORTB, 8, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 8, key1_handler);
    
    ulib_exti_init(ULIB_GPIO_PORTB, 9, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 9, key1_handler);
    
    ulib_exti_init(ULIB_GPIO_PORTB, 10, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 10, key1_handler);
    
    ulib_exti_init(ULIB_GPIO_PORTB, 11, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 11, key1_handler);
    
    ulib_exti_init(ULIB_GPIO_PORTB, 12, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 12, key1_handler);
    
    ulib_exti_init(ULIB_GPIO_PORTB, 13, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTB, 13, key1_handler);
    
 
    while (1);
}

