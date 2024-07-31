/******************************************************************************
 * @file
 * @brief     External interrupt control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */
 
 #ifndef __ULIB_EXTI_H__
 #define __ULIB_EXTI_H__
 
 #include "ulib_def.h"
 #include "ulib_gpio.h"
 
 typedef enum {
    ULIB_EXTI_TRIGGER_FALLING = 0x1,    //!< falling edge trigger
    ULIB_EXTI_TRIGGER_RISING  = 0x2,    //!< raising edge trigger
} ulib_exti_trigger_t;

/**
 * @brief  Initialize a pin and set it to external interrupt mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  mode: Trigger mode.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_exti_init(
    uint32_t            port,
    uint32_t            index,
    ulib_exti_trigger_t mode
);

/**
 * @brief  Set an external interrupt handler for the pin.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  handler: External interrupt handler.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_exti_set_handler(
    uint32_t                port,
    uint32_t                index,
    ULIB_INTERRUPT_HANDLER  handler
);
 
#endif
