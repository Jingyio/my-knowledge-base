/******************************************************************************
 * @file      ulib_spi.h
 * @brief     SPI control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/22
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */
#include "ulib_spi.h"

// *****************************************************************************
// Need to implement the following functions to support SPI for each platform
// *****************************************************************************
ULIB_WEAK void ulib_plat_spi_output_init(
    uint32_t    port,
    uint32_t    pin
)
{
    ULIB_ASSERT(0);
}

ULIB_WEAK void ulib_plat_spi_input_init(
    uint32_t    port,
    uint32_t    pin
)
{
    ULIB_ASSERT(0);
}

ULIB_WEAK void ulib_plat_spi_pin_set(
    uint32_t    port,
    uint32_t    pin,
    uint8_t     value
)
{
    ULIB_ASSERT(0);
}

ULIB_WEAK uint8_t ulib_plat_spi_pin_get(
    uint32_t    port,
    uint32_t    pin
)
{
    ULIB_ASSERT(0);
    return 0;
}

ULIB_WEAK void ulib_plat_spi_delay(
    void
)
{
    ULIB_ASSERT(0);
}
// *****************************************************************************
// Need to implement the above functions to support SPI for each platform
// *****************************************************************************

static ulib_spi_config_t g_spi_configs[ULIB_SPI_MAX_NUM] = { 0 };

static inline void spi_cs_1(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    ulib_plat_spi_pin_set(g_spi_configs[index].spi_pins.cs_port,
                          g_spi_configs[index].spi_pins.cs_pin, 1);
}

static inline void spi_cs_0(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    ulib_plat_spi_pin_set(g_spi_configs[index].spi_pins.cs_port,
                          g_spi_configs[index].spi_pins.cs_pin, 0);
}

static inline void spi_sck_1(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    ulib_plat_spi_pin_set(g_spi_configs[index].spi_pins.sck_port,
                          g_spi_configs[index].spi_pins.sck_pin, 1);
}

static inline void spi_sck_0(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    ulib_plat_spi_pin_set(g_spi_configs[index].spi_pins.sck_port,
                          g_spi_configs[index].spi_pins.sck_pin, 0);
}

static inline void spi_mosi_1(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    ulib_plat_spi_pin_set(g_spi_configs[index].spi_pins.mosi_port,
                          g_spi_configs[index].spi_pins.mosi_pin, 1);
}

static inline void spi_mosi_0(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    ulib_plat_spi_pin_set(g_spi_configs[index].spi_pins.mosi_port,
                          g_spi_configs[index].spi_pins.mosi_pin, 0);
}

static inline uint8_t spi_miso_get(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    return ulib_plat_spi_pin_get(g_spi_configs[index].spi_pins.miso_port,
                                 g_spi_configs[index].spi_pins.miso_pin);
}

static void spi_memcpy(void* src, void* dest, uint32_t count)
{
    uint32_t i = 0;
    
    ULIB_ASSERT(src != 0);
    ULIB_ASSERT(dest != 0);
    
    for (i = 0; i < count; i++) {
        *((uint8_t*)dest + i) = *((uint8_t *)src + i);
    }
}

/**
 * @brief  Initialize SPI.
 * @param  spi_index: SPI index.
 * @param  config: SPI config.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_spi_init(
    uint32_t            index,
    ulib_spi_config_t   config
)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    if (index >= ULIB_SPI_MAX_NUM)
        return ULIB_E_INVALID_PARAMS;
    
    spi_memcpy(&config, &g_spi_configs[index], sizeof(ulib_spi_config_t));

    ulib_plat_spi_output_init(g_spi_configs[index].spi_pins.cs_port,
                              g_spi_configs[index].spi_pins.cs_pin);
    ulib_plat_spi_output_init(g_spi_configs[index].spi_pins.sck_port,
                              g_spi_configs[index].spi_pins.sck_pin);
    ulib_plat_spi_output_init(g_spi_configs[index].spi_pins.mosi_port,
                              g_spi_configs[index].spi_pins.mosi_pin);
    ulib_plat_spi_input_init(g_spi_configs[index].spi_pins.miso_port,
                             g_spi_configs[index].spi_pins.miso_pin);
    
    return ULIB_S_OK;
}

/**
 * @brief  Send a byte and receive a byte.
 * @param  spi_index: SPI index.
 * @param  value: Low or high level.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_spi_select(
    uint32_t            index,
    uint8_t             value
)
{
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    if (index >= ULIB_SPI_MAX_NUM)
        return ULIB_E_INVALID_PARAMS;
    
    if (value)
        spi_cs_1(index);
    else
        spi_cs_0(index);
    
    return ULIB_S_OK;
}

/**
 * @brief  Send a byte and receive a byte.
 * @param  spi_index: SPI index.
 * @param  value: Data to send.
 * @retval uint8_t: Data to receive.
 */
uint8_t ulib_spi_readwrite(
    uint32_t            index,
    uint8_t             value
)
{
    uint8_t mode = 0;
    uint8_t i = 0;
    uint8_t read_value = 0;
    
    ULIB_ASSERT(index < ULIB_SPI_MAX_NUM);
    if (index >= ULIB_SPI_MAX_NUM)
        return 0;
    
    mode = (g_spi_configs[index].cpol & 0x1) | g_spi_configs[index].cpha;
    switch (mode) {
    case 0:
        for (i = 0; i < 8; i++) {
            if (value & 0x80)
                spi_mosi_1(index);
            else
                spi_mosi_0(index);
            value <<= 1;
            ulib_plat_spi_delay();
            spi_sck_1(index);
            
            read_value <<= 1;
            if (spi_miso_get(index))
                read_value += 1;
            ulib_plat_spi_delay();
            spi_sck_0(index);
        }
        break;
    case 1:
        for (i = 0; i < 8; i++) {
            spi_sck_1(index);
            if (value & 0x80)
                spi_mosi_1(index);
            else
                spi_mosi_0(index);
            value <<= 1;
            ulib_plat_spi_delay();
            
            spi_sck_0(index);
            read_value <<= 1;
            if (spi_miso_get(index))
                read_value += 1;
            ulib_plat_spi_delay();
        }
        break;
    case 2:
        for (i = 0; i < 8; i++) {
            if (value & 0x80)
                spi_mosi_1(index);
            else
                spi_mosi_0(index);
            value <<= 1;
            ulib_plat_spi_delay();
            spi_sck_0(index);
            
            read_value <<= 1;
            if (spi_miso_get(index))
                read_value += 1;
            spi_sck_1(index);            
        }
        break;
    case 3:
        for (i = 0; i < 8; i++) {
            spi_sck_0(index);
            if (value & 0x80)
                spi_mosi_1(index);
            else
                spi_mosi_0(index);
            value <<= 1;
            ulib_plat_spi_delay();
            
            spi_sck_1(index);
            read_value <<= 1;
            if (spi_miso_get(index))
                read_value += 1;
            ulib_plat_spi_delay();
        }
        break;
    default:
        break;
    }
    
    return read_value;
}

