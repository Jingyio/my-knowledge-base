#include "stm32f10x.h"
#include "ulib_if.h"

int main(void)
{    
    ulib_pin_init(0, 0, PIN_DIR_DIGITAL_IN, PIN_MODE_FLOATING);
    ulib_pin_init(0, 1, PIN_DIR_DIGITAL_IN, PIN_MODE_FLOATING);
    ulib_pin_init(1, 0, PIN_DIR_DIGITAL_OUT, PIN_MODE_PP);
    ulib_pin_init(1, 1, PIN_DIR_DIGITAL_OUT, PIN_MODE_PP);
    
    while (1) {
        if (ulib_pin_get(PORT_A, 0))
            ulib_pin_set(PORT_B, 0, PIN_HIGH);
        else
            ulib_pin_set(PORT_B, 0, PIN_LOW);
        
        if (ulib_pin_get(PORT_A, 1))
            ulib_pin_set(PORT_B, 1, PIN_HIGH);
        else
            ulib_pin_set(PORT_B, 1, PIN_LOW);            
    }
}
