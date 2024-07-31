/******************************************************************************
 * @file      ulib_uart.h
 * @brief     UART control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/10
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#include "ulib_uart.h"

// *****************************************************************************
// Need to implement the following functions to support UART for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_uart_init(
    uint32_t                    index,
    uint32_t                    baudrate,
    ulib_uart_interrupt_mode_t  mode
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_uart_set_handler(
    uint32_t                    index,
    ulib_uart_interrupt_mode_t  mode,
    ULIB_INTERRUPT_HANDLER      handler    
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;    
}

ULIB_WEAK void ulib_plat_uart_send8(
    uint32_t                    index,
    int8_t                      data
)
{
    ULIB_ASSERT(0);
}

ULIB_WEAK int8_t ulib_plat_uart_receive8(
    uint32_t                    index
)
{
    ULIB_ASSERT(0);
    return 0; 
}
// *****************************************************************************
// Need to implement the above functions to support UART for each platform
// *****************************************************************************

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
)
{
    return ulib_plat_uart_init(index, baudrate, mode);
}

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
)
{
    ULIB_ASSERT(handler != 0);
    
    return ulib_plat_uart_set_handler(index, mode, handler);
}

/**
 * @brief  Send a byte by UART.
 * @param  index: UART index.
 * @param  data: A byte.
 */
void ulib_uart_send8(
    uint32_t                    index,
    int8_t                      data
)
{
    return ulib_plat_uart_send8(index, data);
}

/**
 * @brief  Receive a byte by UART.
 * @param  index: UART index.
 * @retval A byte data.
 */
int8_t ulib_uart_receive8(
    uint32_t                    index
)
{
    return ulib_plat_uart_receive8(index);
}
