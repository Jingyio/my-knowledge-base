/******************************************************************************
 * @file      ulib_def.h
 * @brief     Common definition declarations of ulib
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_DEF_H__
#define __ULIB_DEF_H__

#include <stdint.h>
#include "ulib_assert.h"

#if defined(__CC_ARM)
#define ULIB_WEAK   __weak
#else
#error Tool chain not supported
#endif

/**
 * @brief  The status values of ulib. ULIB_S_OK(Success), otherwise(Fail).
 */
typedef enum {
    ULIB_S_OK              =  0x0,
    ULIB_E_INVALID_PARAMS  = -0x1,
    ULIB_E_NOT_INITIALIZED = -0x2,
    ULIB_E_INVALID_HANDLE  = -0x3,
    ULIB_E_OOM             = -0x4,
    ULIB_E_NULL_POINTER    = -0x5,
} ulib_error_t;


/**
 * @brief  Interrupt handling function type. Users may need to implement such functions.
 */
typedef void (*ULIB_INTERRUPT_HANDLER)(void);


#endif

