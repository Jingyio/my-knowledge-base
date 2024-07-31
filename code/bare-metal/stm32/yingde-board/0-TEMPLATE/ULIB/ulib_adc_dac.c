/******************************************************************************
 * @file      ulib_spi.h
 * @brief     SPI control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/22
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */
#include "ulib_adc_dac.h"

// *****************************************************************************
// Need to implement the following functions to support ADC/DAC for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_adc_init(
    uint32_t            port,
    uint32_t            index
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK float ulib_plat_adc_get(
    uint32_t            port,
    uint32_t            index
)
{
    ULIB_ASSERT(0);
    return 0;
}

ULIB_WEAK ulib_error_t ulib_plat_dac_init(
    uint32_t            port,
    uint32_t            index
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK void ulib_plat_dac_set(
    uint32_t            port,
    uint32_t            index,
    float               value
)
{
    ULIB_ASSERT(0);
}
// *****************************************************************************
// Need to implement the above functions to support ADC/DAC for each platform
// *****************************************************************************

/**
 * @brief  Initialize a pin as ADC mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_adc_init(
    uint32_t            port,
    uint32_t            index
)
{
    return ulib_plat_adc_init(port, index);
}

/**
 * @brief  Read the voltage of a certain pin
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @retval float: Voltage value.
 */
float ulib_adc_get(
    uint32_t            port,
    uint32_t            index
)
{
    return ulib_plat_adc_get(port, index);
}

/**
 * @brief  Initialize a pin as DAC mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_dac_init(
    uint32_t            port,
    uint32_t            index
)
{
    return ulib_plat_dac_init(port, index);
}

/**
 * @brief  Write the voltage of a certain pin
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @param  float: Voltage value.
 */
void ulib_dac_set(
    uint32_t            port,
    uint32_t            index,
    float               value
)
{
    ulib_plat_dac_set(port, index, value);
}
