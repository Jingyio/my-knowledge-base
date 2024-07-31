/******************************************************************************
 * @file      ulib_assert.h
 * @brief     Assert control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/07/28
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_ASSERT_H__
#define __ULIB_ASSERT_H__

#include <stdio.h>

/**
 * @brief  Initialize assertion environment.
 */
void ulib_plat_assert_init(void);

#ifndef ULIB_PLAT_ASSERT_DUMP
#ifdef __MICROLIB
#define ULIB_PLAT_ASSERT_DUMP(exp, ...) printf(exp, ##__VA_ARGS__)
#else
#define ULIB_PLAT_ASSERT_DUMP(exp, ...)
#endif
#endif

#define ULIB_ASSERT_FAIL(exp, file, line)                                   \
    do {                                                                    \
        static int initialized = 0;                                         \
        if (!initialized) {                                                 \
            ulib_plat_assert_init();                                        \
            initialized = 1;                                                \
        }                                                                   \
        ULIB_PLAT_ASSERT_DUMP("[FILE] %s [LINE] %d [ASSERT_FAIL] %s\n",     \
            file, line, exp);                                               \
    } while (0)

#define ULIB_ASSERT(exp)                                                    \
    do {                                                                    \
        if (!(exp))                                                         \
           ULIB_ASSERT_FAIL(#exp, __FILE__, __LINE__);                      \
    } while (0)
#endif
