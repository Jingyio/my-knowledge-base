/******************************************************************************
 * @file      ulib_spi.h
 * @brief     SPI control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/07/09
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_ADC_DAC_H__
#define __ULIB_ADC_DAC_H__

#include "ulib_def.h"

/**
 * @brief  Initialize a pin as ADC mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_adc_init(
    uint32_t            port,
    uint32_t            index
);

/**
 * @brief  Read the voltage of a certain pin
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @retval float: Voltage value.
 */
float ulib_adc_get(
    uint32_t            port,
    uint32_t            index
);

/**
 * @brief  Initialize a pin as DAC mode.
 * @param  port: Port index, which can be from ULIB_GPIO_PORTA to ULIB_GPIO_PORTG.
 * @param  index: Pin index.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_dac_init(
    uint32_t            port,
    uint32_t            index
);

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
);

#endif
