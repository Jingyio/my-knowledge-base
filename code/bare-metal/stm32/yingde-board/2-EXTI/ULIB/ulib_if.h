#ifndef __ULIB_IF_H__
#define __ULIB_IF_H__

#include <stdint.h>

#define PORT_A      (0)
#define PORT_B      (1)
#define PORT_C      (2)
#define PORT_D      (3)
#define PORT_E      (4)
#define PORT_F      (5)
#define PORT_G      (6)
#define PIN_HIGH    (1)
#define PIN_LOW     (0)

typedef enum {
    PIN_DIR_DIGITAL_IN = 0,
    PIN_DIR_DIGITAL_OUT,
    PIN_DIR_ANALOG_IN,
    PIN_DIR_ANALIG_OUT,
} pin_direction_t;

typedef enum {
    PIN_MODE_PU,
    PIN_MODE_PD,
    PIN_MODE_FLOATING,
    PIN_MODE_OD,
    PIN_MODE_PP,
} pin_mode_t;

typedef enum {
    EXTI_TRIGGER_FALLING = 0x1,
    EXTI_TRIGGER_RISING  = 0x2,
} exit_trigger_mode_t;

typedef void (*IRQ_HANDLER)(void);

int ulib_plat_pin_init(
    uint32_t        port,
    uint32_t        index,
    pin_direction_t dir,
    pin_mode_t      mode
);

int ulib_plat_pin_set(
    uint32_t        port,
    uint32_t        index,
    uint8_t         value
);

int ulib_plat_pin_get(
    uint32_t        port,
    uint32_t        index,
    uint8_t*        ptr_value
);

int ulib_plat_exti_init(
    uint32_t            port,
    uint32_t            index,
    exit_trigger_mode_t mode
);

int ulib_plat_exti_set_handler(
    uint32_t            port,
    uint32_t            index,
    IRQ_HANDLER         handler
);


//
// Common
//
#define E_INVALID_PARAMS    (-1)
#define E_NOT_INITIALIZED   (-2)
#define E_INVALID_HANDLE    (-3)
#define E_OOM               (-4)
#define E_NULL_POINTER      (-5)

//
// GPIOs
//

static inline int ulib_pin_init(
    uint32_t        port,
    uint32_t        index,
    pin_direction_t dir,
    pin_mode_t      mode
)
{
    return ulib_plat_pin_init(port, index, dir, mode);
}

static inline int ulib_pin_set(
    uint32_t        port,
    uint32_t        index,
    uint8_t         value
)
{
    return ulib_plat_pin_set(port, index, value);
}

static inline uint8_t ulib_pin_get(
    uint32_t        port,
    uint32_t        index,
    uint8_t*        ptr_value
)
{
    return ulib_plat_pin_get(port, index, ptr_value);
}

//
// External Interrupts
//

static inline int ulib_exti_init(
    uint32_t            port,
    uint32_t            index,
    exit_trigger_mode_t mode
)
{
    return ulib_plat_exti_init(port, index, mode);
}

static inline int ulib_exti_set_handler(
    uint32_t            port,
    uint32_t            index,
    IRQ_HANDLER         handler
)
{
    return ulib_plat_exti_set_handler(port, index, handler);
}

#endif
