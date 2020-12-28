

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"

#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256


void sFLASH_LowLevel_DeInit(void);
void sFLASH_LowLevel_Init(void); 


void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite);

static void sFLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_DeInit(void)
{
  sFLASH_LowLevel_DeInit();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  sFLASH_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

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
  SPI_Init(sFLASH_SPI, &SPI_InitStructure);

  /*!< Enable the sFLASH_SPI  */
  SPI_Cmd(sFLASH_SPI, ENABLE);
}

///////////////////////////////////////////////////////////////////
void sFLASH_WriteStatuR(uint8_t byte)
{
   /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x01);

  sFLASH_SendByte(byte);


  sFLASH_CS_HIGH();



}
/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */
uint32_t sFLASH_ReadStatuR(void)
{
  uint8_t Temp = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x05);

  /*!< Read a byte from the FLASH */
  Temp= sFLASH_SendByte(sFLASH_DUMMY_BYTE);


  sFLASH_CS_HIGH();


   
  return Temp;
}



/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t sFLASH_ReadByte(void)
{
  return (sFLASH_SendByte(sFLASH_DUMMY_BYTE));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t sFLASH_SendByte(uint8_t byte)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sFLASH_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(sFLASH_SPI);
}



/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void sFLASH_WriteEnable(void)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Write Enable" instruction */
  sFLASH_SendByte(sFLASH_CMD_WREN);

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void sFLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  sFLASH_SendByte(sFLASH_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  }
  while ((flashstatus & sFLASH_WIP_FLAG) == SET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  sFLASH_SPI_CLK_INIT(sFLASH_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(sFLASH_SPI_SCK_GPIO_CLK | sFLASH_SPI_MISO_GPIO_CLK | 
                         sFLASH_SPI_MOSI_GPIO_CLK | sFLASH_CS_GPIO_CLK, ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(sFLASH_SPI_SCK_GPIO_PORT, sFLASH_SPI_SCK_SOURCE, sFLASH_SPI_SCK_AF);
  GPIO_PinAFConfig(sFLASH_SPI_MISO_GPIO_PORT, sFLASH_SPI_MISO_SOURCE, sFLASH_SPI_MISO_AF);
  GPIO_PinAFConfig(sFLASH_SPI_MOSI_GPIO_PORT, sFLASH_SPI_MOSI_SOURCE, sFLASH_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        
  /*!< SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_SCK_PIN;
  GPIO_Init(sFLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  sFLASH_SPI_MOSI_PIN;
  GPIO_Init(sFLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  sFLASH_SPI_MISO_PIN;
  GPIO_Init(sFLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sFLASH Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = sFLASH_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(sFLASH_CS_GPIO_PORT, &GPIO_InitStructure);
}
///////////////////////////////////////////////////////////////////////////////////////
void sFLASH_Write(uint32_t WriteAddr,uint8_t Wdata)
{
  /*!< Enable the write access to the FLASH */
  sFLASH_WriteEnable();

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_WRITE);
/*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  sFLASH_SendByte((WriteAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  sFLASH_SendByte(WriteAddr & 0xFF);

  /*!< while there is data to be written on the FLASH */
    sFLASH_SendByte(Wdata);

    //////////////////////////////////////////////////
    
    ////////////////////////////////////////////////
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}
///////////////////////////
 uint8_t sFLASH_Read(uint32_t ReadAddr)
{
   uint8_t  mm=0;
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_READ);

/*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  sFLASH_SendByte(ReadAddr & 0xFF);

 // while (NumByteToRead--) /*!< while there is data to be read */
 // {
   
    mm = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

 // }

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
  return mm;
}
//////////////////////////////////////////////////////////////////////////////////////
/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_LowLevel_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Disable the sFLASH_SPI  ************************************************/
  SPI_Cmd(sFLASH_SPI, DISABLE);
  
  /*!< DeInitializes the sFLASH_SPI *******************************************/
  SPI_I2S_DeInit(sFLASH_SPI);
  
  /*!< sFLASH_SPI Periph clock disable ****************************************/
  sFLASH_SPI_CLK_INIT(sFLASH_SPI_CLK, DISABLE);
      
  /*!< Configure all pins used by the SPI as input floating *******************/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_SCK_PIN;
  GPIO_Init(sFLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MISO_PIN;
  GPIO_Init(sFLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = sFLASH_SPI_MOSI_PIN;
  GPIO_Init(sFLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = sFLASH_CS_PIN;
  GPIO_Init(sFLASH_CS_GPIO_PORT, &GPIO_InitStructure);
}  	

#if 0

/*
*********************************************************************************************************
*   �� �� ��: sf_PageWrite
*   ����˵��: ��һ��page��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*   ��    ��:     _pBuf : ����Դ��������
*               _uiWriteAddr ��Ŀ�������׵�ַ
*               _usSize �����ݸ��������ܳ���ҳ���С
*   �� �� ֵ: ��
*********************************************************************************************************
*/
void sFLASH_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
    uint32_t i, j;

    for (j = 0; j < _usSize / 256; j++)
    {
        sf_WriteEnable();                               /* ����дʹ������ */

        SF_CS_LOW();                                    /* ʹ��Ƭѡ */
        sf_SendByte(0x02);                              /* ����AAI����(��ַ�Զ����ӱ��) */
        sf_SendByte((_uiWriteAddr & 0xFF0000) >> 16);   /* ����������ַ�ĸ�8bit */
        sf_SendByte((_uiWriteAddr & 0xFF00) >> 8);      /* ����������ַ�м�8bit */
        sf_SendByte(_uiWriteAddr & 0xFF);               /* ����������ַ��8bit */

        for (i = 0; i < 256; i++)
        {
            sf_SendByte(*_pBuf++);                  /* �������� */
        }

        SF_CS_HIGH();                               /* ��ֹƬѡ */

        sf_WaitForWriteEnd();                       /* �ȴ�����Flash�ڲ�д������� */

        _uiWriteAddr += 256;
    }

    /* ����д����״̬ */
    SF_CS_LOW();
    sf_SendByte(CMD_DISWR);
    SF_CS_HIGH();

    sf_WaitForWriteEnd();                           /* �ȴ�����Flash�ڲ�д������� */
    
}
    
/*
*********************************************************************************************************
*   �� �� ��: sf_ReadBuffer
*   ����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*   ��    ��:     _pBuf : ����Դ��������
*               _uiReadAddr ���׵�ַ
*               _usSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*   �� �� ֵ: ��
*********************************************************************************************************
*/
void sFLASH_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
    /* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
    if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tSF.TotalSize)
    {
        return;
    }

    /* ������������ */
    SF_CS_LOW();                                    /* ʹ��Ƭѡ */
    sf_SendByte(CMD_READ);                          /* ���Ͷ����� */
    sf_SendByte((_uiReadAddr & 0xFF0000) >> 16);    /* ����������ַ�ĸ�8bit */
    sf_SendByte((_uiReadAddr & 0xFF00) >> 8);       /* ����������ַ�м�8bit */
    sf_SendByte(_uiReadAddr & 0xFF);                /* ����������ַ��8bit */
    while (_uiSize--)
    {
        *_pBuf++ = sf_SendByte(DUMMY_BYTE);         /* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
    }
    SF_CS_HIGH();                                   /* ����Ƭѡ */
}

 /*
    *********************************************************************************************************
    *   �� �� ��: sf_AutoWritePage
    *   ����˵��: д1��PAGE��У��,�������ȷ������д���Ρ��������Զ���ɲ���������
    *   ��    ��:     _pBuf : ����Դ��������
    *               _uiWriteAddr ��Ŀ�������׵�ַ
    *               _usSize �����ݸ��������ܳ���ҳ���С
    *   �� �� ֵ: 0 : ���� 1 �� �ɹ�
    *********************************************************************************************************
    */
static uint8_t sf_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
    uint16_t i;
    uint16_t j;                 /* ������ʱ */
    uint32_t uiFirstAddr;       /* ������ַ */
    uint8_t ucNeedErase;        /* 1��ʾ��Ҫ���� */
    uint8_t cRet;

//        uint8_t s_spiBuf[4*1024] = {0}; 


    /* ����Ϊ0ʱ����������,ֱ����Ϊ�ɹ� */
    if (_usWrLen == 0)
    {
        return 1;
    }

    /* ���ƫ�Ƶ�ַ����оƬ�������˳� */
    if (_uiWrAddr >= g_tSF.TotalSize)
    {
        return 0;
    }

    /* ������ݳ��ȴ����������������˳� */
    if (_usWrLen > g_tSF.PageSize)
    {
        return 0;
    }

    /* ���FLASH�е�����û�б仯,��дFLASH */
    bsp_sf_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
    if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
    {
        return 1;
    }

    /* �ж��Ƿ���Ҫ�Ȳ������� */
    /* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
    ucNeedErase = 0;
    if (sf_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
    {
        ucNeedErase = 1;
    }

    uiFirstAddr = _uiWrAddr & (~(g_tSF.PageSize - 1));

    if (_usWrLen == g_tSF.PageSize)     /* ������������д */
    {
        for (i = 0; i < g_tSF.PageSize; i++)
        {
            s_spiBuf[i] = _ucpSrc[i];
        }
    }
    else                        /* ��д�������� */
    {
        /* �Ƚ��������������ݶ��� */
        bsp_sf_ReadBuffer(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

        /* ���������ݸ��� */
        i = _uiWrAddr & (g_tSF.PageSize - 1);
        memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
    }

    /* д��֮�����У�飬�������ȷ����д�����3�� */
    cRet = 0;
    for (i = 0; i < 3; i++)
    {

        /* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
        if (ucNeedErase == 1)
        {
            bsp_sf_EraseSector(uiFirstAddr);        /* ����1������ */
        }

        /* ���һ��PAGE */
        bsp_sf_PageWrite(s_spiBuf, uiFirstAddr, g_tSF.PageSize);

        if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
        {
            cRet = 1;
            break;
        }
        else
        {
            if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
            {
                cRet = 1;
                break;
            }

            /* ʧ�ܺ��ӳ�һ��ʱ�������� */
            for (j = 0; j < 10000; j++);
        }
    }

    return cRet;
}

    

/*
*********************************************************************************************************
*   �� �� ��: sf_WriteBuffer
*   ����˵��: д1��������У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*   ��    ��:     _pBuf : ����Դ��������
*               _uiWrAddr ��Ŀ�������׵�ַ
*               _usSize �����ݸ��������ܳ���ҳ���С
*   �� �� ֵ: 1 : �ɹ��� 0 �� ʧ��
*********************************************************************************************************
*/
uint8_t bsp_sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
    uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    /*mod�������࣬��writeAddr��SPI_FLASH_PageSize��������������AddrֵΪ0*/
    Addr = _uiWriteAddr % g_tSF.PageSize;
    
    /*��count������ֵ���պÿ��Զ��뵽ҳ��ַ*/
    count = g_tSF.PageSize - Addr;

    /*�����Ҫд��������ҳ*/
    NumOfPage =  _usWriteSize / g_tSF.PageSize;

    /*mod�������࣬�����ʣ�಻��һҳ���ֽ���*/
    NumOfSingle = _usWriteSize % g_tSF.PageSize;

    if (Addr == 0) /* ��ʼ��ַ��ҳ���׵�ַ  */
    {
        if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
        {
            if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
            {
                return 0;
            }
        }
        else    /* ���ݳ��ȴ��ڵ���ҳ���С */
        {
            while (NumOfPage--)
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
                {
                    return 0;
                }
                _uiWriteAddr +=  g_tSF.PageSize;
                _pBuf += g_tSF.PageSize;
            }
            if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
            {
                return 0;
            }
        }
    }
    else  /* ��ʼ��ַ����ҳ���׵�ַ  */
    {
        if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
        {
            if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
            {
                temp = NumOfSingle - count;

                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
                {
                    return 0;
                }

                _uiWriteAddr +=  count;
                _pBuf += count;

                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
                {
                    return 0;
                }
            }
            else
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
                {
                    return 0;
                }
            }
        }
        else    /* ���ݳ��ȴ��ڵ���ҳ���С */
        {
            _usWriteSize -= count;
            NumOfPage =  _usWriteSize / g_tSF.PageSize;
            NumOfSingle = _usWriteSize % g_tSF.PageSize;

            if (sf_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
            {
                return 0;
            }

            _uiWriteAddr +=  count;
            _pBuf += count;

            while (NumOfPage--)
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, g_tSF.PageSize) == 0)
                {
                    return 0;
                }
                _uiWriteAddr +=  g_tSF.PageSize;
                _pBuf += g_tSF.PageSize;
            }

            if (NumOfSingle != 0)
            {
                if (sf_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
                {
                    return 0;
                }
            }
        }
    }
    return 1;   /* �ɹ� */
}


#endif


#if  1
 /**
  * @brief  ��FLASH��ҳд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * @param	pBuffer��Ҫд�����ݵ�ָ��
  * @param WriteAddr��д���ַ
  * @param  NumByteToWrite��д�����ݳ��ȣ�����С�ڵ���SPI_FLASH_PerWritePageSize
  * @retval ��
  */
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
  /* ����FLASHдʹ������ */
  sFLASH_WriteEnable();

  /* ѡ��FLASH: CS�͵�ƽ */
  sFLASH_CS_LOW();
  /* дҳдָ��*/
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*����д��ַ�ĸ�λ*/
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  sFLASH_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
  }

  /* д������*/
  while (NumByteToWrite--)
  {
    /* ���͵�ǰҪд����ֽ����� */
    sFLASH_SendByte(*pBuffer);
    /* ָ����һ�ֽ����� */
    pBuffer++;
  }

  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  sFLASH_CS_HIGH();

  /* �ȴ�д�����*/
  sFLASH_WaitForWriteEnd();
}


 /**
  * @brief  ��FLASHд�����ݣ����ñ�����д������ǰ��Ҫ�Ȳ�������
  * @param	pBuffer��Ҫд�����ݵ�ָ��
  * @param  WriteAddr��д���ַ
  * @param  NumByteToWrite��д�����ݳ���
  * @retval ��
  */
void SPI_FLASH_BufferWrite(void *wBuf, u32 WriteAddr, u32 NumByteToWrite)
{
    
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  
  uint8_t *pBuffer = (uint8_t *)wBuf;
	
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
			/*��ǰҳʣ���count��λ�ñ�NumOfSingleС��д����*/
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
			/*��ַ����������count�ֿ������������������*/
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
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
void SPI_FLASH_BufferRead(void *rBuf, u32 ReadAddr, u32 NumByteToRead)
{

  uint8_t *pBuffer = (uint8_t *)rBuf; 
    
  /* ѡ��FLASH: CS�͵�ƽ */
  sFLASH_CS_LOW();

  /* ���� �� ָ�� */
  sFLASH_SendByte(sFLASH_CMD_READ);

  /* ���� �� ��ַ��λ */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* ���� �� ��ַ��λ */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* ���� �� ��ַ��λ */
  sFLASH_SendByte(ReadAddr & 0xFF);
  
	/* ��ȡ���� */
  while (NumByteToRead--)
  {
    /* ��ȡһ���ֽ�*/
    *pBuffer = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    /* ָ����һ���ֽڻ����� */
    pBuffer++;
  }

  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  sFLASH_CS_HIGH();
}


 /**
  * @brief  ����FLASH����
  * @param  SectorAddr��Ҫ������������ַ
  * @retval ��
  */
void SPI_FLASH_SectorErase(u32 WriteAddr,u32 NumByteToCls)
{

    /* ����FLASHдʹ������ */
    sFLASH_WriteEnable();
  
    /* ѡ��FLASH: CS�͵�ƽ */
    sFLASH_CS_LOW();
    /* дҳдָ��*/
    sFLASH_SendByte(sFLASH_CMD_WRITE);
    /*����д��ַ�ĸ�λ*/
    sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
    /*����д��ַ����λ*/
    sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    /*����д��ַ�ĵ�λ*/
    sFLASH_SendByte(WriteAddr & 0xFF);
  
    /* д������*/
    while (NumByteToCls--)
    {
      /* ���͵�ǰҪд����ֽ����� */
      sFLASH_SendByte(0x00);
    }
  
    /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
    sFLASH_CS_HIGH();
  
    /* �ȴ�д�����*/
    sFLASH_WaitForWriteEnd();


}



 /**
  * @brief  ����FLASH��������Ƭ����
  * @param  ��
  * @retval ��
  */
void SPI_FLASH_BulkErase(void)
{
  uint32_t NumByteToCls = 131072;
  /* ����FLASHдʹ������ */
  sFLASH_WriteEnable();
  
  /* ѡ��FLASH: CS�͵�ƽ */
  sFLASH_CS_LOW();
  /* дҳдָ��*/
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*����д��ַ�ĸ�λ*/
  sFLASH_SendByte((0 & 0xFF0000) >> 16);
  /*����д��ַ����λ*/
  sFLASH_SendByte((0 & 0xFF00) >> 8);
  /*����д��ַ�ĵ�λ*/
  sFLASH_SendByte(0 & 0xFF);
  
  /* д������*/
  while (NumByteToCls--)
  {
    /* ���͵�ǰҪд����ֽ����� */
    sFLASH_SendByte(0x00);
  }
  
  /* ֹͣ�ź� FLASH: CS �ߵ�ƽ */
  sFLASH_CS_HIGH();
  
  /* �ȴ�д�����*/
  sFLASH_WaitForWriteEnd();

}




#endif /* #if 0 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
