 /**
  ******************************************************************************
  * @file    bsp_xxx.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   spi flash �ײ�Ӧ�ú���bsp 
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-MINI STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_spi_flash.h"

static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT;    
//static uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode);

/**
  * @brief  SPI_FLASH��ʼ��
  * @param  ��
  * @retval ��
  */
void SPI_FLASH_Init(void)
{
    ulib_spi_config_t config = { 0 };
    
    config.cpha = 0;
    config.cpol = 0;
    config.spi_pins.cs_port = ULIB_GPIO_PORTA;
    config.spi_pins.sck_port = ULIB_GPIO_PORTA;
    config.spi_pins.miso_port = ULIB_GPIO_PORTA;
    config.spi_pins.miso_port = ULIB_GPIO_PORTA;
    config.spi_pins.cs_pin = 4;
    config.spi_pins.sck_pin = 5;
    config.spi_pins.mosi_pin = 7;
    config.spi_pins.miso_pin = 6;

    ulib_spi_init(0, config);
    ulib_spi_select(0, 1);
}
 /**
  * @brief  ����FLASH����
  * @param  SectorAddr��Ҫ������������ַ
  * @retval ��
  */
void SPI_FLASH_SectorErase(u32 SectorAddr)
{
  /* ����FLASHдʹ������ */
  SPI_FLASH_WriteEnable();
  SPI_FLASH_WaitForWriteEnd();
  /* �������� */
  /* ѡ��FLASH: CS�͵�ƽ */
  ulib_spi_select(0, 0);
  /* ������������ָ��*/
  SPI_FLASH_SendByte(W25X_SectorErase);
  /*���Ͳ���������ַ�ĸ�λ*/
  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* ���Ͳ���������ַ����λ */
  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* ���Ͳ���������ַ�ĵ�λ */
  SPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  ulib_spi_select(0, 1);
  /* �ȴ��������*/
  SPI_FLASH_WaitForWriteEnd();
}

 /**
  * @brief  ����FLASH��������Ƭ����
  * @param  ��
  * @retval ��
  */
void SPI_FLASH_BulkErase(void)
{
  /* ����FLASHдʹ������ */
  SPI_FLASH_WriteEnable();

  /* ���� Erase */
  /* ѡ��FLASH: CS�͵�ƽ */
  ulib_spi_select(0, 0);
  /* �����������ָ��*/
  SPI_FLASH_SendByte(W25X_ChipErase);
  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  ulib_spi_select(0, 1);

  /* �ȴ��������*/
  SPI_FLASH_WaitForWriteEnd();
}

 /**
  * @brief  ��FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * @param	pBuffer��Ҫд�����ݵ�ָ��
  * @param WriteAddr��д���ַ
  * @param  NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_FLASH_PerWritePageSize
  * @retval ��
  */
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* ����FLASHдʹ������ */
  SPI_FLASH_WriteEnable();

  /* ѡ��FLASH: CS�͵�ƽ */
  ulib_spi_select(0, 0);
  /* дҳдָ��*/
  SPI_FLASH_SendByte(W25X_PageProgram);
  /*����д��ַ�ĸ�λ*/
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  SPI_FLASH_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
     FLASH_ERROR("SPI_FLASH_PageWrite too large!"); 
  }

  /* д������*/
  while (NumByteToWrite--)
  {
    /* ���͵�ǰҪд����ֽ����� */
    SPI_FLASH_SendByte(*pBuffer);
    /* ָ����һ�ֽ����� */
    pBuffer++;
  }

  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  ulib_spi_select(0, 1);

  /* �ȴ�д�����*/
  SPI_FLASH_WaitForWriteEnd();
}

 /**
  * @brief  ��FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * @param	pBuffer��Ҫд�����ݵ�ָ��
  * @param  WriteAddr��д���ַ
  * @param  NumByteToWrite��д�����ݳ���
  * @retval ��
  */
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
	/*mod�������࣬��writeAddr��SPI_FLASH_PageSize��������������AddrֵΪ0*/
  Addr = WriteAddr % SPI_FLASH_PageSize;
	
	/*��count������ֵ���պÿ��Զ��뵽ҳ��ַ*/
  count = SPI_FLASH_PageSize - Addr;
	/*�����Ҫд��������ҳ*/
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	/*mod�������࣬�����ʣ�಻��һҳ���ֽ���*/
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
	
	/* Addr=0,��WriteAddr �պð�ҳ���� aligned  */
  if (Addr == 0)
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0) 
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    { 
			/*�Ȱ�����ҳ��д��*/
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
	/* ����ַ�� SPI_FLASH_PageSize ������  */
  else 
  {
		/* NumByteToWrite < SPI_FLASH_PageSize */
    if (NumOfPage == 0)
    {
			/*��ǰҳʣ���count��λ�ñ�NumOfSingleС��һҳд����*/
      if (NumOfSingle > count) 
      {
        temp = NumOfSingle - count;
				/*��д����ǰҳ*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
				
        WriteAddr +=  count;
        pBuffer += count;
				/*��дʣ�������*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else /*��ǰҳʣ���count��λ����д��NumOfSingle������*/
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
			/*��ַ����������count�ֿ��������������������*/
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
			
			/* ��д��count�����ݣ�Ϊ��������һ��Ҫд�ĵ�ַ���� */
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
			
			/* ���������ظ���ַ�������� */
      WriteAddr +=  count;
      pBuffer += count;
			/*������ҳ��д��*/
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

 /**
  * @brief  ��ȡFLASH����
  * @param 	pBuffer���洢�������ݵ�ָ��
  * @param   ReadAddr����ȡ��ַ
  * @param   NumByteToRead����ȡ���ݳ���
  * @retval ��
  */
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* ѡ��FLASH: CS�͵�ƽ */
  ulib_spi_select(0, 0);

  /* ���� �� ָ�� */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* ���� �� ��ַ��λ */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* ���� �� ��ַ��λ */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* ���� �� ��ַ��λ */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
	
	/* ��ȡ���� */
  while (NumByteToRead--) /* while there is data to be read */
  {
    /* ��ȡһ���ֽ�*/
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /* ָ����һ���ֽڻ����� */
    pBuffer++;
  }

  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  ulib_spi_select(0, 1);
}

 /**
  * @brief  ��ȡFLASH ID
  * @param 	��
  * @retval FLASH ID
  */
u32 SPI_FLASH_ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* ��ʼͨѶ��CS�͵�ƽ */
  ulib_spi_select(0, 0);

  /* ����JEDECָ���ȡID */
  SPI_FLASH_SendByte(W25X_JedecDeviceID);

  /* ��ȡһ���ֽ����� */
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte);

  /* ��ȡһ���ֽ����� */
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte);

  /* ��ȡһ���ֽ����� */
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

 /* ֹͣͨѶ��CS�ߵ�ƽ */
  ulib_spi_select(0, 1);

  /*�����������������Ϊ�����ķ���ֵ*/
	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}
 /**
  * @brief  ��ȡFLASH Device ID
  * @param 	��
  * @retval FLASH Device ID
  */
u32 SPI_FLASH_ReadDeviceID(void)
{
  u32 Temp = 0;

  /* Select the FLASH: Chip Select low */
  ulib_spi_select(0, 0);

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(W25X_DeviceID);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  
  /* Read a byte from the FLASH */
  Temp = SPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  ulib_spi_select(0, 1);

  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
  /* Select the FLASH: Chip Select low */
  ulib_spi_select(0, 0);

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
}


 /**
  * @brief  ʹ��SPI��ȡһ���ֽڵ�����
  * @param  ��
  * @retval ���ؽ��յ�������
  */
u8 SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

 /**
  * @brief  ʹ��SPI����һ���ֽڵ�����
  * @param  byte��Ҫ���͵�����
  * @retval ���ؽ��յ�������
  */
u8 SPI_FLASH_SendByte(u8 byte)
{
    return ulib_spi_readwrite(0, byte);
}

// /**
//  * @brief  ʹ��SPI���������ֽڵ�����
//  * @param  byte��Ҫ���͵�����
//  * @retval ���ؽ��յ�������
//  */
//u16 SPI_FLASH_SendHalfWord(u16 HalfWord)
//{
//	  SPITimeout = SPIT_FLAG_TIMEOUT;
//  /* �ȴ����ͻ�����Ϊ�գ�TXE�¼� */
//  while (SPI_I2S_GetFlagStatus(FLASH_SPIx , SPI_I2S_FLAG_TXE) == RESET)
//	{
//    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(2);
//   }
//	
//  /* д�����ݼĴ�������Ҫд�������д�뷢�ͻ����� */
//  SPI_I2S_SendData(FLASH_SPIx , HalfWord);

//	 SPITimeout = SPIT_FLAG_TIMEOUT;
//  /* �ȴ����ջ������ǿգ�RXNE�¼� */
//  while (SPI_I2S_GetFlagStatus(FLASH_SPIx , SPI_I2S_FLAG_RXNE) == RESET)
//	 {
//    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(3);
//   }
//  /* ��ȡ���ݼĴ�������ȡ���ջ��������� */
//  return SPI_I2S_ReceiveData(FLASH_SPIx );
//}

 /**
  * @brief  ��FLASH���� дʹ�� ����
  * @param  none
  * @retval none
  */
void SPI_FLASH_WriteEnable(void)
{
  /* ͨѶ��ʼ��CS�� */
  ulib_spi_select(0, 0);

  /* ����дʹ������*/
  SPI_FLASH_SendByte(W25X_WriteEnable);

  /*ͨѶ������CS�� */
  ulib_spi_select(0, 1);
}

/* WIP(busy)��־��FLASH�ڲ�����д�� */
#define WIP_Flag                  0x01

 /**
  * @brief  �ȴ�WIP(BUSY)��־����0�����ȴ���FLASH�ڲ�����д�����
  * @param  none
  * @retval none
  */
void SPI_FLASH_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;

  /* ѡ�� FLASH: CS �� */
  ulib_spi_select(0, 0);

  /* ���� ��״̬�Ĵ��� ���� */
  SPI_FLASH_SendByte(W25X_ReadStatusReg);

  /* ��FLASHæµ����ȴ� */
  do
  {
		/* ��ȡFLASHоƬ��״̬�Ĵ��� */
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);	 
  }
  while ((FLASH_Status & WIP_Flag) == SET);  /* ����д���־ */

  /* ֹͣ�ź�  FLASH: CS �� */
  ulib_spi_select(0, 1);
}


//�������ģʽ
void SPI_Flash_PowerDown(void)   
{ 
  /* ͨѶ��ʼ��CS�� */
  ulib_spi_select(0, 0);

  /* ���� ���� ���� */
  SPI_FLASH_SendByte(W25X_PowerDown);

  /*ͨѶ������CS�� */
  ulib_spi_select(0, 1);
}   

//����
void SPI_Flash_WAKEUP(void)   
{
  /*ѡ�� FLASH: CS �� */
  ulib_spi_select(0, 0);

  /* ���� �ϵ� ���� */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);

   /* ֹͣ�ź� FLASH: CS �� */
  ulib_spi_select(0, 1);
}   
   

///**
//  * @brief  �ȴ���ʱ�ص�����
//  * @param  None.
//  * @retval None.
//  */
//static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
//{
//  /* �ȴ���ʱ��Ĵ���,���������Ϣ */
//  FLASH_ERROR("SPI �ȴ���ʱ!errorCode = %d",errorCode);
//  return 0;
//}
   
/*********************************************END OF FILE**********************/