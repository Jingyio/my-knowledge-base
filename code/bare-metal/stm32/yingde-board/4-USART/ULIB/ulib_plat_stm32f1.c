#include "ulib_if.h"
#include "stm32f10x.h"

//
// Common
//

volatile static uint32_t gDelayCount;

void systick_init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while(1);
    }
    
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
    if (gDelayCount > 0)
        gDelayCount--;
}


void ulib_plat_delay_ms(
    uint32_t ms
)
{
    static int has_init = 0;
    
    if (!has_init) {
        systick_init();
        has_init = 1;
    }
    
    gDelayCount = ms;

    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while(gDelayCount != 0);
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//
// GPIOs
//

int ulib_plat_pin_init(
    uint32_t        port,
    uint32_t        index,
    pin_direction_t dir,
    uint16_t        mode
)
{
    GPIO_InitTypeDef init_struct = {0};
    uint32_t port_clock_addr = 1;
    uint32_t port_addr = 0x40010800;
    GPIOMode_TypeDef gpio_mode = GPIO_Mode_IN_FLOATING;
    int i = 0;   
    
    if (port > 6 || index > 16 || dir > PIN_DIR_ANALIG_OUT || mode > PIN_MODE_PP)
        return E_INVALID_PARAMS;
    
    for (i = 0; i < port + 2; i++)
        port_clock_addr *= 2;
    for (i = 0; i < port; i++)
        port_addr += 0x400;
    
    RCC_APB2PeriphClockCmd(port_clock_addr, ENABLE);
 
    if (mode & PIN_MODE_AF) {
        if (dir == PIN_DIR_DIGITAL_OUT && mode == PIN_MODE_OD)
            gpio_mode = GPIO_Mode_AF_OD;    
        else if (dir == PIN_DIR_DIGITAL_OUT && mode == PIN_MODE_PP)
            gpio_mode = GPIO_Mode_AF_PP;  
    } else {
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
    }
    
    init_struct.GPIO_Mode  = gpio_mode;
    init_struct.GPIO_Pin   = (uint16_t)(1 << index);
    init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init((GPIO_TypeDef*)port_addr, &init_struct);
    
    return 0;
}

int ulib_plat_pin_set(
    uint32_t        port,
    uint32_t        index,
    uint8_t         value
)
{
    uint32_t port_addr = 0x40010800;
    int i = 0;   
    
    if (port > 6 || index > 16)
        return E_INVALID_PARAMS;
    
    for (i = 0; i < port; i++)
        port_addr += 0x400;
    
    if (value)
        GPIO_SetBits((GPIO_TypeDef*)port_addr, (uint16_t)(1 << index));
    else
        GPIO_ResetBits((GPIO_TypeDef*)port_addr, (uint16_t)(1 << index));
    
    return 0;
}

int ulib_plat_pin_get(
    uint32_t        port,
    uint32_t        index,
    uint8_t*        ptr_value
)
{
    uint32_t port_addr = 0x40010800;
    int i = 0;
        
    if (port > 6 || index > 16)
        return E_INVALID_PARAMS;
    if (!ptr_value)
        return E_NULL_POINTER;
    
    for (i = 0; i < port; i++)
        port_addr += 0x400;
    
    *ptr_value = GPIO_ReadInputDataBit((GPIO_TypeDef*)port_addr, (uint16_t)(1 << index));   
    return 0;
}



//
// External Interrupts
//

static void nvic_configure_channel(
    uint8_t channel
)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    uint8_t inner_channel = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    
    if (channel < 5) {
        inner_channel = channel + EXTI0_IRQn;
    } else if (channel < 9) {
        inner_channel = EXTI9_5_IRQn;
    } else {
        inner_channel = EXTI15_10_IRQn;
    }
    
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_InitStruct.NVIC_IRQChannel = inner_channel;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;   
    
    NVIC_Init(&NVIC_InitStruct);
}

static void nvic_configure_pin(
    uint32_t            port,
    uint32_t            index,
    exit_trigger_mode_t mode
)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTITrigger_TypeDef inner_mode = EXTI_Trigger_Falling;
    
    if ((mode & EXTI_TRIGGER_FALLING) && (mode & EXTI_TRIGGER_RISING))
        inner_mode = EXTI_Trigger_Rising_Falling;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(port, index);
    
    EXTI_InitStruct.EXTI_Line    = (uint32_t)(1 << index);
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = inner_mode;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStruct);
}

int ulib_plat_exti_init(
    uint32_t            port,
    uint32_t            index,
    exit_trigger_mode_t mode
)
{
    int result = 0;
    
    if (port > 6 || index > 16)
        return E_INVALID_PARAMS;
    
    result = ulib_plat_pin_init(port,
                                index, PIN_DIR_DIGITAL_IN,
                                PIN_MODE_PU);
    if (result)
        return result;
    
    nvic_configure_channel(index);
    nvic_configure_pin(port, index, mode);
    
    return result;
}

static IRQ_HANDLER user_handlers[16] = {0};

#define SETUP_EXTI_HANDLER(x)                           \
    void EXTI##x##_IRQHandler(void)                     \
    {                                                   \
        if ( EXTI_GetITStatus(EXTI_Line##x) != RESET) { \
                                                        \
            if (user_handlers[x])                       \
                user_handlers[x]();                     \
                                                        \
            EXTI_ClearITPendingBit(EXTI_Line##x);       \
        }                                               \
    }
    
#define FILL_EXTI_HANDLER(x)                            \
    do {                                                \
        if ( EXTI_GetITStatus(EXTI_Line##x) != RESET) { \
                                                        \
             if (user_handlers[x])                      \
                user_handlers[x]();                     \
                                                        \
            EXTI_ClearITPendingBit(EXTI_Line##x);       \
        }                                               \
    } while (0)

SETUP_EXTI_HANDLER(0)
SETUP_EXTI_HANDLER(1)
SETUP_EXTI_HANDLER(2)
SETUP_EXTI_HANDLER(3)
SETUP_EXTI_HANDLER(4)

void EXTI9_5_IRQHandler(void)
{
    FILL_EXTI_HANDLER(5);
    FILL_EXTI_HANDLER(6);
    FILL_EXTI_HANDLER(7);
    FILL_EXTI_HANDLER(8);
    FILL_EXTI_HANDLER(9);
}

void EXTI15_10_IRQHandler(void)
{
    FILL_EXTI_HANDLER(10);
    FILL_EXTI_HANDLER(11);
    FILL_EXTI_HANDLER(12);
    FILL_EXTI_HANDLER(13);
    FILL_EXTI_HANDLER(14);
}

    
int ulib_plat_exti_set_handler(
    uint32_t            port,
    uint32_t            index,
    IRQ_HANDLER         handler
)
{
    if (port > 6 || index > 16)
        return E_INVALID_PARAMS;
    if (!handler)
        return E_NULL_POINTER;
    
    user_handlers[index] = handler;
    
    return 0;
}


//
// USART
//

uint8_t gUartMap[2][4] = {
    {PORT_A, 9, PORT_A, 10},    // {TX, RX}
    {PORT_A, 2, PORT_A,  3}
};

static void nvic_usart_init(uint32_t index)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    if (index == 0)
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    else
        NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStruct);
}


int ulib_plat_uart_init(
    uint32_t            index,
    uint32_t            baudrate        
)
{
    USART_InitTypeDef USART_InitStruct;
    
    if (index > 1)
        return E_INVALID_PARAMS;
    
    ulib_plat_pin_init(gUartMap[index][0], gUartMap[index][1],
                       PIN_DIR_DIGITAL_OUT, PIN_MODE_AF | PIN_MODE_PP);
    ulib_plat_pin_init(gUartMap[index][2], gUartMap[index][3],
                       PIN_DIR_DIGITAL_IN, PIN_MODE_FLOATING);
    
    nvic_usart_init(index);
    
    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;

    if (index == 0) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        
        USART_Init(USART1, &USART_InitStruct);
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        USART_Cmd(USART1, ENABLE);        
    } else {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        
        USART_Init(USART2, &USART_InitStruct);
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        USART_Cmd(USART2, ENABLE);
    }
    
    return 0;
}

static IRQ_HANDLER user_usart_handlers[2] = {0};

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        if (user_usart_handlers[0])
            user_usart_handlers[0]();
    }
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        if (user_usart_handlers[1])
            user_usart_handlers[1]();
    }
}

int ulib_plat_uart_set_handler(
    uint32_t            index,
    IRQ_HANDLER         handler    
)
{
    if (index > 1)
        return E_INVALID_PARAMS;
    
    user_usart_handlers[index] = handler;
    
    return 0;
}

void ulib_plat_uart_send8(
    uint32_t            index,
    int8_t              data
)
{
    USART_TypeDef* ptr_usart = 0;
    
    if (index > 1)
        return;
    
    if (index == 0)
        ptr_usart = USART1;
    else
        ptr_usart = USART2;
    
    USART_SendData(ptr_usart, (uint8_t)data);

    while(USART_GetFlagStatus(ptr_usart, USART_FLAG_TC) == RESET);
}

int8_t ulib_plat_uart_receive8(
    uint32_t            index
)
{
    USART_TypeDef* ptr_usart = 0;
    
    if (index > 1)
        return 0;
        
    if (index == 0)
        ptr_usart = USART1;
    else
        ptr_usart = USART2;
    
	while (USART_GetFlagStatus(ptr_usart, USART_FLAG_RXNE) == RESET);

	return USART_ReceiveData(ptr_usart);
}
