/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ��������ָ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CmdHandle"
#include "elog.h"	

#include "cmdhandle.h"
#include "tool.h"
#include "bsp_led.h"
#include "malloc.h"
#include "ini.h"
#include "bsp_uart_fifo.h"
#include "version.h"
#include "easyflash.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"
#include "version.h"
#include "eth_cfg.h"
#include "bsp_ds1302.h"
#include "LocalData.h"
#include "deviceInfo.h"
#include "bsp_time.h"

					



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0])) //�������鳤��


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
    

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
int gConnectStatus = 0;
int	gMySock = 0;
uint8_t gUpdateDevSn = 0; 

//static uint16_t ggcnt = 0;
//static uint16_t ggsum = 0;




READER_BUFF_STRU gReaderMsg;
READER_BUFF_STRU gReaderRecvMsg;

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode);
static SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf ); //����
static SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf ); //Զ�̱���
static SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf ); //��ӿ���
static SYSERRORCODE_E DelCardNoAll ( uint8_t* msgBuf ); //ɾ������
static SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf ); //���豸��������
static SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf ); //����Ӧ��
static SYSERRORCODE_E EnableDev ( uint8_t* msgBuf ); //�����豸
//static SYSERRORCODE_E DisableDev ( uint8_t* msgBuf ); //�ر��豸
static SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf ); //��ȡ�豸��Ϣ
static SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf ); //��ȡģ�����
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf ); //��ȡģ�����
static SYSERRORCODE_E DownLoadCardID ( uint8_t* msgBuf ); //��ȡ�û���Ϣ
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf ); //Զ�̺���
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf ); //ɾ���û���Ϣ
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf ); //���ñ���ʱ��
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf ); //���ñ���SN��MQTT��
static SYSERRORCODE_E DelCardSingle( uint8_t* msgBuf ); //ɾ������
static SYSERRORCODE_E getRemoteTime ( uint8_t* msgBuf );//��ȡԶ�̷�����ʱ��
static SYSERRORCODE_E RemoteResetDev ( uint8_t* msgBuf );//Զ������

//static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ); //����Ĭ����Ϣ

static SYSERRORCODE_E upMac( uint8_t* msgBuf );//�ϴ�MAC��ַ
static SYSERRORCODE_E downPubKey( uint8_t* msgBuf );//ƽ̨�·���Ԫ����Ϣ




typedef SYSERRORCODE_E ( *cmd_fun ) ( uint8_t *msgBuf ); 

typedef struct
{
	const char* cmd_id;             /* ����id */
	cmd_fun  fun_ptr;               /* ����ָ�� */
} CMD_HANDLE_T;


const CMD_HANDLE_T CmdList[] =
{
    {"10011",  OpenDoor},
	{"1006", AbnormalAlarm},
	{"1012", AddCardNo},
	{"1053", DelCardNoAll},
	{"10006", UpgradeDev},
	{"1017", UpgradeAck},
	{"1026", GetDevInfo},  
	{"10005", DelCardSingle},  
	{"10010", RemoteResetDev}, 
	{"30001", SetLocalSn},	
	{"30003", downPubKey},
	{"30004", upMac},
    {"3002", GetServerIp},
    {"10001", GetTemplateParam},
    {"10004", DownLoadCardID},   
    {"10031", RemoteOptDev},        
    {"10003", ClearUserInof},   
    {"10002", EnableDev}, //ͬ��
    {"88888", SetLocalTime},
    {"88881", getRemoteTime}
};




SYSERRORCODE_E exec_proc ( char* cmd_id, uint8_t *msg_buf )
{
	SYSERRORCODE_E result = NO_ERR;
	int i = 0;

    if(cmd_id == NULL)
    {
        log_d("empty cmd \r\n");
        return CMD_EMPTY_ERR; 
    }

	for ( i = 0; i < DIM ( CmdList ); i++ )
	{
		if ( 0 == strcmp ( CmdList[i].cmd_id, cmd_id ) )
		{
			CmdList[i].fun_ptr ( msg_buf );
			return result;
		}
	}
	log_d ( "invalid id %s\n", cmd_id );

    
//    ReturnDefault(msg_buf);
	return result;
}


void Proscess(void* data)
{
    char cmd[8+1] = {0};
    log_d("Start parsing JSON data\r\n");
    
    strcpy(cmd,(const char *)GetJsonItem ( data, ( const uint8_t* ) "commandCode",0 ));  

    log_d("-----commandCode = %s-----\r\n",cmd);
    
    exec_proc (cmd ,data);
}

static SYSERRORCODE_E RemoteResetDev ( uint8_t* msgBuf )//Զ������
{

    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }
    
    uint8_t *buf = packetBaseJson(msgBuf,1);

    if(buf == NULL)
    {
        return STR_EMPTY_ERR;
    }
    
    len = strlen((const char*)buf);

    log_d("RemoteResetDev = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);    
     
    NVIC_SystemReset(); 
    
    return NO_ERR;
}

//�ϴ�MAC��ַ
static SYSERRORCODE_E upMac( uint8_t* msgBuf )
{
    uint8_t buf[64] = {0};
    uint8_t bufLen = 0;
    sprintf((char*)buf,"{\"commandCode\":\"30004\",\"mac\":\"%s\"}",gDevBaseParam.deviceCode.deviceSn);
    bufLen = strlen((const char*)buf);

    log_d("upMac = %s,len = %d\r\n",buf,bufLen);
    
    //mqttSendData(buf,bufLen);  
    
    return NO_ERR;
}


static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode)
{
    SYSERRORCODE_E result = NO_ERR;

    READER_BUFF_STRU *ptMsg = &gReaderMsg;
    
	/* ���� */
    ptMsg->mode = authMode; 
    memset(ptMsg->cardID,0x00,sizeof(ptMsg->cardID)); 
    memcpy(ptMsg->cardID,buf,len);

    
    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    if(xQueueSend(xCardIDQueue,              /* ��Ϣ���о�� */
                 (void *) &ptMsg,   /* ����ָ�����recv_buf�ĵ�ַ */
                 (TickType_t)50) != pdPASS )
    {
        log_d("the queue is full!\r\n");                
        xQueueReset(xCardIDQueue);
        result = QUEUE_FULL_ERR;
    } 
    else
    {
        dbh("SendToQueue",(char *)buf,len);
    } 


    return result;
}


int mqttSendData(uint8_t *payload_out,uint16_t payload_out_len)
{   
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[1280];
	int buflen = sizeof(buf);

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

    if(!payload_out)
    {
        return STR_EMPTY_ERR;
    }


   if(gConnectStatus == 1)
   { 
       topicString.cstring = gDevBaseParam.mqttTopic.publish;       //�����ϱ� ����

       log_d("payloadlen = %d,payload = %s\r\n",payload_out_len,payload_out);

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
       if(rc == len) 
        {
           log_d("send PUBLISH Successfully,rc = %d,len = %d\r\n",rc,len);
       }
       else
       {
           log_d("send PUBLISH failed,rc = %d,len = %d\r\n",rc,len);     
       }
      
   }
   else
   {
        log_d("MQTT Lost the connect!!!\r\n");
   }
  

   return rc;
}

SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t tmp[4] = {0}; 
    int type = -1;
    uint16_t len = 0;
    READER_BUFF_STRU *ptReaderBuf = &gReaderMsg;     

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //2.��ȡ���Ŷ��� 
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"doorOption",1));
    log_d("doorOption = %s,len = %d\r\n",tmp,strlen((const char*)tmp));

    type = atoi(tmp);
    
    strcpy((char *)buf,(char *)packetBaseJson(msgBuf,1));

    len = strlen((const char*)buf);

    log_d("RemoteResetDev = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);  
    
    log_d("gSectorBuff = %d\r\n",sizeof(gSectorBuff));

    if(1 == type)
    {
        ptReaderBuf->devID = READER1; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }      
    }
    else if(2 == type)
    {
        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }      
    }
    else if(0 == type)
    {
        ptReaderBuf->devID = READER1; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }    

        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }            
    }

   

#ifdef DEBUG_PRINT
    TestFlash(CARD_MODE);
#endif    
	return result;
}

SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	//1.������ͨѶ�쳣��
	//2.�豸��ͣ�ã�����豸�����ʲô�ģ�������һ��״̬,�㻹���ҷ�Զ�̵ĺ���,�Ҿ͸�����һ���������쳣״̬���㡣
	//3.�洢���𻵣�
	//4.����������
	return result;
}




SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;

    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t tmp[CARD_NO_LEN] = {0};    
    uint8_t cardNo[CARD_NO_BCD_LEN] = {0};
    uint32_t ret = 0;
    int sendLen = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }
    
    log_d("AddCardNo = %s\r\n",msgBuf);
    
    //2.���濨��    
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    log_d("cardNo = %s,len = %d\r\n",tmp,strlen((const char*)tmp));
    
 
    memset(cardNo,0x00,sizeof(cardNo));
    asc2bcd(cardNo, tmp, CARD_NO_LEN, 1); 

    cardNo[0] = 0x00;//Τ��26���λ������
    
    log_d("add cardNo=  %02x, %02x, %02x, %02x\r\n",cardNo[0],cardNo[1],cardNo[2],cardNo[3]);

    memset(buf,0x00,sizeof(buf));
    

    ret = addCard(cardNo,CARD_MODE);
    log_d("addCard = %d\r\n",ret);
    
    if(ret >= 1)
    {
        gCardSortTimer.outTimer = 60000;
        gCardSortTimer.flag = 1; 
        
        //Ϊ�˷�ֹ©�£���д�뵽FLASH��,OK��Ӧ������� 
        result = packetSingleAddCardJson(msgBuf,1,buf);        
//        log_d("packetSingleAddCardJson %s,len = %d\r\n",buf,strlen((char *)buf)); 
    }  
    else
    {
        //Ϊ�˷�ֹ©�£���д�뵽FLASH��,OK��Ӧ������� 
        result = packetSingleAddCardJson(msgBuf,0,buf);        
        //        log_d("packetSingleAddCardJson %s,len = %d\r\n",buf,strlen((char *)buf));        
    }

    if(result != NO_ERR)
    {
        return result;
    }
    
    sendLen = mqttSendData(buf,strlen((const char*)buf)); 
    
    if(sendLen < 20)//���һ������
    {
        result = FLASH_W_ERR;     
    }      
    
	return result;
}

//1013 ɾ����Ա�����п�
SYSERRORCODE_E DelCardNoAll ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t tmp[CARD_NO_BCD_LEN] = {0};
    uint16_t len = 0;
    int wRet=1;
    int ret = 0;
    uint8_t num=0;
    int i = 0;  
    uint8_t cardArray[20][8] = {0};    
   

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }   

//    cardArray = GetCardArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&num);  
    GetCardArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&num,cardArray);  

    
    //ɾ��CARDNO
    for(i=0; i<num;i++)
    {
        log_d("%d / %d :cardNo = %s\r\n",num,i+1,cardArray[i]);      
        memset(tmp,0x00,sizeof(tmp));
        asc2bcd(tmp, cardArray[i], CARD_NO_LEN, 1);        
        log_d("cardNo: %02x %02x %02x %02x\r\n",tmp[0],tmp[1],tmp[2],tmp[3]);
        tmp[0] = 0x00;

        wRet = readHead(tmp,CARD_MODE);
        
//        wRet = delHead(tmp,CARD_MODE);
//        log_d("cardArray %d = %s,ret = %d\r\n",i,cardArray[i],wRet);  

        //2.��ѯ�Կ���ΪID�ļ�¼����ɾ��
        if(wRet != NO_FIND_HEAD)
        {
            //��Ӧ������
            result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",0,buf);
        }
        else
        {
            result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",0,buf);
        }  

        if(result != NO_ERR)
        {
            return result;
        }

        len = strlen((const char*)buf);

        ret = mqttSendData(buf,len); 

        if((ret > 20) && (wRet != NO_FIND_HEAD)) //���������һ�����ȣ�Ϊ�˱������������ͻ����������Ҫ��
        {        
            SendToQueue(tmp,CARD_NO_BCD_LEN,4);            
        }     

    }
    
    return result;
}

SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t tmpUrl[MQTT_TEMP_LEN] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //3.��������JSON����
    saveUpgradeData(msgBuf);

    //1.����URL
    strcpy((char *)tmpUrl,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"url",1));
    log_d("tmpUrl = %s\r\n",tmpUrl);
    
    
    ef_set_env((const char*)"url",(const char*)tmpUrl); 

    //2.��������״̬Ϊ������״̬
    ef_set_env("up_status", "101700"); 
    
    //4.���ñ�־λ������
    SystemUpdate();
    
	return result;

}



SYSERRORCODE_E getRemoteTime ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    result = getTimePacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("getRemoteTime len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    
	return result;

}



SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;

    //��ȡ�������ݲ�����JSON��   

    result = upgradeDataPacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("UpgradeAck len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    
	return result;

}

SYSERRORCODE_E EnableDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t typeBuf[4] = {0};
    uint16_t len = 0;
    int type = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.��ȡָ����
    memset(typeBuf,0x00,sizeof(typeBuf));
    strcpy((char *)typeBuf,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"type",1));
    type = atoi((const char*)typeBuf);
    
    if(type == 1)
    {
        SaveDevState(DEVICE_ENABLE);
    }
    else
    {
         SaveDevState(DEVICE_DISABLE);
    }
    
     //add 2020.04.27    
     xQueueReset(xCardIDQueue);  
     
    strcpy((char *)buf,(const char *)packetBaseJson(msgBuf,1));
    
    len = strlen((const char*)buf);

    log_d("EnableDev len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    //������Ҫ����Ϣ����Ϣ���У�����
    SendToQueue(typeBuf,strlen((const char*)typeBuf),AUTH_MODE_BIND);
    return result;



}

//SYSERRORCODE_E DisableDev ( uint8_t* msgBuf )
//{
//    SYSERRORCODE_E result = NO_ERR;
//    uint8_t buf[MQTT_TEMP_LEN] = {0};
//    uint8_t type[4] = {"0"};
//    uint16_t len = 0;

//    if(!msgBuf)
//    {
//        return STR_EMPTY_ERR;
//    }


//    result = modifyJsonItem(msgBuf,"status","1",1,buf);

//    if(result != NO_ERR)
//    {
//        return result;
//    }

//    SaveDevState(DEVICE_DISABLE);
//    
//    
//    len = strlen((const char*)buf);

//    log_d("DisableDev len = %d,buf = %s,status = %x\r\n",len,buf,gDevBaseParam.deviceState.iFlag);

//    mqttSendData(buf,len);
//    
//    //������Ҫ����Ϣ����Ϣ���У�����
//    SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
//    return result;


//}

//SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf )
//{
//	SYSERRORCODE_E result = NO_ERR;
//	
//	return result;
//}

SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint16_t len = 0;
    uint8_t tmpBcd[CARD_NO_BCD_LEN] = {0};

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = PacketDeviceInfo(msgBuf,buf);


    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetDevInfo len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);    

    //�������һ��ָ��������������ݽ�������    
    SendToQueue(tmpBcd,CARD_NO_BCD_LEN,10); //���������ҳ����
    
	return result;

}
 
//ɾ������  ����ɾ��
static SYSERRORCODE_E DelCardSingle( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	int ret = 0;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t cardNo[CARD_NO_LEN] = {0};
    uint8_t tmp[CARD_NO_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }
    
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    sprintf((char *)cardNo,"%08s",tmp); 

    log_d("tmp = %s,cardNo = %s\r\n",tmp,cardNo);

    memset(tmp,0x00,sizeof(tmp));
    asc2bcd(tmp, cardNo, CARD_NO_LEN, 1);        
    log_d("cardNo: %02x %02x %02x %02x\r\n",tmp[0],tmp[1],tmp[2],tmp[3]);    

    tmp[0] = 0x00;

    //1.����Ҫɾ���Ŀ���
    ret = readHead(tmp,CARD_MODE);
    
//    //ɾ��CARDNO
//    wRet = delHead(tmp,CARD_MODE);
    
    //2.��ѯ�Կ���ΪID�ļ�¼����ɾ��
    if(ret != NO_FIND_HEAD)
    {
        //��Ӧ������
//        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",0,buf);
        strcpy((char *)buf,(const char*)packetBaseJson(msgBuf,1));
    }
    else
    {
        //����û�и�����¼����������
//        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",0,buf);
        strcpy((char *)buf,(const char *)packetBaseJson(msgBuf,0));
    }  

//    if(result != NO_ERR)
//    {
//        return result;
//    }

    len = strlen((const char*)buf);

    ret = mqttSendData(buf,len); 
    if(ret > 20) //���������һ�����ȣ�Ϊ�˱������������ͻ����������Ҫ��
    {        
        SendToQueue(tmp,CARD_NO_BCD_LEN,4);            
    } 
    
	return result;

}

SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }


    //����ģ������ ����Ӧ����һ���߳�ר�����ڶ�дFLASH�������ڼ䣬��ʱ������Ӧ���
    //saveTemplateParam(msgBuf);    
    
    uint8_t *buf = packetRespTemplateJson(msgBuf,1);

    if(buf == NULL)
    {
        return STR_EMPTY_ERR;
    }
    
    len = strlen((const char*)buf);

    log_d("GetParam len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    //����ģ������
    saveTemplateParam(msgBuf);
    
	return result;
}

//�������IP
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t ip[32] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.����IP     
    strcpy((char *)ip,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"ip",1));
    log_d("server ip = %s\r\n",ip);

    //Ӱ�������
    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);
    
	return result;

}

//��ȡ�û���Ϣ
static SYSERRORCODE_E DownLoadCardID ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[256] = {0};
    uint8_t tmpBcd[CARD_NO_BCD_LEN] = {0};   
    uint8_t tmpAsc[CARD_NO_LEN] = {0};
    uint8_t cardArray[20][8] = {0};
    uint8_t multipleCardNum=0;    
    uint16_t i = 0;  
    int sendLen = 0;
    uint32_t ret = 0;    

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    gCardSortTimer.outTimer = 60000;
    gCardSortTimer.flag = 1;
    
    //2.���濨��
    GetCardArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&multipleCardNum,cardArray);
    
    for(i=0;i<multipleCardNum;i++)
    { 
//        ggsum++;
        memset(tmpAsc,0x00,sizeof(tmpAsc));
        memset(tmpBcd,0x00,sizeof(tmpBcd));
        memcpy(tmpAsc,cardArray[i],CARD_NO_LEN);
        log_d("%d / %d :cardNo = %s,asc = %s\r\n",multipleCardNum,i+1,cardArray[i],tmpAsc); 
        
        asc2bcd(tmpBcd, tmpAsc, CARD_NO_LEN, 1);        
        tmpBcd[0] = 0x00;//Τ��26���λ������     
        
        memset(buf,0x00,sizeof(buf));


        ret = addCard(tmpBcd,CARD_MODE);
        log_d("addCard = %d\r\n",ret);
        
        if(ret >= 1)
        {
//            result = modifyJsonItem(packetBaseJson(msgBuf,1),"cardNo",tmpAsc,0,buf); 
            strcpy((char *)buf,(const char*)packetBaseJson(msgBuf,1));
        }
        else
        {
//            result = modifyJsonItem(packetBaseJson(msgBuf,0),"cardNo",tmpAsc,0,buf); 
            
            strcpy((char *)buf,(const char*)packetBaseJson(msgBuf,0));
        }

//        if(result != NO_ERR)
//        {            
//            return result;
//        }     
        
        //Ϊ�˷�ֹ©�£���д�뵽FLASH��,OK��Ӧ������� 
        sendLen = mqttSendData(buf,strlen((const char*)buf)); 
        
        if(sendLen < 20)//���һ������
        {
            result = FLASH_W_ERR;     
        }

        if((ret/1024 >= 1) && (ret%1024 == 0))
        {
            SendToQueue(tmpBcd,CARD_NO_BCD_LEN,2); //���������ҳ����
        }   

    }
    
	return result;

}

//Զ�̺���
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t tmp[4] = {0};    
    uint16_t len = 0;
    int type = -1;
    
    READER_BUFF_STRU *ptReaderBuf = &gReaderMsg;     

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //2.��ȡ���Ŷ��� 
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"doorOption",1));
    log_d("doorOption = %s,len = %d\r\n",tmp,strlen((const char*)tmp));    
    
    strcpy((char *)buf,(const char*)packetBaseJson(msgBuf,1));

    len = strlen((const char*)buf);

    log_d("RemoteResetDev = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);      

    if(1 == type)
    {
        ptReaderBuf->devID = READER1; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }      
    }
    else if(2 == type)
    {
        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }      
    }
    else if(0 == type)
    {
        ptReaderBuf->devID = READER1; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }    

        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    	if(xQueueSend(xCardIDQueue,             /* ��Ϣ���о�� */
    				 (void *) &ptReaderBuf,             /* ���ͽṹ��ָ�����ptReader�ĵ�ַ */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);ɾ���þ䣬Ϊ�˷�ֹ���·����ݵ�ʱ��ˢ��
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //���Ϳ���ʧ�ܷ�������ʾ
            //�����Ƕ�����                
        }            
    }   

#ifdef DEBUG_PRINT
    TestFlash(CARD_MODE);
#endif    
    return result;


}



//ɾ���û���Ϣ
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    if(!msgBuf)
    {
    
        return STR_EMPTY_ERR;
    }
    
    //����û���Ϣ
    eraseUserDataAll();    
    return result;
}


//���ñ���ʱ��
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t localTime[32] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)localTime,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"time",1));

    //���汾��ʱ��
    log_d("server time is %s\r\n",localTime);

    bsp_ds1302_mdifytime(localTime);


    return result;

}




//���ñ���SN��MQTT��
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t deviceCode[32] = {0};//�豸ID
    uint8_t deviceID[5] = {0};//QRID
    uint16_t len = 0;

    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)deviceCode,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"deviceCode",0));
    strcpy((char *)deviceID,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"id",0));


    result = modifyJsonItem(msgBuf,"status","1",0,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("SetLocalSn len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);
    

    //��¼SN
    ClearDevBaseParam();
    optDevBaseParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    
    log_d("gDevBaseParam.deviceCode.deviceSn = %s,len = %d\r\n",gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen);

    gDevBaseParam.deviceCode.qrSnLen = strlen((const char*)deviceID);
    gDevBaseParam.deviceCode.deviceSnLen = strlen((const char*)deviceCode);
    memcpy(gDevBaseParam.deviceCode.deviceSn,deviceCode,gDevBaseParam.deviceCode.deviceSnLen);
    memcpy(gDevBaseParam.deviceCode.qrSn,deviceID,gDevBaseParam.deviceCode.qrSnLen);

    gDevBaseParam.deviceCode.downLoadFlag.iFlag = DEFAULT_INIVAL;    
    
    strcpy ( gDevBaseParam.mqttTopic.publish,DEVICE_PUBLISH );
    strcpy ( gDevBaseParam.mqttTopic.subscribe,DEVICE_SUBSCRIBE );    
    strcat ( gDevBaseParam.mqttTopic.subscribe,(const char*)deviceCode );     
    optDevBaseParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    gUpdateDevSn = 1;

    return result;


}

//ƽ̨�·���Ԫ����Ϣ
static SYSERRORCODE_E downPubKey( uint8_t* msgBuf )
{

}


//void sendHeartbeat(void)
//{
//    SYSERRORCODE_E result = NO_ERR;

//    uint8_t buf[128] = {0};
//    uint16_t len = 0;

//    result = getHeartbeatPacket(buf);

//    if(result != NO_ERR)
//    {
//        log_e("sendHeartbeat error\r\n");
//        return;
//    }

//    len = strlen((const char*)buf);

//    log_d("sendHeartbeat len = %d,buf = %s\r\n",len,buf);

//    mqttSendData(buf,len);  
//}




