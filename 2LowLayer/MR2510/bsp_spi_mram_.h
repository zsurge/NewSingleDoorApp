/**
  ******************************************************************************
  * @file    bsp_spi_mram.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    13-April-2012
  * @brief   This file contains all the functions prototypes for the spi_flash
  *          firmware driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SPI_MRAM_H
#define __BSP_SPI_MRAM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* M25P SPI Flash supported commands */  
#define sFLASH_CMD_WRITE          0x02  /* Write to Memory instruction */
#define sFLASH_CMD_WRSR           0x01  /* Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /* Write enable instruction */
#define sFLASH_CMD_READ           0x03  /* Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /* Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x9F  /* Read identification */
#define sFLASH_CMD_SE             0xD8  /* Sector Erase instruction */
#define sFLASH_CMD_BE             0xC7  /* Bulk Erase instruction */

#define sFLASH_WIP_FLAG           0x01  /* Write In Progress (WIP) flag */

#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       0x100

#define sFLASH_M25P128_ID         0x202018
#define sFLASH_M25P64_ID          0x202017
  
/* M25P FLASH SPI Interface pins  */  
#define sFLASH_SPI                           SPI3
#define sFLASH_SPI_CLK                       RCC_APB1Periph_SPI3
#define sFLASH_SPI_CLK_INIT                  RCC_APB1PeriphClockCmd

#define sFLASH_SPI_SCK_PIN                   GPIO_Pin_10
#define sFLASH_SPI_SCK_GPIO_PORT             GPIOC
#define sFLASH_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOC
#define sFLASH_SPI_SCK_SOURCE                GPIO_PinSource10
#define sFLASH_SPI_SCK_AF                    GPIO_AF_SPI3

#define sFLASH_SPI_MISO_PIN                  GPIO_Pin_11
#define sFLASH_SPI_MISO_GPIO_PORT            GPIOC
#define sFLASH_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define sFLASH_SPI_MISO_SOURCE               GPIO_PinSource11
#define sFLASH_SPI_MISO_AF                   GPIO_AF_SPI3

#define sFLASH_SPI_MOSI_PIN                  GPIO_Pin_12
#define sFLASH_SPI_MOSI_GPIO_PORT            GPIOC
#define sFLASH_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define sFLASH_SPI_MOSI_SOURCE               GPIO_PinSource12
#define sFLASH_SPI_MOSI_AF                   GPIO_AF_SPI3

#define sFLASH_CS_PIN                        GPIO_Pin_8
#define sFLASH_CS_GPIO_PORT                  GPIOC
#define sFLASH_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOC

/* Exported macro ------------------------------------------------------------*/
/* Select sFLASH: Chip Select pin low */
#define sFLASH_CS_LOW()       GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/* Deselect sFLASH: Chip Select pin high */
#define sFLASH_CS_HIGH()      GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)   

/* Exported functions ------------------------------------------------------- */

/* High layer functions  */
void sFLASH_DeInit(void);
void sFLASH_Init(void);

uint32_t sFLASH_ReadStatuR(void);
void sFLASH_WriteStatuR(uint8_t byte);

/* Low layer functions */
uint8_t sFLASH_ReadByte(void);
uint8_t sFLASH_SendByte(uint8_t byte);
uint16_t sFLASH_SendHalfWord(uint16_t HalfWord);
void sFLASH_WriteEnable(void);
void sFLASH_WaitForWriteEnd(void);

 uint8_t sFLASH_Read(uint32_t ReadAddr);
void sFLASH_Write(uint32_t WriteAddr,uint8_t Wdata);



void sFLASH_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

void SPI_FLASH_BufferWrite(void *wBuf, u32 WriteAddr, u32 NumByteToWrite);

void SPI_FLASH_BufferRead(void *rBuf, u32 ReadAddr, u32 NumByteToRead);

void SPI_FLASH_SectorErase(u32 WriteAddr,u32 NumByteToCls);
void SPI_FLASH_BulkErase(void);



#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

