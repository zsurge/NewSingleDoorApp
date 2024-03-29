/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : rs485Reader_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年9月29日
  最近修改   :
  功能描述   : 485读卡器任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年9月29日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

#define LOG_TAG    "BarCode"
#include "elog.h"

#include "stdio.h"
#include "bsp_uart_fifo.h"
#include "rs485Reader_Task.h"
#include "CmdHandle.h"
#include "bsp_beep.h"
#include "tool.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define RS485READER_TASK_PRIO		(tskIDLE_PRIORITY + 1)
#define RS485READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *rS485ReaderTaskName = "vRS485ReadTask";  

typedef struct FROMREADER
{
    uint8_t rxBuff[32];               //接收字节数    
    uint8_t rxStatus;                   //接收状态
    uint16_t rxCnt;                     //接收字节数
}FROMREADER_STRU;

typedef union
{
	uint32_t id;        //卡号
	uint8_t sn[4];    //卡号按字符
}CARD_TYPE;


static FROMREADER_STRU gReaderData;


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskRs485Reader = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskRs485Reader(void *pvParameters);
static uint8_t parseReader(COM_PORT_E port);
static uint16_t parseCardId(char *src,char *cardId,uint8_t *mode);
static void reverseArray(uint8_t *array) ;



void CreateRs485ReaderTask(void)
{
    //读取条码数据并处理
    xTaskCreate((TaskFunction_t )vTaskRs485Reader,     
                (const char*    )rS485ReaderTaskName,   
                (uint16_t       )RS485READER_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )RS485READER_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskRs485Reader);
}

static void reverseArray(uint8_t *array) 
{
    int i,temp;
    int size = sizeof(array)/sizeof(array[0]);

    
    for(i=0; i<size/2; i++)
    {
        temp = array[i];
        array[i] = array[size-i-1];
        array[size-i-1] = temp;
    }   

}

static void vTaskRs485Reader(void *pvParameters)
{ 
        uint8_t sendBuff[32] = {0};
        uint16_t len = 0; 
        uint8_t tmpValue;
        CARD_TYPE cardDev1,cardDev2;
        READER_BUFF_STRU *ptReaderBuf = &gReaderMsg;     
        uint32_t tmpID = 0;

        log_d("485 Reader Start\r\n");
        
        while(1)
        { 
            memset(sendBuff,0x00,sizeof(sendBuff));            
            if(parseReader(COM3) == FINISHED)
            {
                len = parseCardId((char *)gReaderData.rxBuff,(char *)sendBuff,&tmpValue);
                
                memset(&gReaderData,0x00,sizeof(FROMREADER_STRU));

                log_d("recv buff = %s,len = %d\r\n",sendBuff,len);
                
                if(len == 8)
                {    

                    memcpy(sendBuff,"00",2);
                    sscanf((const char*)sendBuff,"%8x",&tmpID);

                    
                    log_d("sendBuff = %s\r\n",sendBuff);
                    
                    /* 清零 */
                    ptReaderBuf->devID = 0; 
                    ptReaderBuf->mode = 0;
                    memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
                    Sound2(50);

                    cardDev1.id = tmpID;
                    reverseArray(cardDev1.sn);
                    
                    ptReaderBuf->devID = READER1; 
                    ptReaderBuf->mode = READMODE;
                    memcpy(ptReaderBuf->cardID,cardDev1.sn,sizeof(cardDev1.sn));   


                    /* 使用消息队列实现指针变量的传递 */
                    if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
                               (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
                               (TickType_t)10) != pdPASS )
                    {
                    //                xQueueReset(xCardIDQueue); 删除该句，为了防止在下发数据的时候刷卡
                      log_d("send card1  queue is error!\r\n"); 
                      //发送卡号失败蜂鸣器提示
                      //或者是队列满                
                    }
                }
            }

            memset(sendBuff,0x00,sizeof(sendBuff));            
            if(parseReader(COM2) == FINISHED)
            {
                len = parseCardId((char *)gReaderData.rxBuff,(char *)sendBuff,&tmpValue);
                
                memset(&gReaderData,0x00,sizeof(FROMREADER_STRU));

                log_d("recv buff = %s,len = %d\r\n",sendBuff,len);
                
                if(len == 8)
                {    

                    memcpy(sendBuff,"00",2);
                    sscanf((const char*)sendBuff,"%8x",&tmpID);

                    
                    log_d("sendBuff = %s\r\n",sendBuff);
                    
                    /* 清零 */
                    ptReaderBuf->devID = 0; 
                    ptReaderBuf->mode = 0;
                    memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
                    Sound2(50);

                    cardDev2.id = tmpID;
                    reverseArray(cardDev2.sn);
                    
                    ptReaderBuf->devID = READER2; 
                    ptReaderBuf->mode = READMODE;
                    memcpy(ptReaderBuf->cardID,cardDev2.sn,sizeof(cardDev2.sn));   


                    /* 使用消息队列实现指针变量的传递 */
                    if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
                               (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
                               (TickType_t)10) != pdPASS )
                    {
                    //                xQueueReset(xCardIDQueue); 删除该句，为了防止在下发数据的时候刷卡
                      log_d("send card1  queue is error!\r\n"); 
                      //发送卡号失败蜂鸣器提示
                      //或者是队列满                
                    }
                }
            }           
            
        	/* 发送事件标志，表示任务正常运行 */        
        	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);   

            vTaskDelay(100); 
        }

}

static uint8_t parseReader(COM_PORT_E port)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //读取485数据，若是没读到，退出，再重读
        if(!RS485_Recv(port,&ch,1))
        {            
            return UNFINISHED;
        }
        
        //读取缓冲区数据到BUFF
        gReaderData.rxBuff[gReaderData.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //最后一个字节为回车，或者总长度为510后，结束读取
        if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A || gReaderData.rxCnt >=32)
        {   
            
           if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //若没读到最后一个字节，但是总长到位后，清空，重读缓冲区
            memset(&gReaderData,0xFF,sizeof(FROMREADER_STRU));
        }
        
    }   

   
}

static uint16_t parseCardId(char *src,char *cardId,uint8_t *mode)
{
    uint16_t srcLen = 0;
    uint16_t dataLen = 0;
    
    
    if(!src)
    {
        return 0;
    }
    
    //去掉0D0A
    if(strlen(src)-2 >QUEUE_BUF_LEN)
    {
        srcLen = QUEUE_BUF_LEN;
    }
    else
    {
        srcLen = strlen(src)-2;
    }

        //判定是刷卡还是QR
    if(strstr_t((const char*)src,(const char*)"CARD") == NULL)
    {
        *mode = AUTH_MODE_QR;   
        dataLen = srcLen;
        memcpy(cardId,src,dataLen);
    }
    else
    {
        dataLen = 8;//卡号长度为8
        *mode = AUTH_MODE_CARD;  
        //CARD 120065AA89000000000 所以offset = 7
        memcpy(cardId,src + 9,dataLen);        
    }

    return dataLen;
    
}


