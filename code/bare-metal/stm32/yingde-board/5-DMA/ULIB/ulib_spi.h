/******************************************************************************
 * @file      ulib_spi.h
 * @brief     SPI control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/22
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_SPI_H__
#define __ULIB_SPI_H__

#include "ulib_def.h"

#define ULIB_SPI_MAX_NUM    (3)

typedef struct {
    uint32_t        cs_port;
    uint32_t        cs_pin;
    uint32_t        sck_port;
    uint32_t        sck_pin;
    uint32_t        mosi_port;
    uint32_t        mosi_pin;
    uint32_t        miso_port;
    uint32_t        miso_pin;    
} ulib_spi_pins_t;

typedef struct {
    uint8_t         cpol;   // [CPOL:CPHA]
    uint8_t         cpha;
    ulib_spi_pins_t spi_pins;
} ulib_spi_config_t;

/**
 * @brief  Initialize SPI.
 * @param  spi_index: SPI index.
 * @param  config: SPI config.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_spi_init(
    uint32_t            index,
    ulib_spi_config_t   config
);

/**
 * @brief  Send a byte and receive a byte.
 * @param  spi_index: SPI index.
 * @param  value: Low or high level.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_spi_select(
    uint32_t            index,
    uint8_t             value
);

/**
 * @brief  Send a byte and receive a byte.
 * @param  spi_index: SPI index.
 * @param  value: Data to send.
 * @retval uint8_t: Data to receive.
 */
uint8_t ulib_spi_readwrite(
    uint32_t            index,
    uint8_t             value
);



#endif
