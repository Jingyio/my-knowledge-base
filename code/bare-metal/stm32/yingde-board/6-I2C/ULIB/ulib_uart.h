/******************************************************************************
 * @file      ulib_uart.h
 * @brief     UART control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/10
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */
 
#ifndef __ULIB_UART_H__
#define __ULIB_UART_H__

#include "ulib_def.h"
#include <stdio.h>
 
typedef enum {
    ULIB_UART_INTERRUPT_MODE_OFF = 0,    //!< disable UART interrupt
    ULIB_UART_INTERRUPT_MODE_RX  = 1,    //!< enable RX interrupt
    ULIB_UART_INTERRUPT_MODE_TX  = 2,    //!< enable TX interrupt
} ulib_uart_interrupt_mode_t;

/**
 * @brief  Initialize UART.
 * @param  index: UART index.
 * @param  baudrate: Baud rate.
 * @param  mode: Interrupt mode.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_uart_init(
    uint32_t                    index,
    uint32_t                    baudrate,
    ulib_uart_interrupt_mode_t  mode
);

/**
 * @brief  Set an interrupt handler for UART.
 * @param  index: UART index.
 * @param  mode: Interrupt mode.
 * @param  handler: UART interrupt handler.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_uart_set_handler(
    uint32_t                    index,
    ulib_uart_interrupt_mode_t  mode,
    ULIB_INTERRUPT_HANDLER      handler    
);

/**
 * @brief  Send a byte by UART.
 * @param  index: UART index.
 * @param  data: A byte.
 */
void ulib_uart_send8(
    uint32_t                    index,
    int8_t                      data
);

/**
 * @brief  Receive a byte by UART.
 * @param  index: UART index.
 * @retval A byte data.
 */
int8_t ulib_uart_receive8(
    uint32_t                    index
);

#endif
