#include "ulib_if.h"
#include "oled.h"

void ulib_delay_us(
    uint32_t us
);

void key1_handler(void)
{
    static uint8_t state = 0;
    
    state = !state;
    ulib_gpio_set(ULIB_GPIO_PORTB, 0, state);
}

void key2_handler(void)
{
    static uint8_t state = 0;
    
    state = !state;
    ulib_gpio_set(ULIB_GPIO_PORTB, 1, state);
}

int main(void)
{
    int i = 0;
    int j = 0;
    int k = 0;
    
    ulib_uart_init(0, 115200, ULIB_UART_INTERRUPT_MODE_OFF);
    
    ulib_gpio_init(ULIB_GPIO_PORTB, 0, ULIB_GPIO_DIR_OUT);
    ulib_gpio_init(ULIB_GPIO_PORTB, 1, ULIB_GPIO_DIR_OUT);
    
    ulib_exti_init(ULIB_GPIO_PORTA, 0, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_init(ULIB_GPIO_PORTA, 1, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTA, 0, key1_handler);
    ulib_exti_set_handler(ULIB_GPIO_PORTA, 1, key2_handler);
    
    printf("hello");
    oled_init();
    oled_reverse(0);
    oled_display_on();
    oled_rotate(0);
    oled_clear();
    
    while (1) {
        ulib_delay_us(1000000);
        
        for (i = 0; i < 128; i++) {
            for (j = 0; j < k; j++) {
                oled_buf_draw(i, j, 1);
            }
        }
        oled_buf_render();

        k++;
        if (k > 32) {
        for (i = 0; i < 128; i++) {
            for (j = 0; j < 32; j++) {
                    oled_buf_draw(i, j, 0);
                }
            }
            oled_buf_render();
            k = 0;
        }
    }
}
