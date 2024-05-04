#include "stm32f10x.h"
#include "ulib_if.h"

int main(void)
{   
    ulib_pin_init(PORT_B, 0, PIN_DIR_DIGITAL_OUT, PIN_MODE_PP);
    ulib_pin_init(PORT_B, 1, PIN_DIR_DIGITAL_OUT, PIN_MODE_PP);
    
    while (1) {
        ulib_pin_set(PORT_B, 0, 0);
        ulib_pin_set(PORT_B, 1, 1);
        ulib_delay_ms(1000);
        ulib_pin_set(PORT_B, 0, 1);
        ulib_pin_set(PORT_B, 1, 0);
        ulib_delay_ms(1000);
    }
}
