#include <stdlib.h>
#include "ulib_if.h"
#include "oled.h"

uint8_t g_src[2048] = {0};
uint8_t g_dest[2048] = {0};

void key1_handler(void)
{
    uint8_t value = 0;
    
    ulib_gpio_get(ULIB_GPIO_PORTB, 0, &value);
    ulib_gpio_set(ULIB_GPIO_PORTB, 0, !value);
}

void key2_handler(void)
{
    uint8_t value = 0;
    
    ulib_gpio_get(ULIB_GPIO_PORTB, 1, &value);
    ulib_gpio_set(ULIB_GPIO_PORTB, 1, !value);
}

static int memcmp(uint8_t* ptr_src, uint8_t* ptr_dest, uint32_t size)
{
    uint32_t i = 0;
    
    for (i = 0; i < size; i++) {
        if (ptr_src[i] != ptr_dest[i])
            return -1;
    }
    
    return 0;
}

int test(void)
{
    const char* msg = "Hello";
    int i = 0;
    int j = 0;
    
    // GPIO
    ulib_gpio_init(ULIB_GPIO_PORTB, 0, ULIB_GPIO_DIR_OUT);
    ulib_gpio_init(ULIB_GPIO_PORTB, 1, ULIB_GPIO_DIR_OUT);
    ulib_gpio_set(ULIB_GPIO_PORTB, 0, 1);
    ulib_gpio_set(ULIB_GPIO_PORTB, 1, 1);
    
    // EXTI
    ulib_exti_init(ULIB_GPIO_PORTA, 0, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_init(ULIB_GPIO_PORTA, 1, ULIB_EXTI_TRIGGER_FALLING);
    ulib_exti_set_handler(ULIB_GPIO_PORTA, 0, key1_handler);
    ulib_exti_set_handler(ULIB_GPIO_PORTA, 1, key2_handler);
    
    // UART
    ulib_uart_init(0, 115200, UART_INTERRUPT_MODE_OFF);
    for (i = 0; i < 5; i++)
        ulib_uart_send8(0, msg[i]);
    
    // DMA
    for (i = 0; i < 2048; i++) {
        g_src[i] = rand() % 255;
    }
    ulib_dma_memcpy(0, g_src, g_dest, 2048);
    ulib_dma_wait4done(0);
    if (!memcmp(g_src, g_dest, 2048))
        ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);
    
    // I2C
    oled_init();
    oled_reverse(0);
    oled_display_on();
    oled_rotate(0);
    oled_clear();
    
    for (i = 0; i < 128; i++) {
        for (j = 0; j < 10; j++) {
            oled_buf_draw(i, j, 1);
        }
    }
    oled_buf_render();
    
    while (1) {
    
    }
}
