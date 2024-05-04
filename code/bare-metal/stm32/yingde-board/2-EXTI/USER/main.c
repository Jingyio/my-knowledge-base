#include "stm32f10x.h"
#include "ulib_if.h"

void key1_handler(void)
{
    static int count = 0;
    
    if (count % 2) {
        ulib_pin_set(PORT_B, 0, 0);
        ulib_pin_set(PORT_B, 1, 1);
    } else {
        ulib_pin_set(PORT_B, 0, 1);
        ulib_pin_set(PORT_B, 1, 0);    
    }
    
    count++;
}

int main(void)
{
    ulib_pin_init(PORT_B, 0, PIN_DIR_DIGITAL_OUT, PIN_MODE_PP);
    ulib_pin_init(PORT_B, 1, PIN_DIR_DIGITAL_OUT, PIN_MODE_PP);
    
    
    ulib_exti_init(PORT_A, 0, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_A, 0, key1_handler);
    
    ulib_exti_init(PORT_A, 1, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_A, 1, key1_handler);

    
    ulib_exti_init(PORT_B, 3, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 3, key1_handler);

    ulib_exti_init(PORT_B, 4, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 4, key1_handler);

    ulib_exti_init(PORT_B, 5, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 5, key1_handler);

    ulib_exti_init(PORT_B, 6, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 6, key1_handler);

    ulib_exti_init(PORT_B, 7, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 7, key1_handler);

    ulib_exti_init(PORT_B, 8, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 8, key1_handler);
    
    ulib_exti_init(PORT_B, 9, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 9, key1_handler);
    
    ulib_exti_init(PORT_B, 10, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 10, key1_handler);
    
    ulib_exti_init(PORT_B, 11, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 11, key1_handler);
    
    ulib_exti_init(PORT_B, 12, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 12, key1_handler);
    
    ulib_exti_init(PORT_B, 13, EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(PORT_B, 13, key1_handler);
    
 
    while (1);
}
