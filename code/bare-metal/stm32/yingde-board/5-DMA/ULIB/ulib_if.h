#ifndef __ULIB_IF_H__
#define __ULIB_IF_H__

#include "ulib_assert.h"

#define ULIB_GPIO_SUPPORT       (0)
#define ULIB_EXTI_SUPPORT       (0)
#define ULIB_UART_SUPPORT       (1)
#define ULIB_DMA_SUPPORT        (1)
#define ULIB_I2C_SUPPORT        (0)
#define ULIB_SPI_SUPPORT        (0)
#define ULIB_TIMER_SUPPORT      (0)
#define ULIB_ADC_DAC_SUPPORT    (0)

#if ULIB_EXTI_SUPPORT
#undef ULIB_GPIO_SUPPORT
#define ULIB_GPIO_SUPPORT       (1)
#endif

#if ULIB_UART_SUPPORT
#undef ULIB_GPIO_SUPPORT
#define ULIB_GPIO_SUPPORT       (1)
#endif

#if ULIB_I2C_SUPPORT
#undef ULIB_GPIO_SUPPORT
#define ULIB_GPIO_SUPPORT       (1)
#endif

#if ULIB_SPI_SUPPORT
#undef ULIB_GPIO_SUPPORT
#define ULIB_GPIO_SUPPORT       (1)
#endif

#if ULIB_GPIO_SUPPORT
#include "ulib_gpio.h"
#endif

#if ULIB_EXTI_SUPPORT
#include "ulib_exti.h"
#endif

#if ULIB_UART_SUPPORT
#include "ulib_uart.h"
#endif

#if ULIB_DMA_SUPPORT
#include "ulib_dma.h"
#endif

#if ULIB_I2C_SUPPORT
#include "ulib_i2c.h"
#endif

#if ULIB_SPI_SUPPORT
#include "ulib_spi.h"
#endif

#if ULIB_TIMER_SUPPORT
#include "ulib_timer.h"
#endif

#if ULIB_ADC_DAC_SUPPORT
#include "ulib_adc_dac.h"
#endif

#endif
