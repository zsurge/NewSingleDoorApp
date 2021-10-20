/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : DataProcess_Task.c
  �� �� ��   : ����
  ��    ��   :
  ��������   : 2020��5��15��
  ����޸�   :
  ��������   : ��ˢ��/QR/Զ���͹��������ݽ��д���
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��5��15��
    ��    ��   :
    �޸�����   : �����ļ�

******************************************************************************/
/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "DataProcess"
#include "elog.h"

#include "DataProcess_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "jsonUtils.h"
#include "malloc.h"
#include "tool.h"
#include "bsp_time.h"
#include "bsp_dipSwitch.h"
#include "bsp_led.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 6)
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*12)

#define SORT_FLAG

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char* dataProcTaskName = "vDataProcTask";

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDataProc = NULL;
CMD_BUFF_STRU gCmd_buff = {0};

//static void getCard(uint8_t *src);


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

static void vTaskDataProcess ( void* pvParameters );

void CreateDataProcessTask ( void )
{
	xTaskCreate ( ( TaskFunction_t ) vTaskDataProcess,
	              ( const char*    ) dataProcTaskName,
	              ( uint16_t       ) DATAPROC_STK_SIZE,
	              ( void*          ) NULL,
	              ( UBaseType_t    ) DATAPROC_TASK_PRIO,
	              ( TaskHandle_t*  ) &xHandleTaskDataProc );
}


static void vTaskDataProcess ( void* pvParameters )
{
	BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	BaseType_t devReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS ( 50 ); /* �������ȴ�ʱ��Ϊ100ms */
	int ret = 0;
	int len = 0;

	uint8_t jsonbuff[256] = {0};
	uint32_t devID = 0;
	READER_BUFF_STRU* ptMsg  = &gReaderRecvMsg;
	LOCAL_USER_STRU localUserData ;

	uint8_t openLeft[8] = { 0x02,0x00,0x07,0x01,0x06,0x4c,0x03,0x4D };
	uint8_t openRight[8] = { 0x02,0x00,0x07,0x01,0x06,0x52,0x03,0x53 };
	CMD_BUFF_STRU* ptCmd = &gCmd_buff;
	memset ( &gCmd_buff,0x00,sizeof ( CMD_BUFF_STRU ) );
	memset ( &gReaderRecvMsg,0x00,sizeof ( READER_BUFF_STRU ) );

	while ( 1 )
	{
	    LEDERROR = !LEDERROR; 
		//�����·���ɺ�30���޿����·���������һ���������
		if ( gCardSortTimer.flag && gCardSortTimer.outTimer == 0 )
		{
			log_d ( "-----start sort-----\r\n" );
			gCardSortTimer.flag = 0;

#ifdef SORT_FLAG
			sortLastPageCard();
#endif
		}

		//��ȡ���������������ݣ�����������ʷ��¼
//        while(gRecordIndex.accessRecoIndex-- > 0) һ��һ�����ͣ���������ȫ������
		if ( gRecordIndex.accessRecoIndex > 0 && gConnectStatus==1 )
		{
			len = readRecord ( jsonbuff );
			if ( len < RECORD_MAX_LEN && len > 0 )
			{
				mqttSendData ( jsonbuff,len );
			}
		}

		xReturn = xQueueReceive ( xCardIDQueue,   /* ��Ϣ���еľ�� */
		                          ( void* ) &ptMsg, /*�����ȡ���ǽṹ��ĵ�ַ */
		                          xMaxBlockTime ); /* ��������ʱ�� */
        if(pdTRUE != xReturn)
        {
            continue;
        }


			//��Ϣ���ճɹ������ͽ��յ�����Ϣ
			log_d ( "cardid %02x,%02x,%02x,%02x,devid = %d,mode = %d\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3],ptMsg->devID,ptMsg->mode );
            log_d ("xQueueReceive = %s\r\n",ptMsg->cardID);;

        switch(ptMsg->mode)
        {
            case SORT_CARD_MODE:
			//����			
#ifdef SORT_FLAG
				sortPageCard();
#endif
            break;
			case MANUAL_SORT:
				//����������ݽ�������
#ifdef SORT_FLAG
				manualSortCard();
#endif
            break;
			case DEL_CARD_MODE:		
				ret = delHead ( ptMsg->cardID,CARD_MODE );
				log_d ( "delHead = %d\r\n",ret );

				if ( ret != 1 )
				{
					//1.ɾ���û�ʧ��

				}
			break;
			 //����
			case READMODE:	
				//memcpy(ptMsg->cardID,"\x00\xfb\x4b\xfb",4);
				log_d ( "test cardid %02x,%02x,%02x,%02x\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3]);

				ret = readHead ( ptMsg->cardID, CARD_MODE );
				log_d ( "readHead = %d\r\n",ret );	
				
				if ( ret != NO_FIND_HEAD )
				{
					log_d ( "read card success\r\n" );

					//if(gDevBaseParam.progamMode == PROGRAMMODE_CHANNEL)
					if ( DIP0 == 0 )
					{
						ptCmd->cmd_len = 8;

						if ( ptMsg->devID == 1 )
						{
							memcpy ( ptCmd->cmd,openLeft,ptCmd->cmd_len );
						}
						else
						{
							memcpy ( ptCmd->cmd,openRight,ptCmd->cmd_len );
						}

						/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
						devReturn = xQueueSend ( xCmdQueue,           /* ��Ϣ���о�� */
						                         ( void* ) &ptCmd,            /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						                         ( TickType_t ) 30 );
					}
					//else if(gDevBaseParam.progamMode == PROGRAMMODE_DOOR)
					else if ( DIP0 == 1 )
					{
						devID = ptMsg->devID;

						if ( devID == READER1 )
						{
							gOpenDoorTimer.flag = 1;
							gOpenDoorTimer.outTimer = 12000;
						}
						else if ( devID == READER2 )
						{
							gOpenDoor2Timer.flag = 1;
							gOpenDoor2Timer.outTimer = 12000;
						}

						devReturn = xQueueSend ( xCmdQueue,           /* ��Ϣ���о�� */
						                         ( void* ) &devID,            /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						                         ( TickType_t ) 30 );
					}				

					if ( devReturn != pdPASS )
					{
						xQueueReset ( xCmdQueue );
						log_d ( "send card2  queue is error!\r\n" );
						//���Ϳ���ʧ�ܷ�������ʾ
						//�����Ƕ�����
					}
					else
					{
						log_d ( "2 cardid %02x,%02x,%02x,%02x,devid = %d\r\n",ptMsg->cardID[0],ptMsg->cardID[1],ptMsg->cardID[2],ptMsg->cardID[3],ptMsg->devID );
						//�������
						packetCard ( ptMsg->cardID, jsonbuff );

						//�������ݵ�MQTT������
						len = strlen ( ( const char* ) jsonbuff );
						log_d ( "send = %d,buff = %s\r\n",len,jsonbuff );

						len = mqttSendData ( jsonbuff,len );

						//�����ж�mqttSendData�Ƿ�Ϊ0�����ǣ�����Ҫд�뵽FLASH��ȥ
						if ( len == 0 )
						{
							//д��¼��FLASH
							writeRecord ( jsonbuff,  strlen ( ( const char* ) jsonbuff ) );
						}
					}
				}
				else
				{
					log_d ( "read card error: not find card\r\n" );
                    //if(gDevBaseParam.progamMode == PROGRAMMODE_TEST)
                    if(DIP3 == 0)
					{
                        devID = ptMsg->devID;
                        gOpenDoorTimer.flag = 1;
                        gOpenDoorTimer.outTimer = 12000;
                        gOpenDoor2Timer.flag = 1;
                        gOpenDoor2Timer.outTimer = 12000;                        
					
						devReturn = xQueueSend ( xCmdQueue,           /* ��Ϣ���о�� */
						                         ( void* ) &devID,            /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
						                         ( TickType_t ) 30 );					    
//					    getCard(ptMsg->cardID);
					}					
				}
		    break;
		    
		    //Զ�̿���
			case REMOTE_OPEN_MODE:
			
				//���Ϳ���ָ��

				log_d ( "read card success\r\n" );

				//if (gDevBaseParam.progamMode == PROGRAMMODE_CHANNEL)
				if ( DIP0 == 0 )
				{
					ptCmd->cmd_len = 8;
					memcpy ( ptCmd->cmd,openLeft,ptCmd->cmd_len );
					/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
					devReturn = xQueueSend ( xCmdQueue,                     /* ��Ϣ���о�� */
					                         ( void* ) &ptCmd,              /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
					                         ( TickType_t ) 30 );
				}
				//else if(gDevBaseParam.progamMode == PROGRAMMODE_DOOR)
				else if ( DIP0 == 1 )
				{
                    devID = ptMsg->devID;
                    
                    if ( devID == READER1 )
                    {
                        gOpenDoorTimer.flag = 1;
                        gOpenDoorTimer.outTimer = 12000;
                    }
                    else if ( devID == READER2 )
                    {
                        gOpenDoor2Timer.flag = 1;
                        gOpenDoor2Timer.outTimer = 12000;
                    }
                    
                    devReturn = xQueueSend ( xCmdQueue,           /* ��Ϣ���о�� */
                                             ( void* ) &devID,            /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
                                             ( TickType_t ) 30 );
				}
				else if ( DIP3 == 0 )
				//else if(gDevBaseParam.progamMode == PROGRAMMODE_TEST)
				{
					devID = 1;
                    gOpenDoorTimer.flag = 1;
                    gOpenDoorTimer.outTimer = 12000;
                    gOpenDoor2Timer.flag = 1;
                    gOpenDoor2Timer.outTimer = 12000;  

					devReturn = xQueueSend ( xCmdQueue,                 /* ��Ϣ���о�� */
					                         ( void* ) &devID,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
					                         ( TickType_t ) 30 );				
				}

				if ( devReturn != pdPASS )
				{
					xQueueReset ( xCmdQueue );
					log_d ( "send card2  queue is error!\r\n" );
					//���Ϳ���ʧ�ܷ�������ʾ
					//�����Ƕ�����
				}
			break;
			case AUTH_MODE_QR:						    
			    ret = parseQrCode(ptMsg->cardID,&localUserData);
                if(ret == NO_ERR)
                {
                    log_d("not find record\r\n");                    
                }

				devID = ptMsg->devID;

				if ( devID == READER1 )
				{
					gOpenDoorTimer.flag = 1;
					gOpenDoorTimer.outTimer = 12000;
				}
				else if ( devID == READER2 )
				{
					gOpenDoor2Timer.flag = 1;
					gOpenDoor2Timer.outTimer = 12000;
				}

				devReturn = xQueueSend ( xCmdQueue,           /* ��Ϣ���о�� */
				                         ( void* ) &devID,            /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
				                         ( TickType_t ) 30 );                

                localUserData.authMode = ptMsg->mode;               
                
                //1.���
                packetRespQr(&localUserData,jsonbuff); 
                len = strlen((const char*)jsonbuff);
                
                //2.����������
                len = mqttSendData(jsonbuff,len);
                log_d("send = %d\r\n",len);  
                
			break;
		}


		/* �����¼���־����ʾ������������ */
		xEventGroupSetBits ( xCreatedEventGroup, TASK_BIT_3 );
		vTaskDelay ( 30 );

	}

}


//void getCard(uint8_t *src)
//{
//    char buf[128] = {0};
//    
//    uint8_t i = 0;
//    uint8_t crc = 0;   

//    i = 3;
//    buf[0] = 0xA5;
//    buf[i++] = 0x0C;
//    buf[i++] = 0x00;

//    memcpy(buf+i,src,4);
//    
//    i+= 4;   
//    
//    buf[i++] = 0x5A;
//    
//    buf[1] = i>>8;   //high
//    buf[2] = i&0xFF; //low   
//    
//    crc = xorCRC((uint8_t *)buf, i);  
//    buf[i++] = crc; 


//    dbh("txBuf", buf, i);

//   RS485_SendBuf(COM2,(uint8_t *)buf,i);  
//    
//}



