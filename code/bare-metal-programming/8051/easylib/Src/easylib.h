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

#ifndef __EASYLIB_H_
#define __EASYLIB_H_

#include <reg52.h>

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

#define E_INVALID_PARAM             (-1)
#define E_OUT_OF_MEMORY             (-2)

//
// Common
//
typedef enum interrupt_type {
    TIMER0    = 0x0,
    TIMER1    = 0x1,
    EXTERNAL0 = 0x2,
    EXTERNAL1 = 0x3,
    UART      = 0x4,
} int_type_t;

typedef enum interrupt_state {
    INT_OFF = 0,
    INT_ON  = 1,
} int_state_t;

#define EXTERNAL0_INT_OFF           (EX0=0)
#define EXTERNAL0_INT_ON            (EX0=1)
#define TIMER0_INT_OFF              (TR0=0)
#define TIMER0_INT_ON               (TR0=1)
#define EXTERNAL1_INT_OFF           (EX1=0)
#define EXTERNAL1_INT_ON            (EX1=1)
#define TIMER1_INT_OFF              (TR1=0)
#define TIMER1_INT_ON               (TR1=1)
#define UART_INT_OFF                (ES=0)
#define UART_INT_ON                 (ES=1)

#define set_interrupt_state(t, s)           \
    do {                                    \
        t##_##s;                            \
        EA = EX0 | ET0 | EX1 | ET1 | ES;    \
    } while (0)
    
int hook_interrupt_handler(int_type_t type, void (*handler)(void));

//
// External Interrupt
//
#ifdef EASYLIB_EXTERNAL_INTERRUPT_SUPPORT
typedef enum eint_trigger_mode {
    TRIGGER_LEVEL = 0,
    TRIGGER_FALLING = 1
} eint_trigger_mode_t;

#define EXTERNAL0_TRIGGER_LEVEL     (IT0=0)
#define EXTERNAL0_TRIGGER_FALLING   (IT0=1)
#define EXTERNAL1_TRIGGER_LEVEL     (IT1=0)
#define EXTERNAL1_TRIGGER_FALLING   (IT1=1)

#define configure_exint(t, s, h)            \
    do {                                    \
        t##_##s;                            \
        hook_interrupt_handler(t, h);       \
    } while (0)
#endif

//
// Timer Interrupt
//
#ifdef EASYLIB_TIMER_SUPPORT
typedef enum timer_int_mode {
    TINT_MODE_0 = 0xff1f,   // Max Period ~= 0xff1f * 1 us
    TINT_MODE_1 = 0xffff,   // Max Period ~= 0xffff * 1 us
    TINT_MODE_2 = 0x00ff,   // Max Period ~= 0x00ff * 1 us
    TINT_MODE_3 = 0x00ff,   // Max Period ~= 0x00ff * 1 us
} timer_int_mode_t;
    
#define CRYSTAL_FREQUENCY_HZ        (11059200)      // set by user
#define DEFAULT_TRIGGER_PERIOD_US   (50000)         // set by user
#define MACHINE_PERIOD_US           ((12 * 1000000) / CRYSTAL_FREQUENCY_HZ)

#define calculate_reload_value(m, th, tl)                                                                       \
    do {                                                                                                        \
        (tl) = ((((uint32_t)m) + 1) - (DEFAULT_TRIGGER_PERIOD_US / MACHINE_PERIOD_US)) & (m) & 0xff;            \
        (th) = ((((((uint32_t)m) + 1) - (DEFAULT_TRIGGER_PERIOD_US / MACHINE_PERIOD_US)) & (m)) >> 8) & 0xff;   \
    } while (0)

int configure_timer(int_type_t, uint8_t, uint8_t, uint8_t, uint8_t, void (*)(void));

#endif

//
// UART
//
#ifdef EASYLIB_UART_SUPPORT
typedef struct baudrate_preset {
    uint32_t baudrate;
    uint8_t  timer_mode;
    uint8_t  double_baudrate;
    uint8_t  th;
    uint8_t  tl;
} baudrate_preset_t;

int      configure_uart     (uint32_t);
void     uart_send8         (uint8_t);
uint8_t  uart_receive8      (void);
void     uart_send_string   (uint8_t*);
uint8_t* uart_receive_string(uint32_t);
#endif


#endif