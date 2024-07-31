/******************************************************************************
 * @file      ulib_timer.c
 * @brief     Timer control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/19
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#include "ulib_timer.h"

// *****************************************************************************
// Need to implement the following functions to support timer for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_timer_start(
    uint32_t                index,
    uint64_t                period_in_us,
    ULIB_INTERRUPT_HANDLER  callback
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_timer_stop(
    uint32_t                index
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK void ulib_plat_delay_us(
    uint32_t                us
)
{
    ULIB_ASSERT(0);
}
// *****************************************************************************
// Need to implement the above functions to support timer for each platform
// *****************************************************************************

/**
 * @brief  Initialize a timer. Automatically trigger callback function after
           a certain period of time.
 * @param  index: Timer index.
 * @param  period_in_us: Period, in microseconds.
 * @param  callback: User callback function.
 * @param  cb_mode: Callbacks will be triggered once or multiple times.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_timer_start(
    uint32_t                index,
    uint64_t                period_in_us,
    ULIB_INTERRUPT_HANDLER  callback
) 
{
    ULIB_ASSERT(callback != 0);
    return ulib_plat_timer_start(index, period_in_us, callback);
}

/**
 * @brief  Stop and uninitialize a timer.
 * @param  index: Timer index.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_timer_stop(
    uint32_t             index
)
{
    return ulib_plat_timer_stop(index);
}

/**
 * @brief  Simple delay
 * @param  us: Delay time in microseconds.
 */
void ulib_delay_us(
    uint32_t                us
)
{
    return ulib_plat_delay_us(us);
}
