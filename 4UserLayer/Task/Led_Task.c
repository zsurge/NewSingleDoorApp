/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Led_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : LED灯控制
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "led"
#include "elog.h"

#include "led_task.h"
#include "version.h" 
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_dipSwitch.h"
#include "ini.h"
#include "bsp_uart_fifo.h"
#include "tool.h"
#include "lwip_comm.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define LED_TASK_PRIO	    (tskIDLE_PRIORITY+2)
#define LED_STK_SIZE 		(configMINIMAL_STACK_SIZE)

#ifdef SERIAL
#define STEP_1   0
#define STEP_2   10
#define STEP_3   20

#define UNFINISH	 0x00
#define FINISH	     0x55

#define READ_STATUS 0x01
#define READ_IP     0x02
#define SET_IP      0x03
#define RESET_SN      0x04
#define RESET_BASE_PARAM 0x05

//添加设备程序模式，读卡器类型，门禁类型
#define SET_PROGRAM_MODE 0x06
#define SET_CARD_READER_TYPE 0x07
#define SET_DOOR_TYPE 0x08

//添加设备程序模式，读卡器类型，门禁类型
#define GET_PROGRAM_MODE 0x09
#define GET_CARD_READER_TYPE 0x0A
#define GET_DOOR_TYPE 0x0B


#define GET_CARD_NO 0x0C


#define HEAD    0xA5
#define TAIL    0x5A

#define OK  0x00
#define ERR 0x01
#endif


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *ledTaskName = "vLedTask";      //LED任务名称


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskLed = NULL;      //LED灯



/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskLed(void *pvParameters);

static void showTask(void);


#ifdef SERIAL
typedef struct FROM_PC
{
    uint8_t rxStatus;                   //接收状态
    uint8_t rxCRC;                      //校验值.
    uint8_t rxCnt;                      //已接收字节数  
    uint8_t rxBuff[64];                 //接收字节
    uint8_t rxPacketState;              //整个数据包接收状态       
}FROM_PC_STRU;

static FROM_PC_STRU rxFromPc;

static void sendToHost(char CMD);
static void deal_pc_procotol(void);
static void deal_pc_data(void);
static void init_from_pc_buf(void);

static uint8_t readDevStatus(char *txBuf);
static uint8_t readIp(char *txBuf);
static void SetLocalIP(char *buf,uint8_t bufLen);
static uint8_t retDefBuff(char *txBuf);
static uint8_t resetSn(char *txBuf);
static uint8_t resetBaseParam(char *txBuf);

static uint8_t setDevMode(uint8_t cmd,char *src,uint8_t bufLen,char *txBuf);
static uint8_t getDevMode(uint8_t cmd,char *txBuf);
#endif









void CreateLedTask(void)
{
    //创建LED任务
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )ledTaskName,       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);
}


//LED任务函数 
static void vTaskLed(void *pvParameters)
{  
    uint16_t i = 0;
    
//    DisplayDevInfo();
    
    while(1)
    {  
         
        if(i++ == 50)
        {
            LEDERROR = !LEDERROR; 
            i = 0;
//            showTask();
        }

//        deal_pc_procotol();
//        deal_pc_data();

        //这里要判定是否是测试模式，若是，才响应上位机
        
		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
        vTaskDelay(20);  
    }
} 


static void showTask(void)
{
	uint8_t pcWriteBuffer[1024];
    uint32_t g_memsize;
    
    printf("=================================================\r\n");
    printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
    vTaskList((char *)&pcWriteBuffer);
    printf("%s\r\n", pcWriteBuffer);
    
    printf("\r\n任务名       运行计数         使用率\r\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s\r\n", pcWriteBuffer);          
    
    g_memsize = xPortGetFreeHeapSize();
    printf("系统当前内存大小为 %d 字节，开始申请内存\r\n",g_memsize);      

}


#ifdef SERIAL

void deal_pc_procotol(void)
{
    uint8_t ch = 0;
    uint16_t dataLen = 0;      

    
    while(RS485_Recv(COM2,&ch,1))
    {    
       switch (rxFromPc.rxStatus)
        {                
            case STEP_1:
                if(HEAD == ch) /*接收包头*/
                {
                    rxFromPc.rxBuff[0] = ch;                    
                    rxFromPc.rxCnt = 1;
                    rxFromPc.rxCRC = ch;
                    rxFromPc.rxStatus = STEP_2;
                }
                break;
           case STEP_2:
                if (TAIL == ch) 
                {
                    rxFromPc.rxStatus = STEP_3;
                }
                rxFromPc.rxBuff[rxFromPc.rxCnt++] = ch; //ETX
                rxFromPc.rxCRC ^=  ch;
                break;
          case STEP_3:                
                dataLen = ((uint16_t)rxFromPc.rxBuff[1] << 8) |(uint16_t)(rxFromPc.rxBuff[2]);                

                //这里减2主要是减去长度的两个字节
                if(dataLen == rxFromPc.rxCnt && rxFromPc.rxCRC == ch)
                {
                     rxFromPc.rxBuff[rxFromPc.rxCnt++] = ch;
                     rxFromPc.rxPacketState = FINISH;                    
                     rxFromPc.rxStatus = STEP_1;
                     break;
                }
                else
                {   
                    log_d("error step3 = %02x,len = %d,cnt = %d,crc = %02x\r\n",ch,dataLen,rxFromPc.rxCnt,rxFromPc.rxCRC);
                    
                    rxFromPc.rxPacketState = FINISH;  
                    rxFromPc.rxBuff[rxFromPc.rxCnt++] = ch;
                    rxFromPc.rxStatus = STEP_2;
                    rxFromPc.rxCRC =  0;
                }
                break;         
            default:
                if (rxFromPc.rxPacketState == UNFINISH) 
                {
                    rxFromPc.rxPacketState = FINISH;
                    rxFromPc.rxStatus = STEP_1;
                } 
         }
    }  
}

void deal_pc_data(void)
{
    uint8_t buf[128] = {0};

    memset(buf,0x00,sizeof(buf));  
    
    if (FINISH == rxFromPc.rxPacketState)
    {  
        dbh("rxFromPc.rxBuff", (char *)rxFromPc.rxBuff, rxFromPc.rxCnt);        

        if((((uint16_t)rxFromPc.rxBuff[1] << 8) |(uint16_t)(rxFromPc.rxBuff[2])) != rxFromPc.rxCnt-1)                                   //解析02数据包
        {   
            log_d("len error\r\n");
            init_from_pc_buf();
            return ;
        }

        switch(rxFromPc.rxBuff[3])
        {
            case READ_STATUS:
                 log_d("read status\r\n");
                 sendToHost(READ_STATUS);
                 break;
            case READ_IP:
                 log_d("read ip\r\n");
                 sendToHost(READ_IP);
                 break;   
            case SET_IP:
                 log_d("set ip\r\n");
                 sendToHost(SET_IP);
                 break;
            case RESET_SN:
                 log_d("RESET SN\r\n");
                 sendToHost(RESET_SN);            
                 break;        
            case RESET_BASE_PARAM:
                 log_d("RESET BASE PARAM\r\n");
                 sendToHost(RESET_BASE_PARAM);           
                 break;
            case SET_PROGRAM_MODE:
                 log_d("SET_PROGRAM_MODE\r\n");
                 sendToHost(SET_PROGRAM_MODE);
                 break;
            case SET_CARD_READER_TYPE:
                 log_d("SET_CARD_READER_TYPE\r\n");
                 sendToHost(SET_CARD_READER_TYPE);            
                 break;        
            case SET_DOOR_TYPE:
                 log_d("SET_DOOR_TYPE\r\n");
                 sendToHost(SET_DOOR_TYPE);           
                 break;
            case GET_PROGRAM_MODE:
                 log_d("GET_PROGRAM_MODE\r\n");
                 sendToHost(GET_PROGRAM_MODE);
                 break;
            case GET_CARD_READER_TYPE:
                 log_d("GET_CARD_READER_TYPE\r\n");
                 sendToHost(GET_CARD_READER_TYPE);            
                 break;        
            case GET_DOOR_TYPE:
                 log_d("GET_DOOR_TYPE\r\n");
                 sendToHost(GET_DOOR_TYPE);           
                 break;                  
                 
            default:
                 break;                
        }                        
       
    }

}


static void init_from_pc_buf(void)
{ 
    memset(&rxFromPc,0x00,sizeof(FROM_PC_STRU));
}


void sendToHost(char CMD)
{
    char sendBuff[128] = {0};
    uint8_t len = 0;
    switch (CMD)
    {
        case READ_STATUS:
            log_d("resp read status\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));
            len = readDevStatus(sendBuff);            
            break;
        case READ_IP:
            log_d("resp dev ip\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));        
            len = readIp(sendBuff);  
            break;    
       case SET_IP:
            log_d("resp dev set ip\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));        
            SetLocalIP((char *)rxFromPc.rxBuff, rxFromPc.rxCnt);   
            len = retDefBuff(sendBuff);
            RS485_SendBuf(COM2,(uint8_t *)sendBuff,len); 
            vTaskDelay(50); 
            //4.重启
            NVIC_SystemReset();
            break;
        case RESET_SN:
             log_d("resp RESET SN\r\n");
             len=0;
             memset(sendBuff,0x00,sizeof(sendBuff));        
             resetLocalSn();
             len = resetSn(sendBuff);  

             RS485_SendBuf(COM2,(uint8_t *)sendBuff,len); 
             vTaskDelay(50); 
             //4.重启
             NVIC_SystemReset();                   
             break;        
        case RESET_BASE_PARAM:
             log_d("resp RESET SN\r\n");
             len=0;
             memset(sendBuff,0x00,sizeof(sendBuff));  
             resetLocalBaseParam();
             len = resetBaseParam(sendBuff);  
             
             RS485_SendBuf(COM2,(uint8_t *)sendBuff,len); 
             vTaskDelay(50); 
             //4.重启
             NVIC_SystemReset();             
             break;

       case SET_PROGRAM_MODE:
            log_d("resp SET PROGRAM MODE\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));        
            len = setDevMode(SET_PROGRAM_MODE,(char *)rxFromPc.rxBuff, rxFromPc.rxCnt,sendBuff);   
            RS485_SendBuf(COM2,(uint8_t *)sendBuff,len); 
            vTaskDelay(50); 
            //4.重启
            NVIC_SystemReset();       
            break;
       case SET_CARD_READER_TYPE:
            log_d("resp SET_CARD_READER_TYPE\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));        
            len = setDevMode(SET_CARD_READER_TYPE,(char *)rxFromPc.rxBuff, rxFromPc.rxCnt,sendBuff);   
            RS485_SendBuf(COM2,(uint8_t *)sendBuff,len); 
            vTaskDelay(50); 
            //4.重启
            NVIC_SystemReset();       
            break;
       case SET_DOOR_TYPE:
            log_d("resp SET_DOOR_TYPE\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));        
            len = setDevMode(SET_DOOR_TYPE,(char *)rxFromPc.rxBuff, rxFromPc.rxCnt,sendBuff);   
            RS485_SendBuf(COM2,(uint8_t *)sendBuff,len); 
            vTaskDelay(50); 
            //4.重启
            NVIC_SystemReset();       
            break;        
        case GET_PROGRAM_MODE:
            log_d("resp GET_PROGRAM_MODE\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));
            len = getDevMode(GET_PROGRAM_MODE,sendBuff);            
            break;  
        case GET_CARD_READER_TYPE:
            log_d("resp GET_CARD_READER_TYPE\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));
            len = getDevMode(GET_CARD_READER_TYPE,sendBuff);            
            break;
        case GET_DOOR_TYPE:
            log_d("resp GET_DOOR_TYPE\r\n");
            len=0;
            memset(sendBuff,0x00,sizeof(sendBuff));
            len = getDevMode(GET_DOOR_TYPE,sendBuff);            
            break;  
        default:            
            break;
    }

    dbh("send buff", sendBuff, len);

    RS485_SendBuf(COM2,(uint8_t *)sendBuff,len);    
    init_from_pc_buf();
}




static uint8_t retDefBuff(char *txBuf)
{
    char buf[128] = {0};
    char ip[32] = {0};
    
    uint8_t i = 0;
    uint8_t crc = 0;

    i = 3;
    buf[0] = HEAD;
    buf[i++] = SET_IP;
    buf[i++] = OK;

    sprintf(ip,"%d.%d.%d.%d",gDevBaseParam.localIP.ip[0],gDevBaseParam.localIP.ip[1],gDevBaseParam.localIP.ip[2],gDevBaseParam.localIP.ip[3]);

    memcpy(buf+i,ip,strlen((const char*)ip));
    i+= strlen((const char*)ip);
    
    buf[i++] = TAIL;
    
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);

    dbh("txBuf", txBuf, i);

    return i; 
}


static uint8_t readDevStatus(char *txBuf)
{
    char buf[128] = {0};
    char tmp[16] = {0};
    uint8_t i = 0;
    uint8_t crc = 0;

    i = 3;
    buf[0] = HEAD;
    buf[i++] = READ_STATUS;
    buf[i++] = OK;
    memcpy(buf+i,gDevinfo.SoftwareVersion,strlen((const char*)gDevinfo.SoftwareVersion));
    i+= strlen((const char*)gDevinfo.SoftwareVersion);
    buf[i++] = '-';

    memcpy(buf+i,gDevinfo.GetSn(),strlen((const char*)gDevinfo.GetSn()));
    i+= strlen((const char*)gDevinfo.GetSn());   
    buf[i++] = '-';    

    memcpy(buf+i,gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen);
    i+= gDevBaseParam.deviceCode.deviceSnLen;   
    buf[i++] = '-';    

    memset(tmp,0x00,sizeof(tmp));
    sprintf(tmp,"%d",gRecordIndex.cardNoIndex);

    memcpy(buf+i,tmp,strlen((const char*)tmp));
    i+= strlen((const char*)tmp);   
    buf[i++] = '-';

    if(gDevBaseParam.progamMode == PROGRAMMODE_CHANNEL)
    {     
        memcpy(buf+i,"C",1);
        i+= 1;   
        buf[i++] = '-';    
    }
    else if(gDevBaseParam.progamMode == PROGRAMMODE_DOOR) 
    {
        memcpy(buf+i,"D",1);
        i+= 1;   
        buf[i++] = '-';    
    }
    else
    {
        memcpy(buf+i,"0",1);
        i+= 1;   
        buf[i++] = '-';    
    }
    
    buf[i++] = TAIL;
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);

    return i;    
}


static uint8_t readIp(char *txBuf)
{
    char buf[128] = {0};
    
    uint8_t i = 0;
    uint8_t crc = 0;

    i = 3;
    buf[0] = HEAD;
    buf[i++] = READ_IP;
    buf[i++] = OK;
     
    memcpy(buf+i,gDevinfo.GetIP(),strlen((const char*)gDevinfo.GetIP()));
    
    i+= strlen((const char*)gDevinfo.GetIP());   
    buf[i++] = TAIL;
    
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);

    dbh("txBuf", txBuf, i);

    return i; 

}

static void SetLocalIP(char *buf,uint8_t bufLen)
{    
    uint8_t ip[4] = {0};   
    uint8_t netmask[4] = {0};   
    uint8_t gateway[4] = {0};   
    uint8_t tmpBuf[64] = {0};
            
    int ip1,ip2,ip3,ip4,mark1,mark2,mark3,mark4,gateway1,gateway2,gateway3,gateway4;

    memcpy(tmpBuf,buf+4,bufLen-6);
    
    log_d("ip and mask and gateway = %s\r\n",tmpBuf); 

    sscanf((const char*)tmpBuf,"%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4,&mark1,&mark2,&mark3,&mark4,&gateway1,&gateway2,&gateway3,&gateway4);

    ip[0] = ip1;    
    ip[1] = ip2;
    ip[2] = ip3;
    ip[3] = ip4;

    netmask[0] = mark1;
    netmask[1] = mark2;
    netmask[2] = mark3;
    netmask[3] = mark4;

    gateway[0] = gateway1;
    gateway[1] = gateway2;
    gateway[2] = gateway3;
    gateway[3] = gateway4;  

    
    if(setLocalIpAddr(ip,netmask,gateway))
    {
        //reset
        log_d("reset dev\r\n");
    }
}

static uint8_t resetSn(char *txBuf)
{
    char buf[128] = {0};

    
    uint8_t i = 0;
    uint8_t crc = 0;

    i = 3;
    buf[0] = HEAD;
    buf[i++] = RESET_SN;
    buf[i++] = OK;
    

    buf[i++] = TAIL;
    
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);
    dbh("txBuf", txBuf, i);

    return i; 
}


static uint8_t resetBaseParam(char *txBuf)
{
    char buf[128] = {0};
    
    uint8_t i = 0;
    uint8_t crc = 0;
    
    
    i = 3;
    buf[0] = HEAD;
    buf[i++] = READ_IP;
    buf[i++] = OK;

    buf[i++] = TAIL;
    
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);

   dbh("txBuf", txBuf, i);

  return i; 

}



static uint8_t setDevMode(uint8_t cmd,char *src,uint8_t bufLen,char *txBuf)
{
    char buf[128] = {0};
    
    uint8_t i = 0;
    uint8_t crc = 0;
    uint8_t tmpValue = 0;

    i = 3;
    buf[0] = HEAD;
    buf[i++] = SET_PROGRAM_MODE;

    if(!src || bufLen < 6)
    {
        buf[i++] = ERR;
    }
    else
    {
        buf[i++] = OK;
    }   
    
    tmpValue = src[4];

    log_d("setDevMode = %d\r\n",tmpValue);

    if(cmd == SET_PROGRAM_MODE)
    {
        setProgramMode((PROGRAM_MODE)tmpValue);    
        log_d("set Porgram Mode = %s\r\n",tmpValue); 
    }
    else if(cmd == SET_CARD_READER_TYPE)
    {
        setCardReaderType((CARD_READER_TYPE)tmpValue);    
        log_d("set CARD_READER_TYPE = %s\r\n",tmpValue); 
    }
    else if(cmd == SET_DOOR_TYPE)
    {
        setDoorType((DOOR_TYPE) tmpValue);
        log_d("set  DOOR_TYPE = %s\r\n",tmpValue); 
    }   

    
    buf[i++] = TAIL;
    
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);
    dbh("txBuf", txBuf, i);

    return i;     
    
}



static uint8_t getDevMode(uint8_t cmd,char *txBuf)
{
    char buf[128] = {0};
    
    uint8_t i = 0;
    uint8_t crc = 0;   

    i = 3;
    buf[0] = HEAD;
    buf[i++] = cmd;
    buf[i++] = OK;

    
    if(cmd == GET_PROGRAM_MODE)
    {
        buf[i++] = getProgramMode();
        
    }
    else if(cmd == GET_CARD_READER_TYPE)
    {
        buf[i++] = getCardReaderType();    
    }
    else if(cmd == GET_DOOR_TYPE)
    {
        buf[i++] = getDoorType();
    }  

    
    buf[i++] = TAIL;
    
    buf[1] = i>>8;   //high
    buf[2] = i&0xFF; //low   
    
    crc = xorCRC((uint8_t *)buf, i);  
    buf[i++] = crc; 

    memcpy(txBuf,buf,i);
    dbh("txBuf", txBuf, i);

    return i;     
    
}
#endif





