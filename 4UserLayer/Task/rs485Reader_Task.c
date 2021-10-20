/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : rs485Reader_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��9��29��
  ����޸�   :
  ��������   : 485����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��9��29��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/
 
#define RS485READER_TASK_PRIO		(tskIDLE_PRIORITY + 1)
#define RS485READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *rS485ReaderTaskName = "vRS485ReadTask";  

typedef struct FROMREADER
{
    uint8_t rxBuff[512];               //�����ֽ���    
    uint8_t rxStatus;                   //����״̬
    uint16_t rxCnt;                     //�����ֽ���
}FROMREADER_STRU;

typedef union
{
	uint32_t id;        //����
	uint8_t sn[4];    //���Ű��ַ�
}CARD_TYPE;


static FROMREADER_STRU gReaderData;


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskRs485Reader = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskRs485Reader(void *pvParameters);
static uint8_t parseReader(COM_PORT_E port);
static uint16_t parseCardId(char type,char *src,char *cardId,uint8_t *mode);
static void reverseArray(uint8_t *array) ;



void CreateRs485ReaderTask(void)
{
    //��ȡ�������ݲ�����
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
        uint8_t sendBuff[512] = {0};
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
                log_d("0 recv buff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt);
                    
                len = parseCardId(READER1,(char *)gReaderData.rxBuff,(char *)sendBuff,&tmpValue);
                
                memset(&gReaderData,0x00,sizeof(FROMREADER_STRU));

                log_d("1 recv buff = %s,len = %d\r\n",sendBuff,len);
                
                if(len == 8)
                { 
                    memcpy(sendBuff,"00",2);
                    sscanf((const char*)sendBuff,"%8x",&tmpID);

                    
                    log_d("sendBuff = %s\r\n",sendBuff);
                    
                    /* ���� */
                    ptReaderBuf->devID = 0; 
                    ptReaderBuf->mode = 0;
                    memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
                    Sound2(50);

                    cardDev1.id = tmpID;
                    reverseArray(cardDev1.sn);
                    
                    ptReaderBuf->devID = READER1; 
                    ptReaderBuf->mode = READMODE;
                    memcpy(ptReaderBuf->cardID,cardDev1.sn,sizeof(cardDev1.sn));   


                    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
                    if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
                               (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
                               (TickType_t)10) != pdPASS )
                    {
                    //                xQueueReset(xCardIDQueue); ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
                      log_d("send card1  queue is error!\r\n"); 
                      //���Ϳ���ʧ�ܷ�������ʾ
                      //�����Ƕ�����                
                    }
                }                
                else
                {                    
                    /* ���� */
                    ptReaderBuf->devID = 0; 
                    ptReaderBuf->mode = 0;
                    memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
                    Sound2(50);

                    ptReaderBuf->devID = READER1; 
                    ptReaderBuf->mode = AUTH_MODE_QR;
                    memcpy(ptReaderBuf->cardID,sendBuff,len);   


                    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
                    if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
                               (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
                               (TickType_t)10) != pdPASS )
                    {
                    //                xQueueReset(xCardIDQueue); ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
                      log_d("send card1  queue is error!\r\n"); 
                      //���Ϳ���ʧ�ܷ�������ʾ
                      //�����Ƕ�����                
                    }                    
                }
            }

            
            //���֤
            memset(sendBuff,0x00,sizeof(sendBuff));            
            if(parseReader(COM2) == FINISHED)
            {
                len = parseCardId(READER2,(char *)gReaderData.rxBuff,(char *)sendBuff,&tmpValue);
                
                memset(&gReaderData,0x00,sizeof(FROMREADER_STRU));

                log_d("2 recv buff = %s,len = %d\r\n",sendBuff,len);

                
                if(len == 8)
                {    

                    sscanf((const char*)sendBuff,"%8x",&tmpID);
                    
                    log_d("sendBuff = %s,tmpID = %d\r\n",sendBuff,tmpID);
                    
                    /* ���� */
                    ptReaderBuf->devID = 0; 
                    ptReaderBuf->mode = 0;
                    memset(ptReaderBuf->cardID,0x00,sizeof(ptReaderBuf->cardID));  
                    Sound2(50);

                    cardDev2.id = tmpID;
                    reverseArray(cardDev2.sn);
                    
                    ptReaderBuf->devID = READER2; 
                    ptReaderBuf->mode = AUTH_MODE_ID;
                    memcpy(ptReaderBuf->cardID,cardDev2.sn,sizeof(cardDev2.sn));   


                    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
                    if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
                               (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
                               (TickType_t)10) != pdPASS )
                    {
                    //                xQueueReset(xCardIDQueue); ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
                      log_d("send card1  queue is error!\r\n"); 
                      //���Ϳ���ʧ�ܷ�������ʾ
                      //�����Ƕ�����                
                    }
                }
            }           
            
        	/* �����¼���־����ʾ������������ */        
        	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);   

            vTaskDelay(100); 
        }

}

static uint8_t parseReader(COM_PORT_E port)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //��ȡ485���ݣ�����û�������˳������ض�
        if(!RS485_Recv(port,&ch,1))
        {            
            return UNFINISHED;
        }
        
        //��ȡ���������ݵ�BUFF
        gReaderData.rxBuff[gReaderData.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //���һ���ֽ�Ϊ�س��������ܳ���Ϊ510�󣬽�����ȡ
        if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A || gReaderData.rxCnt >=510)
        {   
            
           if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //��û�������һ���ֽڣ������ܳ���λ����գ��ض�������
            memset(&gReaderData,0xFF,sizeof(FROMREADER_STRU));
        }
        
    }   

   
}

static uint16_t parseCardId(char type,char *src,char *cardId,uint8_t *mode)
{
    uint16_t srcLen = 0;
    uint16_t dataLen = 0;   
    
    if(!src)
    {
        return 0;
    }
    
    //ȥ��0D0A
    if(strlen(src)-2 >QR_MAX_BUF_LEN)
    {
        srcLen = QR_MAX_BUF_LEN;
    }
    else
    {
        srcLen = strlen(src)-2;
    }

    if(type == 1)
    {
        //�ж���ˢ������QR
        if(strstr_t((const char*)src,(const char*)"CARD") == NULL)
        {
            *mode = AUTH_MODE_QR;   
            dataLen = srcLen;
            memcpy(cardId,src,dataLen);
        }
        else
        {

            dataLen = 8;//���ų���Ϊ8
            *mode = AUTH_MODE_CARD;  
            //100830576206
            memcpy(cardId,src + 9,dataLen);
            
        }
    }
    else if(type == 2 )
    {
        if(src[1] == 1) //��ͨ IC��
        {
            dataLen = 8;//���ų���Ϊ8
            *mode = AUTH_MODE_CARD;  
            //100830576206
            memcpy(cardId,src + srcLen-8,dataLen);     
        }
        else if(src[1] == 2) //���֤��
        {
            dataLen = 8;//���ų���Ϊ8
            *mode = AUTH_MODE_ID;  
            //201011D3D0020DC2281A
            memcpy(cardId,src + srcLen-8,dataLen);        
        }
    }
    

    return dataLen;
    
}


