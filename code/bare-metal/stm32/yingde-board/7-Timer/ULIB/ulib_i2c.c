/******************************************************************************
 * @file      ulib_i2c.c
 * @brief     I2C control
 * @author    jingyi.lin at Donghaihuiyuan
 * @date      2024/06/10
 * @version   v1.0
 * @copyright TWOB corporation
 ******************************************************************************
 */
 
#include "ulib_i2c.h"

// *****************************************************************************
// Need to implement the following functions for each platform
// *****************************************************************************
ULIB_WEAK ulib_error_t ulib_plat_i2c_scl_init(
    uint32_t            scl_port,
    uint32_t            scl_pin
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_i2c_sda_init(
    uint32_t            sda_port,
    uint32_t            sda_pin
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK ulib_error_t ulib_plat_i2c_pin_set(
    uint32_t            port,
    uint32_t            pin,
    uint8_t             value
)
{
    ULIB_ASSERT(0);
    return ULIB_E_NOT_INITIALIZED;
}

ULIB_WEAK uint8_t ulib_plat_i2c_pin_get(
    uint32_t            port,
    uint32_t            pin
)
{
    ULIB_ASSERT(0);
    return 0;
}

ULIB_WEAK void ulib_plat_i2c_delay(
    void
)
{
    ULIB_ASSERT(0);
}
// *****************************************************************************
// Need to implement the above functions for each platform
// *****************************************************************************

static ulib_i2c_pins_t g_i2c_pins[ULIB_I2C_SW_MAX] = {0};

static inline void i2c_scl_1(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    ulib_plat_i2c_pin_set(g_i2c_pins[index].scl_port,
                          g_i2c_pins[index].scl_pin, 1);
}

static inline void i2c_scl_0(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    ulib_plat_i2c_pin_set(g_i2c_pins[index].scl_port,
                          g_i2c_pins[index].scl_pin, 0);
}

static inline void i2c_sda_1(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    ulib_plat_i2c_pin_set(g_i2c_pins[index].sda_port,
                          g_i2c_pins[index].sda_pin, 1);
}

static inline void i2c_sda_0(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    ulib_plat_i2c_pin_set(g_i2c_pins[index].sda_port,
                          g_i2c_pins[index].sda_pin, 0);
}

static inline uint8_t i2c_sda_get(uint8_t index)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    return ulib_plat_i2c_pin_get(g_i2c_pins[index].sda_port,
                                 g_i2c_pins[index].sda_pin);
}

static inline void i2c_delay(void)
{
    ulib_plat_i2c_delay();
}

/**
 * @brief  Initialize I2C.
 * @param  config: I2C config.
 * @retval ulib_error_t: Status value.
 */
ulib_error_t ulib_i2c_init(
    ulib_i2c_config_t   config
)
{
    if (config.i2c_mode == ULIB_I2C_MODE_SW) {
        ULIB_ASSERT(config.i2c_index < ULIB_I2C_SW_MAX);
        if (config.i2c_index < ULIB_I2C_SW_MAX) {
            g_i2c_pins[config.i2c_index].scl_port = config.i2c_pins.scl_port;
            g_i2c_pins[config.i2c_index].sda_port = config.i2c_pins.sda_port;
            g_i2c_pins[config.i2c_index].scl_pin = config.i2c_pins.scl_pin;
            g_i2c_pins[config.i2c_index].sda_pin = config.i2c_pins.sda_pin;
        }

        ulib_plat_i2c_scl_init(config.i2c_pins.scl_port,
                               config.i2c_pins.scl_pin);
        ulib_plat_i2c_sda_init(config.i2c_pins.sda_port,
                               config.i2c_pins.sda_pin);
    } else if (config.i2c_mode == ULIB_I2C_MODE_HW) {
        // TODO: Implement with hardware
    }
    
    return ULIB_S_OK;
}

/**
 * @brief  Trigger an I2C start signal.
 * @param  index: I2C index.
 */
void ulib_i2c_start(
    uint32_t            index
)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return;
    
    i2c_sda_1(index);
    i2c_scl_1(index);
    i2c_delay();
    i2c_sda_0(index);
    i2c_delay();
    i2c_scl_0(index);
    i2c_delay();
}

/**
 * @brief  Trigger an I2C stop signal.
 * @param  index: I2C index.
 */
void ulib_i2c_stop(
    uint32_t            index
)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return;
    
    i2c_sda_0(index);
    i2c_delay();
    i2c_scl_1(index);
    i2c_delay();
    i2c_sda_1(index);
    i2c_delay();    
}

/**
 * @brief  Trigger an I2C wait ack signal.
 * @param  index: I2C index.
 */
void ulib_i2c_wait_ack(
    uint32_t            index
)
{
    uint16_t i = 0;
    
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return;
    
    i2c_sda_1(index);
    i2c_scl_1(index);
    i2c_delay();
    
    while (i < 256) {
        if (!i2c_sda_get(index))
            break;
        i++;
    }
    
    i2c_scl_0(index);
    i2c_delay();
}

/**
 * @brief  Trigger an I2C ack signal.
 * @param  index: I2C index.
 */
void ulib_i2c_ack(
    uint32_t            index
)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return;
    
	i2c_sda_0(index);
	i2c_delay();
    i2c_scl_1(index);
	i2c_delay();
    i2c_scl_0(index);
	i2c_delay();
	i2c_sda_1(index);    
}

/**
 * @brief  Trigger an I2C not ack signal.
 * @param  index: I2C index.
 */
void ulib_i2c_nack(
    uint32_t            index
)
{
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return;
    
	i2c_sda_1(index);
	i2c_delay();
    i2c_scl_1(index);
	i2c_delay();
    i2c_scl_0(index);
	i2c_delay();
}

/**
 * @brief  Send a byte data by I2C.
 * @param  index: I2C index.
 * @param  data: A byte data.
 */
void ulib_i2c_write(
    uint32_t            index,
    uint8_t             data
)
{
    uint8_t i = 0;
    
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return;
    
    for (i = 0; i < 8; i++) {
        if (((data & 0x80) >> 7))
            i2c_sda_1(index);
        else
            i2c_sda_0(index);
        
        i2c_delay();                
        i2c_scl_1(index);
        i2c_delay();
        i2c_scl_0(index);         
        i2c_delay();
        
        if (i == 7)
            i2c_sda_1(index);
        
        data <<= 1;
        i2c_delay();
    }
}

/**
 * @brief  Receive a byte data by I2C.
 * @param  index: I2C index.
 * @retval data: A byte data.
 */
uint8_t ulib_i2c_read(
    uint32_t            index
)
{
    uint8_t i = 0;
    uint8_t data = 0;
    
    ULIB_ASSERT(index < ULIB_I2C_SW_MAX);
    if (index >= ULIB_I2C_SW_MAX)
        return 0;
        
    i2c_scl_0(index);
    
    for (i = 0; i < 8; i++) {
        i2c_scl_1(index);            
        i2c_delay();
        
        data = (data << 1) | i2c_sda_get(index);
        
        i2c_delay();
        i2c_scl_0(index);
        i2c_delay();
    }
    
    return data;    
}
