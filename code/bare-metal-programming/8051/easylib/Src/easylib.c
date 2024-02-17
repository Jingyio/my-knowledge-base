/**
 * Copyright 2023-2032 Lin Jingyi. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "easylib.h"

static uint32_t xdata g_interrupt_handlers[5] = { 0 };

#ifdef EASYLIB_TIMER_SUPPORT
static uint16_t xdata g_timer_mode_max_count[4] = { 0xff1f, 0xffff, 0x00ff, 0x00ff };
static uint8_t  xdata g_reload_th0 = 0;
static uint8_t  xdata g_reload_tl0 = 0;
static uint8_t  xdata g_reload_th1 = 0;
static uint8_t  xdata g_reload_tl1 = 0;
#endif

int hook_interrupt_handler(int_type_t type, void (*handler)(void))
{
    if (type > UART)
        return E_INVALID_PARAM;

    g_interrupt_handlers[type] =  handler;
    
    return 0;
}

static void execute_interrupt_group(uint8_t offset) reentrant
{
    if (g_interrupt_handlers[offset] == 0)
        return;

    ((void (*)(void))g_interrupt_handlers[offset])();
}

static void eint0_handler(void) interrupt 0
{
    execute_interrupt_group(EXTERNAL0);
}

static void tint0_handler(void) interrupt 1
{
    execute_interrupt_group(TIMER0);
    
#ifdef EASYLIB_TIMER_SUPPORT
    // If it's not automatic reloading
    if ((TMOD & 0x3) != 0x20) {
        TH0 = g_reload_th0;
        TL0 = g_reload_tl0;
    }
#endif
}

static void eint1_handler(void) interrupt 2
{
    execute_interrupt_group(EXTERNAL1);
}

static void tint1_handler(void) interrupt 3
{
    execute_interrupt_group(TIMER1);
    
#ifdef EASYLIB_TIMER_SUPPORT
    // If it's not automatic reloading
    if (((TMOD >> 4) & 0x3) != 0x20) {
        TH1 = g_reload_th1;
        TL1 = g_reload_tl1;
    }
#endif
}

static void uart_handler(void) interrupt 4
{
    execute_interrupt_group(UART);
    RI = 0;
}


#ifdef EASYLIB_TIMER_SUPPORT
int configure_timer(int_type_t timer,
                    uint8_t    mode,
                    uint8_t    th,
                    uint8_t    tl,
                    uint8_t    enable_interrupr,
                    void (*handler)(void))
{
    if (timer != TIMER0 && timer != TIMER1)
        return E_INVALID_PARAM;
    if (mode >= 4)
        return E_INVALID_PARAM;
    
    if (timer == TIMER0) {
        if (!th && !tl) {
            calculate_reload_value(g_timer_mode_max_count[mode],
                                   g_reload_th0,
                                   g_reload_tl0);
        } else {
            g_reload_th0 = th;
            g_reload_tl0 = tl;
        }
        
        TMOD &= 0xfc;
        TMOD |= mode;
        TH0 = g_reload_th0;
        TL0 = g_reload_tl0;
        if (enable_interrupr) {
            ET0 = 1;
            hook_interrupt_handler(timer, handler);
        } else {
            ET0 = 0;
        }
        
    } else {
        if (!th && !tl) {
            calculate_reload_value(g_timer_mode_max_count[mode],
                                   g_reload_th1,
                                   g_reload_tl1);
        } else {
            g_reload_th1 = th;
            g_reload_tl1 = tl;
        }
        
        TMOD &= 0xcf;
        TMOD |= (mode << 4);
        TH1 = g_reload_th1;
        TL1 = g_reload_tl1;
        if (enable_interrupr) {
            ET1 = 1;
            hook_interrupt_handler(timer, handler);
        } else {
            ET1 = 0;
        }
    }
    
    return 0;
}
#endif


#ifdef EASYLIB_UART_SUPPORT
#define MAX_BAUDRATE_PRESET (5)
#define RECEIVE_BUFFER_SIZE (32 + 1)

static baudrate_preset_t g_baudrate_presets[MAX_BAUDRATE_PRESET] = {
    { 9600 , 2, 0, 0xFD, 0xFD },
    { 9600 , 2, 1, 0xFA, 0xFA },
    { 14400, 2, 0, 0xFE, 0xFE },
    { 14400, 2, 1, 0xFC, 0xFC },
    { 57600, 2, 1, 0xFF, 0xFF },
};

static volatile uint8_t xdata g_uart_receive_flag = 0;
static volatile uint8_t xdata g_sbuf = 0;
static uint8_t xdata g_receive_buffer[RECEIVE_BUFFER_SIZE] = {0};

static void uart_2stage_handler(void)
{
    SCON &= 0xfe;   // Clear bit0(RI)
    g_uart_receive_flag = 1;
    g_sbuf = SBUF;
}

void uart_send8(uint8_t dat)
{
	SBUF = dat;
	while(!(SCON & 0x2));   // Wait bit1(TI) to become 1
	SCON &= 0xfd;           // Clear TI
}

uint8_t uart_receive8(void)
{
    while (!g_uart_receive_flag);
    g_uart_receive_flag = 0;
    
    return g_sbuf;
}

void uart_send_string(uint8_t *pStr)
{
    while (*pStr != '\0') {
        uart_send8(*pStr);
        pStr += 1;
    }
}

uint8_t *uart_receive_string(uint32_t length)
{
    uint32_t i = 0;
    
    if (length > RECEIVE_BUFFER_SIZE)
        return E_INVALID_PARAM;
    
    g_sbuf = '\0';
    g_uart_receive_flag = 0;
    
    while (length > 0) {
        g_receive_buffer[i] = uart_receive8();
        i += 1;
        length -= 1;
    }
    g_receive_buffer[i] = '\0';
    
    return g_receive_buffer;
}

int configure_uart(uint32_t baudrate)
{
    uint8_t is_found_preset = 0;
    uint8_t i = 0;
    
    for (i = 0; i < MAX_BAUDRATE_PRESET; i++) {
        if (baudrate == g_baudrate_presets[i].baudrate) {
            is_found_preset = 1;
            break;
        }
    }
    
    if (!is_found_preset)
        return E_INVALID_PARAM;
    
    // UART Mode 1, receive enabled
    SCON = 0x50;
    PCON |= g_baudrate_presets[i].double_baudrate == 0 ? 0 : 0x80;
    configure_timer(TIMER1,
                    g_baudrate_presets[i].timer_mode,
                    g_baudrate_presets[i].th,
                    g_baudrate_presets[i].tl,
                    0,
                    0);
    set_interrupt_state(TIMER1, INT_ON);
  
    hook_interrupt_handler(UART, uart_2stage_handler);
    
    return 0;
}
#endif
