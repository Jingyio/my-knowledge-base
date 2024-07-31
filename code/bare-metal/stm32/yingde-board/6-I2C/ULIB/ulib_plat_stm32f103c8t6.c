#include "ulib_if.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>

//
// Common
//

void ulib_plat_delay_us(
    uint32_t us
)
{
	uint32_t i = 0;
	SysTick_Config(SystemCoreClock / 1000000);
	
	for (i = 0; i < us; i++)
	{
		while(!((SysTick->CTRL) & (1 << 16)));
	}

	SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
}


#if ULIB_GPIO_SUPPORT

//
// GPIOs
//

static inline uint32_t stm32_clock_of(uint32_t port)
{
    uint32_t temp = RCC_APB2Periph_GPIOA;
    int i = 0;

    for (i = 0; i < port; i++)
        temp *= 2;

    return temp;
}

static inline GPIO_TypeDef* stm32_port_of(unsigned int index)
{
    return (GPIO_TypeDef*)((unsigned char*)GPIOA_BASE + index * 0x400);
}

static inline uint16_t stm32_pin_of(unsigned int index)
{
    return (GPIO_Pin_0 << index);
}

void stm32_gpio_init(
    uint32_t         clock,
    GPIO_TypeDef*    port,
    uint16_t         pin,
    GPIOMode_TypeDef mode
)
{
    GPIO_InitTypeDef init_struct = {0};

    RCC_APB2PeriphClockCmd(clock, ENABLE);
    
    init_struct.GPIO_Mode  = mode;
    init_struct.GPIO_Pin   = pin;
    init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(port, &init_struct);
}

ulib_error_t ulib_plat_gpio_init(
    uint32_t        port,
    uint32_t        pin,
    ulib_gpio_dir_t dir
)
{
    if (port > ULIB_GPIO_PORTG || pin > 16)
        return ULIB_E_INVALID_PARAMS;
    
    stm32_gpio_init(stm32_clock_of(port),
                    stm32_port_of(port),
                    stm32_pin_of(pin),
                    dir == ULIB_GPIO_DIR_IN ? GPIO_Mode_IN_FLOATING : GPIO_Mode_Out_PP);

    return ULIB_S_OK;
}

ulib_error_t ulib_plat_gpio_input_init(
    uint32_t               port,
    uint32_t               pin,
    ulib_gpio_input_mode_t input_mode
)
{
    GPIOMode_TypeDef gpio_mode_def = GPIO_Mode_IN_FLOATING;
    
    if (port > ULIB_GPIO_PORTG || pin > 16)
        return ULIB_E_INVALID_PARAMS;
    
    if (input_mode == ULIB_GPIO_INPUT_MODE_FLOATING)
        gpio_mode_def = GPIO_Mode_IN_FLOATING;
    else if (input_mode == ULIB_GPIO_INPUT_MODE_PU)
        gpio_mode_def = GPIO_Mode_IPU;
    else if (input_mode == ULIB_GPIO_INPUT_MODE_PD)
        gpio_mode_def = GPIO_Mode_IPD;
    
    stm32_gpio_init(stm32_clock_of(port),
                    stm32_port_of(port),
                    stm32_pin_of(pin),
                    gpio_mode_def);

    return ULIB_S_OK;    
}

ulib_error_t ulib_plat_gpio_set(
    uint32_t        port,
    uint32_t        pin,
    uint8_t         value
)
{
    if (port > ULIB_GPIO_PORTG || pin > 16)
        return ULIB_E_INVALID_PARAMS;
    
    if (value)
        GPIO_SetBits(stm32_port_of(port), stm32_pin_of(pin));
    else
        GPIO_ResetBits(stm32_port_of(port), stm32_pin_of(pin));
    
    return ULIB_S_OK;
}

ulib_error_t ulib_plat_gpio_get(
    uint32_t        port,
    uint32_t        pin,
    uint8_t*        ptr_value
)
{
    if (port > ULIB_GPIO_PORTG || pin > 16)
        return ULIB_E_INVALID_PARAMS;
    if (!ptr_value)
        return ULIB_E_NULL_POINTER;
    
    *ptr_value = GPIO_ReadInputDataBit(stm32_port_of(port), stm32_pin_of(pin));   
    return ULIB_S_OK;
}

#endif


#if ULIB_EXTI_SUPPORT

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
    uint32_t            pin,
    ulib_exti_trigger_t mode
)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTITrigger_TypeDef inner_mode = EXTI_Trigger_Falling;
    
    if ((mode & ULIB_EXTI_TRIGGER_FALLING) && (mode & ULIB_EXTI_TRIGGER_RISING))
        inner_mode = EXTI_Trigger_Rising_Falling;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(port, pin);
    
    EXTI_InitStruct.EXTI_Line    = (uint32_t)(1 << pin);
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = inner_mode;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStruct);
}

ulib_error_t ulib_plat_exti_init(
    uint32_t            port,
    uint32_t            pin,
    ulib_exti_trigger_t mode
)
{
    ulib_error_t result = ULIB_S_OK;
    
    if (port > ULIB_GPIO_PORTG || pin > 16)
        return ULIB_E_INVALID_PARAMS;
    
    result = ulib_plat_gpio_input_init(port, pin, ULIB_GPIO_INPUT_MODE_PU);
    
    if (result)
        return result;
    
    nvic_configure_channel(pin);
    nvic_configure_pin(port, pin, mode);
    
    return result;
}

static ULIB_INTERRUPT_HANDLER user_handlers[16] = {0};

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

    
ulib_error_t ulib_plat_exti_set_handler(
    uint32_t            port,
    uint32_t            pin,
    ULIB_INTERRUPT_HANDLER   handler
)
{
    if (port > ULIB_GPIO_PORTG || pin > 16)
        return ULIB_E_INVALID_PARAMS;
    if (!handler)
        return ULIB_E_NULL_POINTER;
    
    user_handlers[pin] = handler;
    
    return ULIB_S_OK;
}

#endif


#if ULIB_UART_SUPPORT

//
// USART
//

//const int gUartMap[2][4] = {
//    {ULIB_GPIO_PORTA, 9, ULIB_GPIO_PORTA, 10},    // {TX, RX}
//    {ULIB_GPIO_PORTA, 2, ULIB_GPIO_PORTA,  3}
//};

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


ulib_error_t ulib_plat_uart_init(
    uint32_t            index,
    uint32_t            baudrate,
    ulib_uart_interrupt_mode_t mode
)
{
    USART_InitTypeDef USART_InitStruct = {0};
    int tx_port = 0;
    int tx_pin = 0;
    int rx_port = 0;
    int rx_pin = 0;
    
    if (index > 2)
        return ULIB_E_INVALID_PARAMS;
    
    switch (index) {
        case 0:
            tx_port = ULIB_GPIO_PORTA;
            tx_pin = 9;
            rx_port = ULIB_GPIO_PORTA;
            rx_pin = 10;
            break;
        case 1:
            tx_port = ULIB_GPIO_PORTA;
            tx_pin = 2;
            rx_port = ULIB_GPIO_PORTA;
            rx_pin = 3;
            break;
        case 2:
            tx_port = ULIB_GPIO_PORTB;
            tx_pin = 10;
            rx_port = ULIB_GPIO_PORTB;
            rx_pin = 11;
            break;
    }
    
    stm32_gpio_init(stm32_clock_of(tx_port),
                    stm32_port_of(tx_port),
                    stm32_pin_of(tx_pin),
                    GPIO_Mode_AF_PP);
    stm32_gpio_init(stm32_clock_of(rx_port),
                    stm32_port_of(rx_port),
                    stm32_pin_of(rx_pin),
                    GPIO_Mode_IN_FLOATING);
    
    USART_InitStruct.USART_BaudRate = baudrate;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;

    if (mode != ULIB_UART_INTERRUPT_MODE_OFF)
        nvic_usart_init(index);
    
    if (index == 0) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        
        USART_Init(USART1, &USART_InitStruct);
        
        if (mode & ULIB_UART_INTERRUPT_MODE_RX)
            USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        if (mode & ULIB_UART_INTERRUPT_MODE_TX)
            USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        
        USART_Cmd(USART1, ENABLE);        
    } else {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        
        USART_Init(USART2, &USART_InitStruct);

        if (mode & ULIB_UART_INTERRUPT_MODE_RX)
            USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        if (mode & ULIB_UART_INTERRUPT_MODE_TX)
            USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
        
        USART_Cmd(USART2, ENABLE);
    }
    
    return ULIB_S_OK;
}

static ULIB_INTERRUPT_HANDLER user_tx_handlers[2] = {0};
static ULIB_INTERRUPT_HANDLER user_rx_handlers[2] = {0};

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        if (user_rx_handlers[0])
            user_rx_handlers[0]();
    }
    
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
        if (user_tx_handlers[0])
            user_tx_handlers[0]();
    }
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        if (user_rx_handlers[1])
            user_rx_handlers[1]();
    }
    
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        if (user_tx_handlers[1])
            user_tx_handlers[1]();
    }
}

ulib_error_t ulib_plat_uart_set_handler(
    uint32_t                   index,
    ulib_uart_interrupt_mode_t mode,
    ULIB_INTERRUPT_HANDLER     handler    
)
{
    ULIB_INTERRUPT_HANDLER* ptr_handler = 0;
    
    if (index > 1)
        return ULIB_E_INVALID_PARAMS;
    
    if (mode & ULIB_UART_INTERRUPT_MODE_RX)
        ptr_handler = user_rx_handlers;
    if (mode & ULIB_UART_INTERRUPT_MODE_TX)
        ptr_handler = user_tx_handlers;
    
    ptr_handler[index] = handler;
    
    return ULIB_S_OK;
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

int fputc(int ch, FILE *f)
{
    ulib_plat_uart_send8(0, ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		
	
    return (ch);
}

int fgetc(FILE *f)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

    return (int)ulib_plat_uart_receive8(0);
}

void ulib_plat_assert_init(void)
{
    ulib_plat_uart_init(0, 115200, ULIB_UART_INTERRUPT_MODE_OFF);
}

#endif


#if ULIB_DMA_SUPPORT

void ulib_plat_dma_memcpy(
    uint32_t    channel,
    uint8_t*    ptr_src,
    uint8_t*    ptr_dest,
    uint32_t    size_in_bytes
)
{
    DMA_InitTypeDef dma_config;
    DMA_Channel_TypeDef* ptr_dma = 0;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    dma_config.DMA_BufferSize = size_in_bytes;
    dma_config.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma_config.DMA_M2M = DMA_M2M_Enable;
    dma_config.DMA_MemoryBaseAddr = (uint32_t)ptr_dest;
    dma_config.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    dma_config.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_config.DMA_Mode = DMA_Mode_Normal;
    dma_config.DMA_PeripheralBaseAddr = (uint32_t)ptr_src;
    dma_config.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_config.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
    dma_config.DMA_Priority = DMA_Priority_High;
    
    switch (channel) {
        case 0:
            ptr_dma = DMA1_Channel1;
            break;
        case 1:
            ptr_dma = DMA1_Channel2;
            break;
        case 2:
            ptr_dma = DMA1_Channel3;
            break;
        case 3:
            ptr_dma = DMA1_Channel4;
            break;
        case 4:
            ptr_dma = DMA1_Channel5;
            break;
        case 5:
            ptr_dma = DMA1_Channel6;
            break;
        case 6:
            ptr_dma = DMA1_Channel7;
            break;
    }
    
    DMA_Init(ptr_dma, &dma_config);
    DMA_Cmd(ptr_dma, ENABLE);
}

void ulib_plat_dma_wait4done(
    uint32_t    channel
)
{
    uint32_t flag_tc = 0;
    
    switch (channel) {
        case 0:
            flag_tc = DMA1_FLAG_TC1;
            break;
        case 1:
            flag_tc = DMA1_FLAG_TC2;
            break;
        case 2:
            flag_tc = DMA1_FLAG_TC3;
            break;
        case 3:
            flag_tc = DMA1_FLAG_TC4;
            break;
        case 4:
            flag_tc = DMA1_FLAG_TC5;
            break;
        case 5:
            flag_tc = DMA1_FLAG_TC6;
            break;
        case 6:
            flag_tc = DMA1_FLAG_TC7;
            break;
    }
    
    while (DMA_GetFlagStatus(flag_tc) == RESET);
}


#endif


#if ULIB_I2C_SUPPORT

ulib_error_t ulib_plat_i2c_scl_init(
    uint32_t            scl_port,
    uint32_t            scl_pin
)
{
    if (scl_port > ULIB_GPIO_PORTG)
        return ULIB_E_INVALID_PARAMS;
    if (scl_pin > 16)
        return ULIB_E_INVALID_PARAMS;
    
    stm32_gpio_init(stm32_clock_of(scl_port),
                    stm32_port_of(scl_port),
                    1 << scl_pin,
                    GPIO_Mode_Out_OD);
    
    return ULIB_S_OK;
}

ulib_error_t ulib_plat_i2c_sda_init(
    uint32_t            sda_port,
    uint32_t            sda_pin
)
{
    return ulib_plat_i2c_scl_init(sda_port, sda_pin);
}

ulib_error_t ulib_plat_i2c_pin_set(
    uint32_t            port,
    uint32_t            pin,
    uint8_t             value
)
{
    return ulib_plat_gpio_set(port, pin, value);
}

uint8_t ulib_plat_i2c_pin_get(
    uint32_t            port,
    uint32_t            pin
)
{
    uint8_t value = 0;
    
    ulib_plat_gpio_get(port, pin, &value);
    return value;
}

void ulib_plat_i2c_delay(
    void
)
{
    uint8_t i = 3;
    while (i--);
}

#endif

#if ULIB_SPI_SUPPORT
void ulib_plat_spi_output_init(
    uint32_t    port,
    uint32_t    pin
)
{
    if (port > ULIB_GPIO_PORTG)
        return;
    if (pin > 16)
        return;
    
    stm32_gpio_init(stm32_clock_of(port),
                    stm32_port_of(port),
                    stm32_pin_of(pin),
                    GPIO_Mode_Out_PP);
}

void ulib_plat_spi_input_init(
    uint32_t    port,
    uint32_t    pin
)
{
    if (port > ULIB_GPIO_PORTG)
        return;
    if (pin > 16)
        return;
    
    stm32_gpio_init(stm32_clock_of(port),
                    stm32_port_of(port),
                    stm32_pin_of(pin),
                    GPIO_Mode_IN_FLOATING);
}

void ulib_plat_spi_pin_set(
    uint32_t    port,
    uint32_t    pin,
    uint8_t     value
)
{
    ulib_plat_gpio_set(port, pin, value);
}

uint8_t ulib_plat_spi_pin_get(
    uint32_t    port,
    uint32_t    pin
)
{
    uint8_t value = 0;
    ulib_plat_gpio_get(port, pin, &value);
    return value;
}

void ulib_plat_spi_delay(
    void
)
{
    uint8_t i = 3;
    while (i--);
}
#endif

#if ULIB_TIMER_SUPPORT

#define ULIB_TIMER_MAX_NUM  (2)

static ULIB_INTERRUPT_HANDLER g_timer_handlers[ULIB_TIMER_MAX_NUM] = { 0 };

static void nvic_timer_config(
    uint32_t index
)
{
    NVIC_InitTypeDef init_struct = { 0 }; 

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    if (index == 0)
        init_struct.NVIC_IRQChannel = TIM2_IRQn;
    else
        init_struct.NVIC_IRQChannel = TIM3_IRQn;
    init_struct.NVIC_IRQChannelPreemptionPriority = 0;
    init_struct.NVIC_IRQChannelSubPriority = 3;
    init_struct.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&init_struct);
}

ulib_error_t ulib_plat_timer_start(
    uint32_t                index,
    uint64_t                period_in_us,
    ULIB_INTERRUPT_HANDLER  callback
)
{
    TIM_TimeBaseInitTypeDef init_struct;
    TIM_TypeDef* timer = 0;
    
    switch (index) {
        case 0:
            timer = TIM2;
            nvic_timer_config(0);
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
            g_timer_handlers[0] = callback;

        case 1:
            timer = TIM3;
            nvic_timer_config(1);
            g_timer_handlers[1] = callback;
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        break;
        default:
            return ULIB_E_INVALID_PARAMS;
    }
	
    init_struct.TIM_Period = period_in_us;        // period_in_us count every interrupt
    init_struct.TIM_Prescaler = 71;               // 1 us per count
    init_struct.TIM_ClockDivision = 0;
    init_struct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(timer, &init_struct);
    TIM_ClearFlag(timer, TIM_FLAG_Update);
    
    TIM_ARRPreloadConfig(timer, ENABLE);
    TIM_ITConfig(timer, TIM_IT_Update, ENABLE);
    TIM_Cmd(timer, ENABLE);
    
    return ULIB_S_OK;
}

ulib_error_t ulib_plat_timer_stop(
    uint32_t             index
)
{
    if (index == 0) {
        TIM_Cmd(TIM2, DISABLE);
        g_timer_handlers[0] = 0;
        return ULIB_S_OK;
    } else if (index == 1) {
        TIM_Cmd(TIM3, DISABLE); 
        g_timer_handlers[1] = 0;
        return ULIB_S_OK;
    }
    
    return ULIB_E_INVALID_PARAMS;
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        if (g_timer_handlers[0])
            g_timer_handlers[0]();
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
	}		 	
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        if (g_timer_handlers[1])
            g_timer_handlers[1]();        
        TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
	}		 	
}

#endif

#if ULIB_ADC_DAC_SUPPORT
ulib_error_t ulib_plat_adc_init(
    uint32_t            port,
    uint32_t            index
)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    
    ULIB_ASSERT(port == ULIB_GPIO_PORTA);
    ULIB_ASSERT(index < 8);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    
	GPIO_InitStructure.GPIO_Pin = stm32_pin_of(index);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_Init(ADC1, &ADC_InitStructure);
    
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1)) {}
    
    return ULIB_S_OK;
}

float ulib_plat_adc_get(
    uint32_t            port,
    uint32_t            index
)
{
    float value = 0.0;
    
    ULIB_ASSERT(port == ULIB_GPIO_PORTA);
    ULIB_ASSERT(index < 8);
    
    ADC_RegularChannelConfig(ADC1, index, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != 1);
    value = (float)ADC_GetConversionValue(ADC1) * 3.3 / 4096;
    
    return value;
}

#endif
