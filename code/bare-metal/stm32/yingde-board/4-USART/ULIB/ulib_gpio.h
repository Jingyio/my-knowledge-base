/******************************************************************************
 * @file
 * @brief     GPIO control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_GPIO_H__
#define __ULIB_GPIO_H__

#include "ulib_def.h"

#define CONSTRUCT_GPIO_ID(name, id) \
    static const int ULIB_GPIO_##name = (id);

CONSTRUCT_GPIO_ID(PORTA, 0)
CONSTRUCT_GPIO_ID(PORTB, 1)
CONSTRUCT_GPIO_ID(PORTC, 2)
CONSTRUCT_GPIO_ID(PORTD, 3)
CONSTRUCT_GPIO_ID(PORTE, 4)
CONSTRUCT_GPIO_ID(PORTF, 5)
CONSTRUCT_GPIO_ID(PORTG, 6)
CONSTRUCT_GPIO_ID(P0,    0)
CONSTRUCT_GPIO_ID(P1,    1)
CONSTRUCT_GPIO_ID(P2,    2)
CONSTRUCT_GPIO_ID(P3,    3)
CONSTRUCT_GPIO_ID(P4,    4)
CONSTRUCT_GPIO_ID(P5,    5)
CONSTRUCT_GPIO_ID(P6,    6)

typedef enum {
    ULIB_GPIO_DIR_IN              = 0,  //!< input mode
    ULIB_GPIO_DIR_OUT             = 1,  //!< output mode
} ulib_gpio_dir_t;

typedef enum {
    ULIB_GPIO_INPUT_MODE_FLOATING = 0,  //!< floating input
    ULIB_GPIO_INPUT_MODE_PU       = 1,  //!< pull-up input
    ULIB_GPIO_INPUT_MODE_PD       = 2,  //!< pull-down input
} ulib_gpio_input_mode_t;

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
);

/**
 * @brief  Initialize a GPIO pin as the input pin. Support input modes
           such as floating, pull-up, or pull-down.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  dir: Pin input mode. See ulib_gpio.h
 * @retval ulib_error_t: Status value. @see ulib_def.h
 */
ulib_error_t ulib_input_init(
    uint32_t                port,
    uint32_t                index,
    ulib_gpio_input_mode_t  input_mode
);

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
);

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
);

#endif
