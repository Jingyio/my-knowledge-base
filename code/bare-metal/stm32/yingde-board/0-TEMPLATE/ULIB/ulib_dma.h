/******************************************************************************
 * @file      ulib_i2c.h
 * @brief     DMA control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/10
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_DMA_H__
#define __ULIB_DMA_H__

#include "ulib_def.h"

/**
 * @brief  Memory copy using DMA.
 * @param  channel: DMA channel.
 * @param  ptr_src: address of source.
 * @param  ptr_dest: address of destination.
 * @param  size_in_bytes: data count.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_dma_memcpy(
    uint32_t    channel,
    uint8_t*    ptr_src,
    uint8_t*    ptr_dest,
    uint32_t    size_in_bytes
);

/**
 * @brief  Wait until DMA operation done.
 * @param  channel: DMA channel.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_dma_wait4done(
    uint32_t    channel
);

#endif
