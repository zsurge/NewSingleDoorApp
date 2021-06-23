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
#define MRAM_CMD_WRITE          0x02  /* Write to Memory instruction */
#define MRAM_CMD_WRSR           0x01  /* Write Status Register instruction */
#define MRAM_CMD_WREN           0x06  /* Write enable instruction */
#define MRAM_CMD_READ           0x03  /* Read from Memory instruction */
#define MRAM_CMD_RDSR           0x05  /* Read Status Register instruction  */
#define MRAM_CMD_RDID           0x9F  /* Read identification */
#define MRAM_CMD_SE             0xD8  /* Sector Erase instruction */
#define MRAM_CMD_BE             0xC7  /* Bulk Erase instruction */

#define MRAM_WIP_FLAG           0x01  /* Write In Progress (WIP) flag */

#define MRAM_DUMMY_BYTE         0xA5
#define MRAM_SPI_PAGESIZE       0x100

#define MRAM_M25P128_ID         0x202018
#define MRAM_M25P64_ID          0x202017
  
/* M25P FLASH SPI Interface pins  */  
#define MRAM_SPI                           SPI3
#define MRAM_SPI_CLK                       RCC_APB1Periph_SPI3
#define MRAM_SPI_CLK_INIT                  RCC_APB1PeriphClockCmd
                                           
#define MRAM_SPI_SCK_PIN                   GPIO_Pin_10
#define MRAM_SPI_SCK_GPIO_PORT             GPIOC
#define MRAM_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOC
#define MRAM_SPI_SCK_SOURCE                GPIO_PinSource10
#define MRAM_SPI_SCK_AF                    GPIO_AF_SPI3
                                           
#define MRAM_SPI_MISO_PIN                  GPIO_Pin_11
#define MRAM_SPI_MISO_GPIO_PORT            GPIOC
#define MRAM_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define MRAM_SPI_MISO_SOURCE               GPIO_PinSource11
#define MRAM_SPI_MISO_AF                   GPIO_AF_SPI3
                                           
#define MRAM_SPI_MOSI_PIN                  GPIO_Pin_12
#define MRAM_SPI_MOSI_GPIO_PORT            GPIOC
#define MRAM_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define MRAM_SPI_MOSI_SOURCE               GPIO_PinSource12
#define MRAM_SPI_MOSI_AF                   GPIO_AF_SPI3
                                           
#define MRAM_CS_PIN                        GPIO_Pin_8
#define MRAM_CS_GPIO_PORT                  GPIOC
#define MRAM_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOC

/* Exported macro ------------------------------------------------------------*/
/* Select MRAM: Chip Select pin low */
#define MRAM_CS_LOW()       GPIO_ResetBits(MRAM_CS_GPIO_PORT, MRAM_CS_PIN)
/* Deselect MRAM: Chip Select pin high */
#define MRAM_CS_HIGH()      GPIO_SetBits(MRAM_CS_GPIO_PORT, MRAM_CS_PIN)   

/* Exported functions ------------------------------------------------------- */

/* High layer functions  */
void bsp_MRAM_DeInit(void);
void bsp_MRAM_Init(void);

uint32_t bsp_MRAM_ReadStatuR(void);
void bsp_MRAM_WriteStatuR(uint8_t byte);

/* Low layer functions */
uint8_t bsp_MRAM_ReadByte(void);
uint8_t bsp_MRAM_SendByte(uint8_t byte);
uint16_t bsp_MRAM_SendHalfWord(uint16_t HalfWord);
void bsp_MRAM_WriteEnable(void);
void bsp_MRAM_WaitForWriteEnd(void);

uint8_t bsp_MRAM_Read(uint32_t ReadAddr);
void bsp_MRAM_Write(uint32_t WriteAddr,uint8_t Wdata);

void bsp_MRAM_BufferWrite(void *wBuf,u32 WriteAddr,  u32 NumByteToWrite);
void bsp_MRAM_BufferRead(void *rBuf, u32 ReadAddr, u32 NumByteToRead);

void bsp_MRAM_SectorErase(u32 WriteAddr,u32 NumByteToCls);
void bsp_MRAM_BulkErase(void);



#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
