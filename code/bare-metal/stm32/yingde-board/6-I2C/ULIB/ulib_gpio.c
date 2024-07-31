/******************************************************************************
 * @file
 * @brief     GPIO control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#include "ulib_gpio.h"

// *****************************************************************************
// Need to implement the following functions to support GPIO for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_gpio_init(
    uint32_t                port,
    uint32_t                index,
    ulib_gpio_dir_t         dir
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_gpio_input_init(
    uint32_t                port,
    uint32_t                index,
    ulib_gpio_input_mode_t  input_mode
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_gpio_set(
    uint32_t                port,
    uint32_t                index,
    uint8_t                 value
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_gpio_get(
    uint32_t                port,
    uint32_t                index,
    uint8_t*                ptr_value
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}
// *****************************************************************************
// Need to implement the above functions to support GPIO for each platform
// *****************************************************************************

/**
 * @brief  Initialize a GPIO pin, which can be configured as input or output mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  dir: Pin input/output mode, which can be ULIB_GPIO_DIR_IN or ULIB_GPIO_DIR_OUT.
 * @retval ulib_error_t: Status value. @see ulib_def.h
 */
ulib_error_t ulib_gpio_init(
    uint32_t                port,
    uint32_t                index,
    ulib_gpio_dir_t         dir
)
{
    ULIB_ASSERT(port <= ULIB_GPIO_PORTG);
    return ulib_plat_gpio_init(port, index, dir);
}

/**
 * @brief  Initialize a GPIO pin as the input pin. Support input modes
           such as floating, pull-up, or pull-down.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  dir: Pin input mode. See ulib_gpio.h
 * @retval ulib_error_t: Status value. @see ulib_def.h
 */
ulib_error_t ulib_gpio_input_init(
    uint32_t                port,
    uint32_t                index,
    ulib_gpio_input_mode_t  input_mode
)
{
    ULIB_ASSERT(port <= ULIB_GPIO_PORTG);
    return ulib_plat_gpio_input_init(port, index, input_mode);
}

/**
 * @brief  Set the status of a GPIO pin.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  value: State. Zero represents low level, the rest represents high level.
 * @retval ulib_error_t: Status value. @see ulib_def.h
 */
ulib_error_t ulib_gpio_set(
    uint32_t                port,
    uint32_t                index,
    uint8_t                 value
)
{
    ULIB_ASSERT(port <= ULIB_GPIO_PORTG);
    return ulib_plat_gpio_set(port, index, value);
}

/**
 * @brief  Get the status of a GPIO pin.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  ptr_value: Pointer to state value. Zero represents low level,
           the rest represents high level.
 * @retval ulib_error_t: Status value. @see ulib_def.h
 */
ulib_error_t ulib_gpio_get(
    uint32_t                port,
    uint32_t                index,
    uint8_t*                ptr_value
)
{
    ULIB_ASSERT(port <= ULIB_GPIO_PORTG);
    return ulib_plat_gpio_get(port, index, ptr_value);
}
