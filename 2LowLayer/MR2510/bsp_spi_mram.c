

/* Includes ------------------------------------------------------------------*/
#include "bsp_spi_mram.h"

#define SPI_MARM_PageSize              256
#define SPI_MARM_PerWritePageSize      256


void MRAM_LowLevel_DeInit(void);
void MRAM_LowLevel_Init(void); 


void SPI_MARM_PageWrite(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite);

//static void MRAM_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void bsp_MRAM_DeInit(void)
{
  MRAM_LowLevel_DeInit();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void bsp_MRAM_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  MRAM_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  MRAM_CS_HIGH();

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(MRAM_SPI, &SPI_InitStructure);

  /*!< Enable the MRAM_SPI  */
  SPI_Cmd(MRAM_SPI, ENABLE);
}

///////////////////////////////////////////////////////////////////
void bsp_MRAM_WriteStatuR(uint8_t byte)
{
   /*!< Select the FLASH: Chip Select low */
  MRAM_CS_LOW();

  /*!< Send "RDID " instruction */
  bsp_MRAM_SendByte(0x01);

  bsp_MRAM_SendByte(byte);


  MRAM_CS_HIGH();



}
/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */
uint32_t bsp_MRAM_ReadStatuR(void)
{
  uint8_t Temp = 0;

  /*!< Select the FLASH: Chip Select low */
  MRAM_CS_LOW();

  /*!< Send "RDID " instruction */
  bsp_MRAM_SendByte(0x05);

  /*!< Read a byte from the FLASH */
  Temp= bsp_MRAM_SendByte(MRAM_DUMMY_BYTE);


  MRAM_CS_HIGH();


   
  return Temp;
}



/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t bsp_MRAM_ReadByte(void)
{
  return (bsp_MRAM_SendByte(MRAM_DUMMY_BYTE));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t bsp_MRAM_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(MRAM_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(MRAM_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(MRAM_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(MRAM_SPI);
}



/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void bsp_MRAM_WriteEnable(void)
{
  /*!< Select the FLASH: Chip Select low */
  MRAM_CS_LOW();

  /*!< Send "Write Enable" instruction */
  bsp_MRAM_SendByte(MRAM_CMD_WREN);

  /*!< Deselect the FLASH: Chip Select high */
  MRAM_CS_HIGH();
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void bsp_MRAM_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

  /*!< Select the FLASH: Chip Select low */
  MRAM_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  bsp_MRAM_SendByte(MRAM_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = bsp_MRAM_SendByte(MRAM_DUMMY_BYTE);

  }
  while ((flashstatus & MRAM_WIP_FLAG) == SET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  MRAM_CS_HIGH();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void MRAM_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  MRAM_SPI_CLK_INIT(MRAM_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(MRAM_SPI_SCK_GPIO_CLK | MRAM_SPI_MISO_GPIO_CLK | 
                         MRAM_SPI_MOSI_GPIO_CLK | MRAM_CS_GPIO_CLK, ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(MRAM_SPI_SCK_GPIO_PORT, MRAM_SPI_SCK_SOURCE, MRAM_SPI_SCK_AF);
  GPIO_PinAFConfig(MRAM_SPI_MISO_GPIO_PORT, MRAM_SPI_MISO_SOURCE, MRAM_SPI_MISO_AF);
  GPIO_PinAFConfig(MRAM_SPI_MOSI_GPIO_PORT, MRAM_SPI_MOSI_SOURCE, MRAM_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
  /*!< SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = MRAM_SPI_SCK_PIN;
  GPIO_Init(MRAM_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  MRAM_SPI_MOSI_PIN;
  GPIO_Init(MRAM_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  MRAM_SPI_MISO_PIN;
  GPIO_Init(MRAM_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure MRAM Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = MRAM_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(MRAM_CS_GPIO_PORT, &GPIO_InitStructure);
}
///////////////////////////////////////////////////////////////////////////////////////
void bsp_MRAM_Write(uint32_t WriteAddr,uint8_t Wdata)
{
  /*!< Enable the write access to the FLASH */
  bsp_MRAM_WriteEnable();

  /*!< Select the FLASH: Chip Select low */
  MRAM_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  bsp_MRAM_SendByte(MRAM_CMD_WRITE);
/*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  bsp_MRAM_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  bsp_MRAM_SendByte((WriteAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  bsp_MRAM_SendByte(WriteAddr & 0xFF);

  /*!< while there is data to be written on the FLASH */
    bsp_MRAM_SendByte(Wdata);

    //////////////////////////////////////////////////
    
    ////////////////////////////////////////////////
  /*!< Deselect the FLASH: Chip Select high */
  MRAM_CS_HIGH();

  /*!< Wait the end of Flash writing */
  bsp_MRAM_WaitForWriteEnd();
}
///////////////////////////
 uint8_t bsp_MRAM_Read(uint32_t ReadAddr)
{
   uint8_t  mm=0;
  /*!< Select the FLASH: Chip Select low */
  MRAM_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  bsp_MRAM_SendByte(MRAM_CMD_READ);

/*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  bsp_MRAM_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  bsp_MRAM_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  bsp_MRAM_SendByte(ReadAddr & 0xFF);

 // while (NumByteToRead--) /*!< while there is data to be read */
 // {
   
    mm = bsp_MRAM_SendByte(MRAM_DUMMY_BYTE);

 // }

  /*!< Deselect the FLASH: Chip Select high */
  MRAM_CS_HIGH();
  return mm;
}
//////////////////////////////////////////////////////////////////////////////////////
/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void MRAM_LowLevel_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Disable the MRAM_SPI  ************************************************/
  SPI_Cmd(MRAM_SPI, DISABLE);
  
  /*!< DeInitializes the MRAM_SPI *******************************************/
  SPI_I2S_DeInit(MRAM_SPI);
  
  /*!< MRAM_SPI Periph clock disable ****************************************/
  MRAM_SPI_CLK_INIT(MRAM_SPI_CLK, DISABLE);
      
  /*!< Configure all pins used by the SPI as input floating *******************/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = MRAM_SPI_SCK_PIN;
  GPIO_Init(MRAM_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MRAM_SPI_MISO_PIN;
  GPIO_Init(MRAM_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MRAM_SPI_MOSI_PIN;
  GPIO_Init(MRAM_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MRAM_CS_PIN;
  GPIO_Init(MRAM_CS_GPIO_PORT, &GPIO_InitStructure);
}  	



 /**
  * @brief  ��FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * @param	pBuffer��Ҫд�����ݵ�ָ��
  * @param WriteAddr��д���ַ
  * @param  NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_MARM_PerWritePageSize
  * @retval ��
  */
void SPI_MARM_PageWrite(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
  /* ����FLASHдʹ������ */
  bsp_MRAM_WriteEnable();

  /* ѡ��FLASH: CS�͵�ƽ */
  MRAM_CS_LOW();
  /* дҳдָ��*/
  bsp_MRAM_SendByte(MRAM_CMD_WRITE);
  /*����д��ַ�ĸ�λ*/
  bsp_MRAM_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  bsp_MRAM_SendByte((WriteAddr & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  bsp_MRAM_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_MARM_PerWritePageSize)
  {
     NumByteToWrite = SPI_MARM_PerWritePageSize;
  }

  /* д������*/
  while (NumByteToWrite--)
  {
    /* ���͵�ǰҪд����ֽ����� */
    bsp_MRAM_SendByte(*pBuffer);
    /* ָ����һ�ֽ����� */
    pBuffer++;
  }

  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  MRAM_CS_HIGH();

  /* �ȴ�д�����*/
  bsp_MRAM_WaitForWriteEnd();
}


 /**
  * @brief  ��FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * @param	pBuffer��Ҫд�����ݵ�ָ��
  * @param  WriteAddr��д���ַ
  * @param  NumByteToWrite��д�����ݳ���
  * @retval ��
  */
void bsp_MRAM_BufferWrite(void *wBuf,u32 WriteAddr,  u32 NumByteToWrite)
{
    
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  
  uint8_t *pBuffer = (uint8_t *)wBuf;
	
	/*mod�������࣬��writeAddr��SPI_MARM_PageSize��������������AddrֵΪ0*/
  Addr = WriteAddr % SPI_MARM_PageSize;
	
	/*��count������ֵ���պÿ��Զ��뵽ҳ��ַ*/
  count = SPI_MARM_PageSize - Addr;	
	/*�����Ҫд��������ҳ*/
  NumOfPage =  NumByteToWrite / SPI_MARM_PageSize;
	/*mod�������࣬�����ʣ�಻��һҳ���ֽ���*/
  NumOfSingle = NumByteToWrite % SPI_MARM_PageSize;

	 /* Addr=0,��WriteAddr �պð�ҳ���� aligned  */
  if (Addr == 0) 
  {
		/* NumByteToWrite < SPI_MARM_PageSize */
    if (NumOfPage == 0) 
    {
      SPI_MARM_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_MARM_PageSize */
    {
			/*�Ȱ�����ҳ��д��*/
      while (NumOfPage--)
      {
        SPI_MARM_PageWrite(pBuffer, WriteAddr, SPI_MARM_PageSize);
        WriteAddr +=  SPI_MARM_PageSize;
        pBuffer += SPI_MARM_PageSize;
      }
			
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
      SPI_MARM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
	/* ����ַ�� SPI_MARM_PageSize ������  */
  else 
  {
		/* NumByteToWrite < SPI_MARM_PageSize */
    if (NumOfPage == 0) 
    {
			/*��ǰҳʣ���count��λ�ñ�NumOfSingleС��д����*/
      if (NumOfSingle > count) 
      {
        temp = NumOfSingle - count;
				
				/*��д����ǰҳ*/
        SPI_MARM_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;
				
				/*��дʣ�������*/
        SPI_MARM_PageWrite(pBuffer, WriteAddr, temp);
      }
      else /*��ǰҳʣ���count��λ����д��NumOfSingle������*/
      {				
        SPI_MARM_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_MARM_PageSize */
    {
			/*��ַ����������count�ֿ������������������*/
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_MARM_PageSize;
      NumOfSingle = NumByteToWrite % SPI_MARM_PageSize;

      SPI_MARM_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;
			
			/*������ҳ��д��*/
      while (NumOfPage--)
      {
        SPI_MARM_PageWrite(pBuffer, WriteAddr, SPI_MARM_PageSize);
        WriteAddr +=  SPI_MARM_PageSize;
        pBuffer += SPI_MARM_PageSize;
      }
			/*���ж���Ĳ���һҳ�����ݣ�����д��*/
      if (NumOfSingle != 0)
      {
        SPI_MARM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
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
void bsp_MRAM_BufferRead(void *rBuf, u32 ReadAddr, u32 NumByteToRead)
{

  uint8_t *pBuffer = (uint8_t *)rBuf; 
    
  /* ѡ��FLASH: CS�͵�ƽ */
  MRAM_CS_LOW();

  /* ���� �� ָ�� */
  bsp_MRAM_SendByte(MRAM_CMD_READ);

  /* ���� �� ��ַ��λ */
  bsp_MRAM_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* ���� �� ��ַ��λ */
  bsp_MRAM_SendByte((ReadAddr& 0xFF00) >> 8);
  /* ���� �� ��ַ��λ */
  bsp_MRAM_SendByte(ReadAddr & 0xFF);
  
	/* ��ȡ���� */
  while (NumByteToRead--)
  {
    /* ��ȡһ���ֽ�*/
    *pBuffer = bsp_MRAM_SendByte(MRAM_DUMMY_BYTE);
    /* ָ����һ���ֽڻ����� */
    pBuffer++;
  }

  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  MRAM_CS_HIGH();
}


 /**
  * @brief  ����FLASH����
  * @param  SectorAddr��Ҫ������������ַ
  * @retval ��
  */
void bsp_MRAM_SectorErase(u32 WriteAddr,u32 NumByteToCls)
{

    /* ����FLASHдʹ������ */
    bsp_MRAM_WriteEnable();
  
    /* ѡ��FLASH: CS�͵�ƽ */
    MRAM_CS_LOW();
    /* дҳдָ��*/
    bsp_MRAM_SendByte(MRAM_CMD_WRITE);
    /*����д��ַ�ĸ�λ*/
    bsp_MRAM_SendByte((WriteAddr & 0xFF0000) >> 16);
    /*����д��ַ����λ*/
    bsp_MRAM_SendByte((WriteAddr & 0xFF00) >> 8);
    /*����д��ַ�ĵ�λ*/
    bsp_MRAM_SendByte(WriteAddr & 0xFF);
  
    /* д������*/
    while (NumByteToCls--)
    {
      /* ���͵�ǰҪд����ֽ����� */
      bsp_MRAM_SendByte(0x00);
    }
  
    /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
    MRAM_CS_HIGH();
  
    /* �ȴ�д�����*/
    bsp_MRAM_WaitForWriteEnd();


}



 /**
  * @brief  ����FLASH��������Ƭ����
  * @param  ��
  * @retval ��
  */
void bsp_MRAM_BulkErase(void)
{
  uint32_t NumByteToCls = 131072;
  /* ����FLASHдʹ������ */
  bsp_MRAM_WriteEnable();
  
  /* ѡ��FLASH: CS�͵�ƽ */
  MRAM_CS_LOW();
  /* дҳдָ��*/
  bsp_MRAM_SendByte(MRAM_CMD_WRITE);
  /*����д��ַ�ĸ�λ*/
  bsp_MRAM_SendByte((0 & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  bsp_MRAM_SendByte((0 & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  bsp_MRAM_SendByte(0 & 0xFF);
  
  /* д������*/
  while (NumByteToCls--)
  {
    /* ���͵�ǰҪд����ֽ����� */
    bsp_MRAM_SendByte(0x00);
  }
  
  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  MRAM_CS_HIGH();
  
  /* �ȴ�д�����*/
  bsp_MRAM_WaitForWriteEnd();

}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
