/******************************************************************************
 * @file      ulib_dma.c
 * @brief     DMA control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#include "ulib_dma.h"

// *****************************************************************************
// Need to implement the following functions for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_dma_memcpy(
    uint32_t    channel,
    uint8_t*    ptr_src,
    uint8_t*    ptr_dest,
    uint32_t    size_in_bytes
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_dma_wait4done(
    uint32_t    channel
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}
// *****************************************************************************
// Need to implement the above functions for each platform
// *****************************************************************************

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
)
{
    ULIB_ASSERT(ptr_src != 0);
    ULIB_ASSERT(ptr_dest != 0);
    
    return ulib_plat_dma_memcpy(channel, ptr_src, ptr_dest, size_in_bytes);
}

/**
 * @brief  Wait until DMA operation done.
 * @param  channel: DMA channel.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_dma_wait4done(
    uint32_t    channel
)
{
    return ulib_plat_dma_wait4done(channel);
}
