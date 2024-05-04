#ifndef __ULIB_IF_H__
#define __ULIB_IF_H__

#include <stdint.h>

#define PORT_A   (0)
#define PORT_B   (1)
#define PORT_C   (2)
#define PORT_D   (3)
#define PORT_E   (4)
#define PORT_F   (5)
#define PORT_G   (6)
#define PIN_HIGH (1)
#define PIN_LOW  (0)

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

int ulib_plat_pin_init(
    uint32_t        port,
    uint32_t        index,
    pin_direction_t dir,
    pin_mode_t      mode
);
static inline int ulib_pin_init(
    uint32_t        port,
    uint32_t        index,
    pin_direction_t dir,
    pin_mode_t      mode
)
{
    return ulib_plat_pin_init(port, index, dir, mode);
}

void ulib_plat_pin_set(
    uint32_t        port,
    uint32_t        index,
    uint8_t         value
);

static inline void ulib_pin_set(
    uint32_t        port,
    uint32_t        index,
    uint8_t         value
)
{
    ulib_plat_pin_set(port, index, value);
}

uint8_t ulib_plat_pin_get(
    uint32_t        port,
    uint32_t        index
);

static inline uint8_t ulib_pin_get(
    uint32_t        port,
    uint32_t        index
)
{
    return ulib_plat_pin_get(port, index);
}

#endif
