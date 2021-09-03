/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : comm.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月18日
  最近修改   :
  功能描述   : 解析串口指令
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月18日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
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
 * 宏定义                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0])) //计算数组长度


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
    

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
int gConnectStatus = 0;
int	gMySock = 0;
uint8_t gUpdateDevSn = 0; 

//static uint16_t ggcnt = 0;
//static uint16_t ggsum = 0;




READER_BUFF_STRU gReaderMsg;
READER_BUFF_STRU gReaderRecvMsg;

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode);
static SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf ); //开门
static SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf ); //远程报警
static SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf ); //添加卡号
static SYSERRORCODE_E DelCardNoAll ( uint8_t* msgBuf ); //删除卡号
static SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf ); //对设备进行升级
static SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf ); //升级应答
static SYSERRORCODE_E EnableDev ( uint8_t* msgBuf ); //开启设备
//static SYSERRORCODE_E DisableDev ( uint8_t* msgBuf ); //关闭设备
static SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf ); //获取设备信息
static SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf ); //获取模板参数
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf ); //获取模板参数
static SYSERRORCODE_E DownLoadCardID ( uint8_t* msgBuf ); //获取用户信息
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf ); //远程呼梯
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf ); //删除用户信息
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf ); //设置本地时间
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf ); //设置本地SN，MQTT用
static SYSERRORCODE_E DelCardSingle( uint8_t* msgBuf ); //删除卡号
static SYSERRORCODE_E getRemoteTime ( uint8_t* msgBuf );//获取远程服务器时间
static SYSERRORCODE_E RemoteResetDev ( uint8_t* msgBuf );//远程重启

//static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ); //返回默认消息

static SYSERRORCODE_E upMac( uint8_t* msgBuf );//上传MAC地址
static SYSERRORCODE_E downPubKey( uint8_t* msgBuf );//平台下发三元组信息




typedef SYSERRORCODE_E ( *cmd_fun ) ( uint8_t *msgBuf ); 

typedef struct
{
	const char* cmd_id;             /* 命令id */
	cmd_fun  fun_ptr;               /* 函数指针 */
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
    {"10002", EnableDev}, //同绑定
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

static SYSERRORCODE_E RemoteResetDev ( uint8_t* msgBuf )//远程重启
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

//上传MAC地址
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
    
	/* 清零 */
    ptMsg->mode = authMode; 
    memset(ptMsg->cardID,0x00,sizeof(ptMsg->cardID)); 
    memcpy(ptMsg->cardID,buf,len);

    
    /* 使用消息队列实现指针变量的传递 */
    if(xQueueSend(xCardIDQueue,              /* 消息队列句柄 */
                 (void *) &ptMsg,   /* 发送指针变量recv_buf的地址 */
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
       topicString.cstring = gDevBaseParam.mqttTopic.publish;       //属性上报 发布

       log_d("payloadlen = %d,payload = %s\r\n",payload_out_len,payload_out);

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//发布消息
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

    //2.获取开门动作 
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

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }      
    }
    else if(2 == type)
    {
        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }      
    }
    else if(0 == type)
    {
        ptReaderBuf->devID = READER1; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }    

        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
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
	//1.跟电梯通讯异常；
	//2.设备已停用；你把设备解绑了什么的，我这有一个状态,你还给我发远程的呼梯,我就给你抛一个这样的异常状态给你。
	//3.存储器损坏；
	//4.读卡器已损坏
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
    
    //2.保存卡号    
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    log_d("cardNo = %s,len = %d\r\n",tmp,strlen((const char*)tmp));
    
 
    memset(cardNo,0x00,sizeof(cardNo));
    asc2bcd(cardNo, tmp, CARD_NO_LEN, 1); 

    cardNo[0] = 0x00;//韦根26最高位无数据
    
    log_d("add cardNo=  %02x, %02x, %02x, %02x\r\n",cardNo[0],cardNo[1],cardNo[2],cardNo[3]);

    memset(buf,0x00,sizeof(buf));
    

    ret = addCard(cardNo,CARD_MODE);
    log_d("addCard = %d\r\n",ret);
    
    if(ret >= 1)
    {
        gCardSortTimer.outTimer = 60000;
        gCardSortTimer.flag = 1; 
        
        //为了防止漏下，先写入到FLASH中,OK后应答服务器 
        result = packetSingleAddCardJson(msgBuf,1,buf);        
//        log_d("packetSingleAddCardJson %s,len = %d\r\n",buf,strlen((char *)buf)); 
    }  
    else
    {
        //为了防止漏下，先写入到FLASH中,OK后应答服务器 
        result = packetSingleAddCardJson(msgBuf,0,buf);        
        //        log_d("packetSingleAddCardJson %s,len = %d\r\n",buf,strlen((char *)buf));        
    }

    if(result != NO_ERR)
    {
        return result;
    }
    
    sendLen = mqttSendData(buf,strlen((const char*)buf)); 
    
    if(sendLen < 20)//随便一个长度
    {
        result = FLASH_W_ERR;     
    }      
    
	return result;
}

//1013 删除人员的所有卡
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

    
    //删除CARDNO
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

        //2.查询以卡号为ID的记录，并删除
        if(wRet != NO_FIND_HEAD)
        {
            //响应服务器
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

        if((ret > 20) && (wRet != NO_FIND_HEAD)) //这里是随便一个长度，为了避免跟错误代码冲突，错误代码表要改
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

    //3.保存整个JSON数据
    saveUpgradeData(msgBuf);

    //1.保存URL
    strcpy((char *)tmpUrl,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"url",1));
    log_d("tmpUrl = %s\r\n",tmpUrl);
    
    
    ef_set_env((const char*)"url",(const char*)tmpUrl); 

    //2.设置升级状态为待升级状态
    ef_set_env("up_status", "101700"); 
    
    //4.设置标志位并重启
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

    //读取升级数据并解析JSON包   

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

    //1.读取指令字
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

    //这里需要发消息到消息队列，启用
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
//    //这里需要发消息到消息队列，禁用
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

    //这里添加一个指令，用来对所有数据进行排序    
    SendToQueue(tmpBcd,CARD_NO_BCD_LEN,10); //这里进行整页排序
    
	return result;

}
 
//删除卡号  单卡删除
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

    //1.查找要删除的卡号
    ret = readHead(tmp,CARD_MODE);
    
//    //删除CARDNO
//    wRet = delHead(tmp,CARD_MODE);
    
    //2.查询以卡号为ID的记录，并删除
    if(ret != NO_FIND_HEAD)
    {
        //响应服务器
//        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",0,buf);
        strcpy((char *)buf,(const char*)packetBaseJson(msgBuf,1));
    }
    else
    {
        //包括没有该条记录和其它错误
//        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"0",0,buf);
        strcpy((char *)buf,(const char *)packetBaseJson(msgBuf,0));
    }  

//    if(result != NO_ERR)
//    {
//        return result;
//    }

    len = strlen((const char*)buf);

    ret = mqttSendData(buf,len); 
    if(ret > 20) //这里是随便一个长度，为了避免跟错误代码冲突，错误代码表要改
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


    //保存模板数据 这里应该有一个线程专门用于读写FLASH，调试期间，暂时放在响应后边
    //saveTemplateParam(msgBuf);    
    
    uint8_t *buf = packetRespTemplateJson(msgBuf,1);

    if(buf == NULL)
    {
        return STR_EMPTY_ERR;
    }
    
    len = strlen((const char*)buf);

    log_d("GetParam len = %d,buf = %s\r\n",len,buf);

    mqttSendData(buf,len);

    //保存模板数据
    saveTemplateParam(msgBuf);
    
	return result;
}

//获服务器IP
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

    //1.保存IP     
    strcpy((char *)ip,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"ip",1));
    log_d("server ip = %s\r\n",ip);

    //影响服务器
    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    mqttSendData(buf,len);
    
	return result;

}

//获取用户信息
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
    
    //2.保存卡号
    GetCardArray ((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",&multipleCardNum,cardArray);
    
    for(i=0;i<multipleCardNum;i++)
    { 
//        ggsum++;
        memset(tmpAsc,0x00,sizeof(tmpAsc));
        memset(tmpBcd,0x00,sizeof(tmpBcd));
        memcpy(tmpAsc,cardArray[i],CARD_NO_LEN);
        log_d("%d / %d :cardNo = %s,asc = %s\r\n",multipleCardNum,i+1,cardArray[i],tmpAsc); 
        
        asc2bcd(tmpBcd, tmpAsc, CARD_NO_LEN, 1);        
        tmpBcd[0] = 0x00;//韦根26最高位无数据     
        
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
        
        //为了防止漏下，先写入到FLASH中,OK后应答服务器 
        sendLen = mqttSendData(buf,strlen((const char*)buf)); 
        
        if(sendLen < 20)//随便一个长度
        {
            result = FLASH_W_ERR;     
        }

        if((ret/1024 >= 1) && (ret%1024 == 0))
        {
            SendToQueue(tmpBcd,CARD_NO_BCD_LEN,2); //这里进行整页排序
        }   

    }
    
	return result;

}

//远程呼梯
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

    //2.获取开门动作 
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

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }      
    }
    else if(2 == type)
    {
        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }      
    }
    else if(0 == type)
    {
        ptReaderBuf->devID = READER1; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }    

        ptReaderBuf->devID = READER2; 
        ptReaderBuf->mode = REMOTE_OPEN_MODE;            

    	/* 使用消息队列实现指针变量的传递 */
    	if(xQueueSend(xCardIDQueue,             /* 消息队列句柄 */
    				 (void *) &ptReaderBuf,             /* 发送结构体指针变量ptReader的地址 */
    				 (TickType_t)10) != pdPASS )
    	{
    //                xQueueReset(xCardIDQueue);删除该句，为了防止在下发数据的时候刷卡
            log_d("send REMOTE_OPEN_MODE!\r\n"); 
            //发送卡号失败蜂鸣器提示
            //或者是队列满                
        }            
    }   

#ifdef DEBUG_PRINT
    TestFlash(CARD_MODE);
#endif    
    return result;


}



//删除用户信息
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    if(!msgBuf)
    {
    
        return STR_EMPTY_ERR;
    }
    
    //清空用户信息
    eraseUserDataAll();    
    return result;
}


//设置本地时间
static SYSERRORCODE_E SetLocalTime( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t localTime[32] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)localTime,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"time",1));

    //保存本地时间
    log_d("server time is %s\r\n",localTime);

    bsp_ds1302_mdifytime(localTime);


    return result;

}




//设置本地SN，MQTT用
static SYSERRORCODE_E SetLocalSn( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_TEMP_LEN] = {0};
    uint8_t deviceCode[32] = {0};//设备ID
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
    

    //记录SN
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

//平台下发三元组信息
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




