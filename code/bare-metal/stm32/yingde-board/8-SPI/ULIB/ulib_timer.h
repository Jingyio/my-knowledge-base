/******************************************************************************
 * @file      ulib_timer.h
 * @brief     Timer control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/19
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_TIMER_H__
#define __ULIB_TIMER_H__

#include "ulib_def.h"

/**
 * @brief  Initialize a timer and start it. Automatically trigger callback
           function after a certain period of time.
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
);

/**
 * @brief  Stop and uninitialize a timer.
 * @param  index: Timer index.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_timer_stop(
    uint32_t                index
);

/**
 * @brief  Simple delay
 * @param  us: Delay time in microseconds.
 */
void ulib_delay_us(
    uint32_t                us
);

#endif
