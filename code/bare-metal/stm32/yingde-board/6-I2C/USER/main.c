#include "ulib_if.h"
#include "oled.h"

int main(void)
{
    int i = 0;
    int j = 0;
    int k = 0;
    
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
