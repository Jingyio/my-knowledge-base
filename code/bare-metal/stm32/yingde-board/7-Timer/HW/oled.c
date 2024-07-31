#include "oled.h"
#include <stdlib.h>
#include "oledfont.h"
#include "ulib_if.h"

static uint8_t g_ram_buffer[128][8];

static void oled_write_byte(uint8_t dat, uint8_t mode)
{
    ulib_i2c_start(I2C_PORT_OLED);
    ulib_i2c_write(I2C_PORT_OLED, 0x78);
    ulib_i2c_wait_ack(I2C_PORT_OLED);

    if (mode) {
        ulib_i2c_write(I2C_PORT_OLED, 0x40);
    } else {
        ulib_i2c_write(I2C_PORT_OLED, 0x00);
    }

    ulib_i2c_wait_ack(I2C_PORT_OLED);
    ulib_i2c_write(I2C_PORT_OLED, dat);
    ulib_i2c_wait_ack(I2C_PORT_OLED);
    ulib_i2c_stop(I2C_PORT_OLED);
}

void oled_reverse(uint8_t is_reversed)
{
    if (is_reversed)
        oled_write_byte(0xA7, OLED_CMD);
    else
        oled_write_byte(0xA6, OLED_CMD);
}

void oled_rotate(uint8_t is_rotated)
{
    if (is_rotated) {
        oled_write_byte(0xC0, OLED_CMD);
        oled_write_byte(0xA0, OLED_CMD);
    } else {
        oled_write_byte(0xC8, OLED_CMD);
        oled_write_byte(0xA1, OLED_CMD);
    }
}

void oled_display_on(void)
{
    oled_write_byte(0x8D, OLED_CMD);
    oled_write_byte(0x14, OLED_CMD);
    oled_write_byte(0xAF, OLED_CMD);
}

void oled_display_off(void)
{
    oled_write_byte(0x8D, OLED_CMD);
    oled_write_byte(0x10, OLED_CMD);
    oled_write_byte(0xAE, OLED_CMD);
}


void oled_buf_render(void)
{
    uint8_t i = 0;
    uint8_t j = 0;

    for (i = 0; i < OLED_PAGE_NUM; i++) {
        oled_write_byte(0xb0 + i, OLED_CMD);
        oled_write_byte(0x00,     OLED_CMD);
        oled_write_byte(0x10,     OLED_CMD);
        
        ulib_i2c_start(I2C_PORT_OLED);
        ulib_i2c_write(I2C_PORT_OLED, 0x78);
        ulib_i2c_wait_ack(I2C_PORT_OLED);
        ulib_i2c_write(I2C_PORT_OLED, 0x40);
        ulib_i2c_wait_ack(I2C_PORT_OLED);

        for (j = 0; j < OLED_PAGE_SIZE; j++) {
            ulib_i2c_write(I2C_PORT_OLED, g_ram_buffer[j][i]);
            ulib_i2c_wait_ack(I2C_PORT_OLED);
        }

        ulib_i2c_stop(I2C_PORT_OLED);
    }
}

void oled_clear(void)
{
    uint8_t i = 0;
    uint8_t j = 0;

    for (i = 0; i < OLED_PAGE_NUM; i++) {
        for (j = 0; j < OLED_PAGE_SIZE; j++) {
            g_ram_buffer[j][i] = 0;
        }
    }

    oled_buf_render();
}

void oled_buf_draw(uint8_t x, uint8_t y, uint8_t bit)
{
    uint8_t row_start = 0;
    uint8_t row_offset = 0;
    uint8_t byte_data = 0;
    
    row_start = y / 8;
    row_offset = y % 8;
    byte_data = 1 << row_offset;

    if (bit) {
        g_ram_buffer[x][row_start] |= byte_data;
    } else {
        g_ram_buffer[x][row_start] = ~g_ram_buffer[x][row_start];
        g_ram_buffer[x][row_start] |= byte_data;
        g_ram_buffer[x][row_start] = ~g_ram_buffer[x][row_start];
    }
}

void oled_init(void)
{
    ulib_i2c_pins_t i2c0_pins = {
        .scl_port  = ULIB_GPIO_PORTA,
        .scl_pin   = 2,
        .sda_port  = ULIB_GPIO_PORTA,
        .sda_pin   = 3,
    };
    
    ulib_i2c_config_t i2c_config = {
        .i2c_pins = i2c0_pins,
        .i2c_index = I2C_PORT_OLED,
        .i2c_mode = ULIB_I2C_MODE_SW,
    };
    
    ulib_i2c_init(i2c_config);

    oled_write_byte(0xAE, OLED_CMD); //--turn off oled panel
    oled_write_byte(0x00, OLED_CMD); //--set low column address
    oled_write_byte(0x10, OLED_CMD); //--set high column address
    oled_write_byte(0x40, OLED_CMD); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    oled_write_byte(0x81, OLED_CMD); //--set contrast control register
    oled_write_byte(0xCF, OLED_CMD); //--Set SEG Output Current Brightness
    oled_write_byte(0xA1, OLED_CMD); //--Set SEG/Column Mapping
    oled_write_byte(0xC8, OLED_CMD); //--Set COM/Row Scan Direction
    oled_write_byte(0xA6, OLED_CMD); //--set normal display
    oled_write_byte(0xA8, OLED_CMD); //--set multiplex ratio(1 to 64)
    oled_write_byte(0x3f, OLED_CMD); //--1/64 duty
    oled_write_byte(0xD3, OLED_CMD); //--set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    oled_write_byte(0x00, OLED_CMD); //--not offset
    oled_write_byte(0xd5, OLED_CMD); //--set display clock divide ratio/oscillator frequency
    oled_write_byte(0x80, OLED_CMD); //--set divide ratio, Set Clock as 100 Frames/Sec
    oled_write_byte(0xD9, OLED_CMD); //--set pre-charge period
    oled_write_byte(0xF1, OLED_CMD); //--Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    oled_write_byte(0xDA, OLED_CMD); //--set com pins hardware configuration
    oled_write_byte(0x12, OLED_CMD);
    oled_write_byte(0xDB, OLED_CMD); //--set vcomh
    oled_write_byte(0x30, OLED_CMD); //--Set VCOM Deselect Level
    oled_write_byte(0x20, OLED_CMD); //--Set Page Addressing Mode (0x00/0x01/0x02)
    oled_write_byte(0x02, OLED_CMD); //
    oled_write_byte(0x8D, OLED_CMD); //--set Charge Pump enable/disable
    oled_write_byte(0x14, OLED_CMD); //--set(0x10) disable
    oled_clear();
    oled_write_byte(0xAF, OLED_CMD);
}

