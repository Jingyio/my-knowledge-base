#include "easylib.h"

sbit led1 = P2^0;
sbit led2 = P2^1;
sbit led3 = P2^2;
sbit led4 = P2^3;

void delay_ms(uint32_t ms)
{
    uint32_t i = 0;
    uint32_t j = 0;
    
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--)
            ;
}

void on_ext0_trigger(void)
{
    unsigned int i = 0;
    
    led1 = !led1;
    
    for (i = 0; i < 1000; i++)
        ;
}

void on_ext1_trigger(void)
{
    unsigned int i = 0;
    
    led2 = !led2;
    
    for (i = 0; i < 1000; i++)
        ;
}

void on_timer0_trigger(void)
{
    static unsigned int i = 0;
    
    if (i < 20) {
        i += 1;
    } else {
        i = 0;
        led3 = !led3;
    }
}

void main(void)
{
    uint8_t *ptr_buf = 0;
    
    configure_exint(EXTERNAL0, TRIGGER_LEVEL, on_ext0_trigger);
    configure_exint(EXTERNAL1, TRIGGER_FALLING, on_ext1_trigger);
    configure_timer(TIMER0, 1, 0x4C, 0x00, 1, on_timer0_trigger);
    
    configure_interrupt_priority(EXTERNAL0, LEVEL_1);
    configure_interrupt_priority(EXTERNAL1, LEVEL_1);
    configure_interrupt_priority(TIMER0, LEVEL_1);
    
    set_interrupt_state(EXTERNAL0, INT_ON);
    set_interrupt_state(EXTERNAL1, INT_ON);
    set_interrupt_state(TIMER0, INT_ON);
    
    delay_ms(5000);
    set_power_down_mode();
    
    while (1) {
    }
}