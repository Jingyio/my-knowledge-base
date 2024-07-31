 /**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   华邦 8M串行flash测试，并将测试信息通过串口1在电脑的超级终端中打印出来
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-MINI STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "stm32f10x.h"
#include "ulib_if.h"
#include "bsp_spi_flash.h"


typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;

/* 获取缓冲区的长度 */
#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define  BufferSize (countof(Tx_Buffer)-1)

#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

     

/* 发送缓冲区初始化 */
uint8_t Tx_Buffer[] = "感谢您选用野火stm32开发板\r\n";
uint8_t Rx_Buffer[BufferSize];

__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

// 函数原型声明
void Delay(__IO uint32_t nCount);
TestStatus Buffercmp(uint8_t* pBuffer1,uint8_t* pBuffer2, uint16_t BufferLength);

/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 */
int main(void)
{ 	
    ulib_gpio_init(ULIB_GPIO_PORTB, 0, ULIB_GPIO_DIR_OUT);
	ulib_gpio_set(ULIB_GPIO_PORTB, 0, 1);
    
	/* 配置串口为：115200 8-N-1 */
	ulib_uart_init(0, 115200, ULIB_UART_INTERRUPT_MODE_OFF);
	
	/* 8M串行flash W25Q64初始化 */
	SPI_FLASH_Init();
	
	/* 获取 Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();	
	Delay( 200 );
	
	/* 获取 SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();	
	
	/* 检验 SPI Flash ID */
	if (FlashID == sFLASH_ID)
	{	
    SPI_FLASH_BulkErase();
		
		/* 擦除将要写入的 SPI FLASH 扇区，FLASH写入前要先擦除 */
		// 这里擦除4K，即一个扇区，擦除的最小单位是扇区
		SPI_FLASH_SectorErase(FLASH_SectorToErase);	 	 
		
		/* 将发送缓冲区的数据写到flash中 */
		// 这里写一页，一页的大小为256个字节
		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);		
		
		/* 将刚刚写入的数据读出来放到接收缓冲区中 */
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);

		/* 检查写入的数据与读出的数据是否相等 */
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
		
		if( PASSED == TransferStatus1 )
		{ 
			ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);
            printf("Test Pass");
		}
		else
		{        
            ulib_gpio_set(ULIB_GPIO_PORTB, 0, 0);
            printf("Test Fail");
		}
	}// if (FlashID == sFLASH_ID)
	else// if (FlashID == sFLASH_ID)
	{ 
        printf("Flash ID not found");
	}
	
	while(1);  
}

/*
 * 函数名：Buffercmp
 * 描述  ：比较两个缓冲区中的数据是否相等
 * 输入  ：-pBuffer1     src缓冲区指针
 *         -pBuffer2     dst缓冲区指针
 *         -BufferLength 缓冲区长度
 * 输出  ：无
 * 返回  ：-PASSED pBuffer1 等于   pBuffer2
 *         -FAILED pBuffer1 不同于 pBuffer2
 */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return PASSED;
}

void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}
/*********************************************END OF FILE**********************/
