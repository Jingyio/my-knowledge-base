/******************************************************************************
 * @file
 * @brief     External interrupt control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#include "ulib_exti.h"

// *****************************************************************************
// Need to implement the following functions to support EXTI for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_exti_init(
    uint32_t                port,
    uint32_t                pin,
    ulib_exti_trigger_t     mode
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_exti_set_handler(
    uint32_t                port,
    uint32_t                pin,
    ULIB_INTERRUPT_HANDLER  handler
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}
// *****************************************************************************
// Need to implement the above functions to support EXTI for each platform
// *****************************************************************************

/**
 * @brief  Initialize a pin and set it to external interrupt mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  pin: Pin index.
 * @param  mode: Trigger mode.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_exti_init(
    uint32_t                port,
    uint32_t                pin,
    ulib_exti_trigger_t     mode
)
{
    ULIB_ASSERT(port <= ULIB_GPIO_PORTG);
    return ulib_plat_exti_init(port, pin, mode);
}

/**
 * @brief  Set an external interrupt handler for the pin.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  pin: Pin index.
 * @param  handler: External interrupt handler.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_exti_set_handler(
    uint32_t                port,
    uint32_t                pin,
    ULIB_INTERRUPT_HANDLER  handler
)
{
    ULIB_ASSERT(port <= ULIB_GPIO_PORTG);
    ULIB_ASSERT(handler != 0);
    return ulib_plat_exti_set_handler(port, pin, handler);
}
