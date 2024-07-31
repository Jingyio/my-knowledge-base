/******************************************************************************
 * @file      ulib_i2c.h
 * @brief     I2C control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/10
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */

#ifndef __ULIB_I2C_H__
#define __ULIB_I2C_H__

#include "ulib_def.h"

#define ULIB_I2C_SW_MAX (4)

typedef enum {
    ULIB_I2C_MODE_SW = 0,
    ULIB_I2C_MODE_HW = 1,
} ulib_i2c_mode_t;

typedef struct {
    uint32_t        scl_port;
    uint32_t        scl_pin;
    uint32_t        sda_port;
    uint32_t        sda_pin;
} ulib_i2c_pins_t;

typedef struct {
    uint32_t        i2c_index;  //!< I2C index. The index of SW and HW groups are independent.
    ulib_i2c_mode_t i2c_mode;   //!< SW or HW mode
    ulib_i2c_pins_t i2c_pins;    //!< only used in SW mode
} ulib_i2c_config_t;

/**
 * @brief  Initialize I2C.
 * @param  config: I2C config.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_i2c_init(
    ulib_i2c_config_t   config
);

/**
 * @brief  Trigger an I2C start signal.
 * @param  index: I2C index.
 */
void ulib_i2c_start(
    uint32_t            index
);

/**
 * @brief  Trigger an I2C stop signal.
 * @param  index: I2C index.
 */
void ulib_i2c_stop(
    uint32_t            index
);

/**
 * @brief  Trigger an I2C wait ack signal.
 * @param  index: I2C index.
 */
void ulib_i2c_wait_ack(
    uint32_t            index
);

/**
 * @brief  Trigger an I2C ack signal.
 * @param  index: I2C index.
 */
void ulib_i2c_ack(
    uint32_t            index
);

/**
 * @brief  Trigger an I2C not ack signal.
 * @param  index: I2C index.
 */
void ulib_i2c_nack(
    uint32_t            index
);

/**
 * @brief  Send a byte data by I2C.
 * @param  index: I2C index.
 * @param  data: A byte data.
 */
void ulib_i2c_write(
    uint32_t            index,
    uint8_t             data
);

/**
 * @brief  Receive a byte data by I2C.
 * @param  index: I2C index.
 * @retval data: A byte data.
 */
uint8_t ulib_i2c_read(
    uint32_t            index
);

#endif
