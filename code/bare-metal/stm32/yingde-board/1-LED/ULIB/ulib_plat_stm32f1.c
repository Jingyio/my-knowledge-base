#include "ulib_if.h"
#include "stm32f10x.h"

int ulib_plat_pin_init(
    uint32_t        port,
    uint32_t        index,
    pin_direction_t dir,
    pin_mode_t      mode
)
{
    GPIO_InitTypeDef init_struct = {0};
    uint32_t port_clock_addr = 1;
    uint32_t port_addr = 0x40010800;
    GPIOMode_TypeDef gpio_mode = GPIO_Mode_IN_FLOATING;
    int i = 0;   
    
    for (i = 0; i < port + 2; i++)
        port_clock_addr *= 2;
    for (i = 0; i < port; i++)
        port_addr += 0x400;
    
    RCC_APB2PeriphClockCmd(port_clock_addr, ENABLE);
    
    if (dir == PIN_DIR_ANALOG_IN)
        gpio_mode = GPIO_Mode_AIN;
    else if (dir == PIN_DIR_DIGITAL_IN && mode == PIN_MODE_FLOATING)
        gpio_mode = GPIO_Mode_IN_FLOATING;
    else if (dir == PIN_DIR_DIGITAL_IN && mode == PIN_MODE_PU)
        gpio_mode = GPIO_Mode_IPU;
    else if (dir == PIN_DIR_DIGITAL_IN && mode == PIN_MODE_PD)
        gpio_mode = GPIO_Mode_IPD;    
    else if (dir == PIN_DIR_DIGITAL_OUT && mode == PIN_MODE_OD)
        gpio_mode = GPIO_Mode_Out_OD;    
    else if (dir == PIN_DIR_DIGITAL_OUT && mode == PIN_MODE_PP)
        gpio_mode = GPIO_Mode_Out_PP;
    
    init_struct.GPIO_Mode  = gpio_mode;
    init_struct.GPIO_Pin   = (uint16_t)(1 << index);
    init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init((GPIO_TypeDef*)port_addr, &init_struct);
    
    return 0;
}

void ulib_plat_pin_set(
    uint32_t        port,
    uint32_t        index,
    uint8_t         value
)
{
    uint32_t port_addr = 0x40010800;
    int i = 0;   
    
    for (i = 0; i < port; i++)
        port_addr += 0x400;
    
    if (value)
        GPIO_SetBits((GPIO_TypeDef*)port_addr, (uint16_t)(1 << index));
    else
        GPIO_ResetBits((GPIO_TypeDef*)port_addr, (uint16_t)(1 << index));   
}

uint8_t ulib_plat_pin_get(
    uint32_t        port,
    uint32_t        index
)
{
    uint32_t port_addr = 0x40010800;
    int i = 0;   
    
    for (i = 0; i < port; i++)
        port_addr += 0x400;
    
    return GPIO_ReadInputDataBit((GPIO_TypeDef*)port_addr, (uint16_t)(1 << index));   
}



