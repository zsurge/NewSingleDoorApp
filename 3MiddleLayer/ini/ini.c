/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ini.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 参数设置
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "ini.h"
#include "stdlib.h"
#include "bsp_spi_flash.h"
#include "FreeRTOS.h"
#include "task.h"


TEMPLATE_PARAM_STRU gtemplateParam;
DEV_BASE_PARAM_STRU gDevBaseParam;
RECORDINDEX_STRU gRecordIndex;

HEADINFO_STRU gSectorBuff[1024] = {0};


static uint8_t opParam(void *Param,uint8_t mode,uint32_t len,uint32_t addr);

static void eraseUserDataIndex ( void );


/*****************************************************************************
 函 数 名  : RestoreDefaultSetting
 功能描述  : 恢复默认设置,实际是写入系统启动所需要的参数
           若使用ef_env_set_default 则需要在ef_port.c中对默认值进行修改
 输入参数  : 
             
 输出参数  : 无
 返 回 值  : void
 
 修改历史      :
  1.日    期   : 2019年5月28日
    作    者   : 张舵

    修改内容   : 新生成函数

*****************************************************************************/
void RestoreDefaultSetting(void)
{
    if(ef_env_set_default()== EF_NO_ERR)
    {
        DBG("RestoreDefaultSetting success\r\n");
    }
    
}

void SystemUpdate(void)
{
    //写升级标志位
    if(ef_set_env("WSPIFLASH","5050") == EF_NO_ERR)
    {
        ef_set_env("WMCUFLASH","6060");
        //jump iap
        DBG("jump iap\r\n");
        NVIC_SystemReset();
    }
}

void SaveDevState(uint32_t state)
{
    uint8_t ret = 0;
    //记录SN
    ClearDevBaseParam();
    optDevBaseParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    gDevBaseParam.deviceState.iFlag = state; 
    ret = optDevBaseParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret != 1)
    {
        DBG("write device state error\r\n");
    }    
}

//保存模板信息
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON* root,*data,*templateData,*templateMap,*holidayTimeMap,*peakTimeMap;   
    cJSON* tmpArray,*arrayElement;
    
    int holidayTimeMapCnt=0,peakTimeMapCnt=0,index = 0;
    uint8_t ret = 0;
    char tmpIndex[2] = {0};
    char tmpKey[32] = {0};
    char *callingWay[4] = {0}; //存放分割后的子字符串 
    int callingWayNum = 0;

    TEMPLATE_PARAM_STRU *templateParam = &gtemplateParam; 

    uint32_t curtick  =  xTaskGetTickCount();    

    ClearTemplateParam();    
    ret = optTemplateParam(templateParam,READ_PRARM,sizeof(TEMPLATE_PARAM_STRU),DEVICE_TEMPLATE_PARAM_ADDR); 
    if(ret != NO_ERR)
    {
        return FLASH_R_ERR;
    }
    
    root = cJSON_Parse((char *)jsonBuff);    //解析数据包
    if (!root)  
    {  
        cJSON_Delete(root);
        DBG("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    data = cJSON_GetObjectItem(root, "data");
    if(data == NULL)
    {
        DBG("data NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;        
    }    

    templateData = cJSON_GetObjectItem(data, "template");
    if(templateMap == NULL)
    {
        DBG("templateData NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }    

    templateMap = cJSON_GetObjectItem(templateData, "templateMap");
    if(templateMap == NULL)
    {
        DBG("templateMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }

    holidayTimeMap = cJSON_GetObjectItem(templateData, "hoildayTimeMap");
    if(holidayTimeMap == NULL)
    {
        DBG("hoildayTimeMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }

    peakTimeMap = cJSON_GetObjectItem(templateData, "peakTimeMap");
    if(peakTimeMap ==NULL)
    {
        DBG("peakTimeMap NULL\r\n");
        result = CJSON_GETITEM_ERR;
        cJSON_Delete(root);
        return result;   
    }


//--------------------------------------------------    
    //获取templateMap数据
    cJSON *json_item = cJSON_GetObjectItem(templateMap, "id");
    templateParam->id = json_item->valueint;
//    sprintf(tmpbuf,"%8d",templateParam->id);
//    ef_set_env_blob("templateID",tmpbuf,8); 
    DBG("templateParam->id = %d\r\n",templateParam->id);

    json_item = cJSON_GetObjectItem(templateMap, "templateCode");
    strcpy((char *)templateParam->templateCode,json_item->valuestring);
//    ef_set_env_blob("templateCode",templateParam->templateCode,strlen((const char *)templateParam->templateCode)); 
    DBG("templateParam->templateCode = %s\r\n",templateParam->templateCode);

    json_item = cJSON_GetObjectItem(templateMap, "templateName");
    strcpy((char *)templateParam->templateName,json_item->valuestring);
//    ef_set_env_blob("templateName",templateParam->templateName,strlen(templateParam->templateName)); 
    DBG("templateParam->templateName = %s\r\n",templateParam->templateName);    

    json_item = cJSON_GetObjectItem(templateMap, "templateStatus");
    templateParam->templateStatus = json_item->valueint;
//    memset(tmpbuf,0x00,sizeof(tmpbuf));
//    sprintf(tmpbuf,"%8d",templateParam->templateStatus);
//    ef_set_env_blob("templateStatus",tmpbuf,8); 
    DBG("templateParam->templateStatus = %d\r\n",templateParam->templateStatus);      
    
    json_item = cJSON_GetObjectItem(templateMap, "callingWay");
    strcpy((char *)templateParam->callingWay,json_item->valuestring);
    //解析模板识别方式    
    split((char *)templateParam->callingWay,",",callingWay,&callingWayNum); //调用函数进行分割 
    while(callingWayNum)
    {
        switch(atoi(callingWay[callingWayNum-1]))
        {
            case 1:
                templateParam->templateCallingWay.isFace = 1;
                break;
            case 2:
                templateParam->templateCallingWay.isQrCode = 1;
                break;
            case 3:
                templateParam->templateCallingWay.isIcCard = 1;
                break; 
        }
        callingWayNum-- ;  
    }       
//    ef_set_env_blob("T_callingWay",templateParam->callingWay,strlen((const char*)templateParam->callingWay));     
    DBG("templateParam->callingWay = %s\r\n",templateParam->callingWay);    

    json_item = cJSON_GetObjectItem(templateMap, "offlineProcessing");
    templateParam->offlineProcessing = json_item->valueint;
//    memset(tmpbuf,0x00,sizeof(tmpbuf));
//    sprintf(tmpbuf,"%8d",templateParam->offlineProcessing);
//    ef_set_env_blob("offlineStatus",tmpbuf,8);     
    DBG("templateParam->offlineProcessing = %d\r\n",templateParam->offlineProcessing);     


    json_item = cJSON_GetObjectItem(templateMap, "modeType");
    strcpy((char *)templateParam->modeType,json_item->valuestring);

    //解析是否开启节假日模式
    memset(callingWay,0x00,sizeof(callingWay));
    callingWayNum = 0;
    split((char *)templateParam->modeType,",",callingWay,&callingWayNum); //调用函数进行分割 
    while(callingWayNum)
    {
        switch(atoi(callingWay[callingWayNum-1]))
        {
            case 1:
                templateParam->workMode.isPeakMode = 1;
                break;
            case 2:
                templateParam->workMode.isHolidayMode = 1;
                break;
            case 5:
                templateParam->workMode.isNormalMode = 1;
                break; 
        }
        callingWayNum-- ;  
    }  
    
//    ef_set_env_blob("modeType",templateParam->modeType,strlen((const char*)templateParam->modeType));     
    DBG("templateParam->modeType = %s\r\n",templateParam->modeType);

    json_item = cJSON_GetObjectItem(templateMap, "peakCallingWay");
    strcpy((char *)templateParam->peakInfo[0].callingWay,json_item->valuestring);

    //解析识别方式    
    memset(callingWay,0x00,sizeof(callingWay));
    callingWayNum = 0;
    split((char *)templateParam->peakInfo[0].callingWay,",",callingWay,&callingWayNum); //调用函数进行分割 
    while(callingWayNum)
    {
        switch(atoi(callingWay[callingWayNum-1]))
        {
            case 1:
                templateParam->peakCallingWay.isFace = 1;
                break;
            case 2:
                templateParam->peakCallingWay.isQrCode = 1;
                break;
            case 3:
                templateParam->peakCallingWay.isIcCard = 1;
                break; 
        }
        callingWayNum-- ;  
    }  
    
//    ef_set_env_blob("peakCallingWay",templateParam->peakInfo[0].callingWay,strlen((const char *)templateParam->peakInfo[0].callingWay));
    DBG("templateParam->peakInfo[0].callingWay = %s\r\n",templateParam->peakInfo[0].callingWay);

    json_item = cJSON_GetObjectItem(templateMap, "peakStartDate");
    strcpy((char *)templateParam->peakInfo[0].beginTime,json_item->valuestring);
//    ef_set_env_blob("peakStartDate",templateParam->peakInfo[0].beginTime,strlen((const char*)templateParam->peakInfo[0].beginTime));
    DBG("templateParam->peakInfo[0].beginTime = %s\r\n",templateParam->peakInfo[0].beginTime);

    json_item = cJSON_GetObjectItem(templateMap, "peakEndDate");
    strcpy((char *)templateParam->peakInfo[0].endTime,json_item->valuestring);
//    ef_set_env_blob("peakEndDate",templateParam->peakInfo[0].endTime,strlen((const char*)templateParam->peakInfo[0].endTime));
    DBG("templateParam->peakInfo[0].endTime = %s\r\n",templateParam->peakInfo[0].endTime);

    json_item = cJSON_GetObjectItem(templateMap, "peakHolidaysType");
    strcpy((char *)templateParam->peakInfo[0].outsideTimeMode,json_item->valuestring);
//    ef_set_env_blob("peakHolidaysType",templateParam->peakInfo[0].outsideTimeMode,strlen((const char*)templateParam->peakInfo[0].outsideTimeMode));
    DBG("templateParam->peakInfo[0].outsideTimeMode = %s\r\n",templateParam->peakInfo[0].outsideTimeMode);

    json_item = cJSON_GetObjectItem(templateMap, "peakHolidays");
    strcpy((char *)templateParam->peakInfo[0].outsideTimeData,json_item->valuestring);
//    ef_set_env_blob("peakHolidays",templateParam->peakInfo[0].outsideTimeData,strlen((const char*)templateParam->peakInfo[0].outsideTimeData));
    DBG("templateParam->peakInfo[0].outsideTimeData = %s\r\n",templateParam->peakInfo[0].outsideTimeData);    
//------------------------------------------------------------------------------
    json_item = cJSON_GetObjectItem(templateMap, "holidayCallingWay");
    strcpy((char *)templateParam->hoildayInfo[0].callingWay,json_item->valuestring);
//    ef_set_env_blob("holidayCallingWay",templateParam->hoildayInfo[0].callingWay,strlen((const char*)templateParam->hoildayInfo[0].callingWay));
    DBG("templateParam->hoildayInfo[0].callingWay = %s\r\n",templateParam->hoildayInfo[0].callingWay);

    json_item = cJSON_GetObjectItem(templateMap, "holidayStartDate");
    strcpy((char *)templateParam->hoildayInfo[0].beginTime,json_item->valuestring);
//    ef_set_env_blob("holidayStartDate",templateParam->hoildayInfo[0].beginTime,strlen((const char*)templateParam->hoildayInfo[0].beginTime));
    DBG("templateParam->hoildayInfo[0].beginTime = %s\r\n",templateParam->hoildayInfo[0].beginTime);

    json_item = cJSON_GetObjectItem(templateMap, "holidayEndDate");
    strcpy((char *)templateParam->hoildayInfo[0].endTime,json_item->valuestring);
//    ef_set_env_blob("holidayEndDate",templateParam->hoildayInfo[0].endTime,strlen((const char*)templateParam->hoildayInfo[0].endTime));
    DBG("templateParam->hoildayInfo[0].endTime = %s\r\n",templateParam->hoildayInfo[0].endTime);

    json_item = cJSON_GetObjectItem(templateMap, "holidayHolidaysType");
    strcpy((char *)templateParam->hoildayInfo[0].outsideTimeMode,json_item->valuestring);
//    ef_set_env_blob("holidayHolidaysType",templateParam->hoildayInfo[0].outsideTimeMode,strlen((const char*)templateParam->hoildayInfo[0].outsideTimeMode));
    DBG("templateParam->hoildayInfo[0].outsideTimeMode = %s\r\n",templateParam->hoildayInfo[0].outsideTimeMode);

    json_item = cJSON_GetObjectItem(templateMap, "holidayHolidays");
    strcpy((char *)templateParam->hoildayInfo[0].outsideTimeData,json_item->valuestring);
//    ef_set_env_blob("holidayHolidays",templateParam->hoildayInfo[0].outsideTimeData,strlen((const char*)templateParam->hoildayInfo[0].outsideTimeData));
    DBG("templateParam->hoildayInfo[0].outsideTimeData = %s\r\n",templateParam->hoildayInfo[0].outsideTimeData);       

//--------------------------------------------------
    //存储hoildayTimeMap中数据
    holidayTimeMapCnt = cJSON_GetArraySize(holidayTimeMap); /*获取数组长度*/
    DBG("array len = %d\r\n",holidayTimeMapCnt);

//    //存储不受控时间段的个数
//    if(holidayTimeMapCnt > 0)
//    {
//        sprintf(tmpKey,"%04d",holidayTimeMapCnt);
//        ef_set_env_blob("holidayTimeMapCnt",tmpKey,4);        
//    }

    for(index=0; index<holidayTimeMapCnt; index++)
    {
        tmpArray = cJSON_GetArrayItem(holidayTimeMap, index);

        memset(tmpKey,0x00,sizeof(tmpKey));
        memset(tmpIndex,0x00,sizeof(tmpIndex));       
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "templateType");
        templateParam->holidayMode[index].templateType = arrayElement->valueint;        
        DBG("templateType = %d\r\n",templateParam->holidayMode[index].templateType);

        arrayElement = cJSON_GetObjectItem(tmpArray, "voiceSize");
        templateParam->holidayMode[index].voiceSize = arrayElement->valueint;        
        DBG("voiceSize = %d\r\n",templateParam->holidayMode[index].voiceSize);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "modeType");
        templateParam->holidayMode[index].channelType = arrayElement->valueint;
        DBG("modeType = %d\r\n",templateParam->holidayMode[index].channelType);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "startTime");
        
        //因为节假日跟高峰共用，所以只记录到FLASH一种就可以了
        strcpy((char*)templateParam->holidayMode[index].startTime,arrayElement->valuestring);
        
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
//        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].startTime,strlen((const char*)templateParam->holidayMode[index].startTime));        
        DBG("%s = %s\r\n",tmpKey,templateParam->holidayMode[index].startTime);
        
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "endTime");
        strcpy((char*)templateParam->holidayMode[index].endTime,arrayElement->valuestring);  
        
        memset(tmpKey,0x00,sizeof(tmpKey));
        strcpy(tmpKey,"hoildayModeEndTime");
        strcat(tmpKey,tmpIndex);      
//        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].endTime,strlen((const char*)templateParam->holidayMode[index].endTime));                
        DBG("%s= %s\r\n",tmpKey,templateParam->holidayMode[index].endTime);        
    }
    
    DBG("=====================================================\r\n");
//--------------------------------------------------
    peakTimeMapCnt = cJSON_GetArraySize(peakTimeMap); /*获取数组长度*/
    DBG("peakTimeMapCnt len = %d\r\n",peakTimeMapCnt);

    for(index=0; index<peakTimeMapCnt; index++)
    {
        tmpArray = cJSON_GetArrayItem(peakTimeMap, index);

        memset(tmpKey,0x00,sizeof(tmpKey));
        memset(tmpIndex,0x00,sizeof(tmpIndex));       
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "templateType");
        templateParam->peakMode[index].templateType = arrayElement->valueint;        
        DBG("templateType = %d\r\n",templateParam->holidayMode[index].templateType);

        arrayElement = cJSON_GetObjectItem(tmpArray, "voiceSize");
        templateParam->peakMode[index].voiceSize = arrayElement->valueint;        
        DBG("voiceSize = %d\r\n",templateParam->holidayMode[index].voiceSize);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "modeType");
        templateParam->peakMode[index].channelType = arrayElement->valueint;
        DBG("modeType = %d\r\n",templateParam->holidayMode[index].channelType);
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "startTime");
        
        //因为节假日跟高峰共用，所以只记录到FLASH一种就可以了
        strcpy((char*)templateParam->peakMode[index].startTime,arrayElement->valuestring);
        sprintf(tmpIndex,"%d",index);
        strcpy(tmpKey,"hoildayModeStartTime");
        strcat(tmpKey,tmpIndex); 
        DBG("%s = %s\r\n",tmpKey,templateParam->peakMode[index].startTime);
        
        
        arrayElement = cJSON_GetObjectItem(tmpArray, "endTime");
        strcpy((char*)templateParam->peakMode[index].endTime,arrayElement->valuestring);  
        
        memset(tmpKey,0x00,sizeof(tmpKey));
        strcpy(tmpKey,"hoildayModeEndTime");
        strcat(tmpKey,tmpIndex);      
        DBG("%s= %s\r\n",tmpKey,templateParam->peakMode[index].endTime);        
        
    }
    
    cJSON_Delete(root);
    
    ret = optTemplateParam(templateParam,WRITE_PRARM,sizeof(TEMPLATE_PARAM_STRU),DEVICE_TEMPLATE_PARAM_ADDR); 
    DBG("saveTemplateParam took %d ms to save\r\n",xTaskGetTickCount()-curtick);

    if(ret != NO_ERR)
    {
        return FLASH_W_ERR;
    }    

    return result;
}


void initTemplateParam(void)
{
    uint8_t ret = 1;    
    
    ClearTemplateParam();
    ret = opParam(&gtemplateParam,READ_PRARM,sizeof(TEMPLATE_PARAM_STRU),DEVICE_TEMPLATE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");
        return;
    }

    DBG("gtemplateParam.initFlag.iFlag = %x\r\n",gtemplateParam.initFlag.iFlag);
    
	if(gtemplateParam.initFlag.iFlag != DEFAULT_INIVAL)
	{	    
        //模板数据赋值
        gtemplateParam.id = 1;
        gtemplateParam.templateStatus = 1;
        gtemplateParam.offlineProcessing = 2;
        gtemplateParam.initFlag.iFlag = DEFAULT_INIVAL;
        strcpy((char *)gtemplateParam.modeType,"5");
        strcpy((char *)gtemplateParam.templateCode,"100000");
        strcpy((char *)gtemplateParam.templateName,DEFAULT_DEV_NAME);
        strcpy((char *)gtemplateParam.callingWay,"1,2,3");      
      
        
        ret = opParam(&gtemplateParam,WRITE_PRARM,sizeof(TEMPLATE_PARAM_STRU),DEVICE_TEMPLATE_PARAM_ADDR);
        if(ret == false)
        {
            DBG("read param error\r\n");
            return;            
        }
	}  
	DBG("init Template Param success\r\n");
}

static uint8_t opParam(void *Param,uint8_t mode,uint32_t len,uint32_t addr)
{
    uint8_t ret = 1;

    char *buff = mymalloc(SRAMIN,len);

    if(buff == NULL)
    {
        DBG("malloc memery error\r\n");
        myfree(SRAMIN,buff);
        return false;
    }     

    if(mode == WRITE_PRARM)
    {
        memcpy(buff,Param,len);   
        
        bsp_MRAM_BufferWrite(buff, addr, len);

    	DBG("write param success\r\n");
	}
	else
	{        
        bsp_MRAM_BufferRead(buff, addr, len);
        
        memcpy (Param, buff,  len );  
        DBG("read param success\r\n");        
	}	

	myfree(SRAMIN,buff);
	
	return ret;
}




uint8_t optTemplateParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr)
{
    return opParam(stParam,mode,len,addr); 
}



//uint8_t optParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr,CallBackParam callBack)
//{
//    return callBack(stParam,mode,len,addr);
//}



void initDevBaseParam(void)
{
    uint8_t ret = 1; 
    char mac[6+1] = {0};
    char temp[20] = {0};
    char asc[12+1] = {0};
    
    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");
        return;
    }

    DBG("gDevBaseParam.deviceCode.downLoadFlag.iFlag = %x\r\n",gDevBaseParam.deviceCode.downLoadFlag.iFlag);

    if(gDevBaseParam.deviceCode.downLoadFlag.iFlag != DEFAULT_INIVAL)
	{	
	    DBG("wirte default base data\r\n");
	    ClearDevBaseParam();    

	    //设备工作模式
	    gDevBaseParam.progamMode = PROGRAMMODE_DOOR;

	    //门禁类型
	    gDevBaseParam.doorType = DOOR_TYPE_ONE;

	    //读卡器类型
	    gDevBaseParam.cardReaderType = CARD_READER_RS485;
	    
        //设备状态为启用状态
        gDevBaseParam.deviceState.iFlag = DEVICE_ENABLE;  
        
        gDevBaseParam.deviceCode.downLoadFlag.iFlag = DEFAULT_INIVAL;  
        
        calcMac ( (unsigned char*)mac);
        bcd2asc ( (unsigned char*)asc, (unsigned char*)mac, 12, 0 );
        Insertchar ( asc,temp,':' );
        

        gDevBaseParam.localIP.ipMode.iMode = DHCP_IP;
        
        gDevBaseParam.localIP.ip[0] = 192;
        gDevBaseParam.localIP.ip[1] = 168;
        gDevBaseParam.localIP.ip[2] = 0;
        gDevBaseParam.localIP.ip[3] = 176;
        
        gDevBaseParam.localIP.netMask[0] = 255;
        gDevBaseParam.localIP.netMask[1] = 255;
        gDevBaseParam.localIP.netMask[2] = 255;        
        gDevBaseParam.localIP.netMask[3] = 0; 
        
        gDevBaseParam.localIP.gateWay[0] = 192;
        gDevBaseParam.localIP.gateWay[1] = 168;
        gDevBaseParam.localIP.gateWay[2] = 0;
        gDevBaseParam.localIP.gateWay[3] = 1;

//        strcpy(temp,"00:00:00:58:36:39");
//        gDevBaseParam.deviceCode.deviceSnLen = strlen ( temp );
        
        gDevBaseParam.deviceCode.deviceSnLen = strlen ( temp )-1 ;
        memcpy ( gDevBaseParam.deviceCode.deviceSn,temp,gDevBaseParam.deviceCode.deviceSnLen);
        strcpy ( gDevBaseParam.mqttTopic.publish,DEV_FACTORY_PUBLISH );
        strcpy ( gDevBaseParam.mqttTopic.subscribe,DEV_FACTORY_SUBSCRIBE );
        strncat ( gDevBaseParam.mqttTopic.subscribe,gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen);
        memcpy ( gDevBaseParam.deviceCode.qrSn,asc+4,8); //使用后8位做为本机的SN

        DBG("gDevBaseParam.deviceCode.deviceSn = %s,len = %d\r\n",gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen);
        DBG("gDevBaseParam.mqttTopic.publish = %s\r\n",gDevBaseParam.mqttTopic.publish);
        DBG("gDevBaseParam.mqttTopic.subscribe = %s\r\n",gDevBaseParam.mqttTopic.subscribe);       
        DBG("gDevBaseParam.deviceCode.qrSn = %s\r\n",gDevBaseParam.deviceCode.qrSn);
        DBG("initDevBaseParam gDevBaseParam.localIP.ipMode.iMode = %x\r\n",gDevBaseParam.localIP.ipMode.iMode);
        DBG("2 gDevBaseParam.deviceCode.downLoadFlag.iFlag = %x\r\n",gDevBaseParam.deviceCode.downLoadFlag.iFlag);


        ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
        
        if(ret == false)
        {
            DBG("read param error\r\n");
            return;            
        }


        //重置索引值
        eraseUserDataIndex();
	}  
	
	DBG("init  DevBase Param success\r\n"); 
}


uint8_t optDevBaseParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr)
{
    return opParam(stParam,mode,len,addr); 
}

//=0 失败, =1 成功
uint8_t setLocalIpAddr(uint8_t *ipAddr,uint8_t * netMask,uint8_t *gateWay)
{
    uint8_t ret = 1; 

    if(!ipAddr || !netMask || !gateWay)
    {
        return 0;
    }
    
    ClearDevBaseParam();    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    gDevBaseParam.localIP.ipMode.iMode = STATIC_IP;    
    
    memcpy(gDevBaseParam.localIP.ip,ipAddr,4);
    memcpy(gDevBaseParam.localIP.netMask,netMask,4);
    memcpy(gDevBaseParam.localIP.gateWay,gateWay,4);    


    ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    DBG("setLocalIpAddr gDevBaseParam.localIP.ipMode.iMode = %x\r\n",gDevBaseParam.localIP.ipMode.iMode);
    
    if(ret == false)
    {
        DBG("read param error\r\n");
        return 0;            
    }  

    return 1;    
}

uint8_t resetLocalBaseParam(void)
{
    uint8_t ret = 1;

    ClearDevBaseParam();    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    
    gDevBaseParam.deviceCode.downLoadFlag.iFlag = 0x00; 
    
    ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    DBG("gDevBaseParam.deviceCode.downLoadFlag.iFlag = %x\r\n",gDevBaseParam.deviceCode.downLoadFlag.iFlag);
    
    if(ret == false)
    {
        DBG("read param error\r\n");
        return 0;            
    }  

    return ret;
}

uint8_t resetLocalSn(void)
{
    char tmp[32] = {0};
    uint8_t ret = 1;

    calcMac((uint8_t*)tmp);

    ClearDevBaseParam();    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    gDevBaseParam.deviceCode.downLoadFlag.iFlag = 0x00;
    gDevBaseParam.deviceCode.deviceSnLen = strlen((const char*)tmp);
    memset(gDevBaseParam.deviceCode.deviceSn,0x00,sizeof(gDevBaseParam.deviceCode.deviceSn));
    memcpy(gDevBaseParam.deviceCode.deviceSn,tmp+4,gDevBaseParam.deviceCode.deviceSnLen);
    
    ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    DBG("gDevBaseParam.deviceCode.deviceSn = %s\r\n",gDevBaseParam.deviceCode.deviceSn);
    
    if(ret == false)
    {
        DBG("read param error\r\n");
        return 0;            
    }  

    return 1;
}

uint8_t optRecordIndex(RECORDINDEX_STRU *recoIndex,uint8_t mode)
{
    uint8_t ret = 0;

    char buff[12] = {0};

    memset(buff,0x00,sizeof(buff));

    if(mode == WRITE_PRARM)
    {
        memcpy(buff,recoIndex,sizeof(RECORDINDEX_STRU));   

        DBG("write index %d,%d,%d\r\n",recoIndex->cardNoIndex,recoIndex->delCardNoIndex,recoIndex->accessRecoIndex);
        bsp_MRAM_BufferWrite(buff, RECORD_INDEX_ADDR, sizeof(RECORDINDEX_STRU));

	}
	else
	{
        bsp_MRAM_BufferRead(buff, RECORD_INDEX_ADDR, sizeof(RECORDINDEX_STRU));

        memcpy (recoIndex, buff,  sizeof(RECORDINDEX_STRU) );          
	}
	
	return ret;
}



void ClearRecordIndex(void)
{
	memset(&gRecordIndex,0x00,sizeof(RECORDINDEX_STRU)); 
}

void ClearTemplateParam(void)
{
	memset(&gtemplateParam,'\0',sizeof(TEMPLATE_PARAM_STRU));

	DBG("TEMPLATE_PARAM_STRU = %d\r\n",sizeof(TEMPLATE_PARAM_STRU));
}

void ClearDevBaseParam(void)
{
	memset(&gDevBaseParam,'\0',sizeof(DEV_BASE_PARAM_STRU));
}

void clearTemplateFRAM(void)
{
    bsp_MRAM_SectorErase(DEVICE_TEMPLATE_PARAM_ADDR,DEVICE_TEMPLATE_PARAM_SIZE);
}

static void eraseUserDataIndex ( void )
{
    gRecordIndex.cardNoIndex = 0;
    gRecordIndex.delCardNoIndex = 0;
    gRecordIndex.accessRecoIndex = 0;
    optRecordIndex(&gRecordIndex,WRITE_PRARM);
}


void eraseUserDataAll ( void )
{
	int32_t iTime1, iTime2;
	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
	eraseHeadSector();
	eraseDataSector();
    eraseUserDataIndex();
	clearTemplateFRAM();
    initTemplateParam();	
	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	DBG ( "eraseUserDataAll成功，耗时: %dms\r\n",iTime2 - iTime1 );
}

void eraseHeadSector ( void )
{	
    bsp_MRAM_SectorErase(0,122880);
}

void eraseDataSector ( void )
{
	uint16_t i = 0;
	uint16_t num = 0;
	
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

    num = gRecordIndex.accessRecoIndex * RECORD_MAX_LEN/CARD_SECTOR_SIZE + 1;

	for ( i=0; i<num; i++ )
	{
		bsp_sf_EraseSector ( ACCESS_RECORD_ADDR+i*CARD_SECTOR_SIZE );
	}

    gRecordIndex.accessRecoIndex = 0;
    
    optRecordIndex(&gRecordIndex,WRITE_PRARM);	
}

void initDevParam(void)
{
    //读取本地时间
    DBG("bsp_ds1302_readtime= %s\r\n",bsp_ds1302_readtime());

    //读取模板数据
//    eraseUserDataAll();

    initDevBaseParam();

    initTemplateParam();   

//    DisplayDevInfo();  
}


void TestFlash ( uint8_t mode )
{
	char buff[156] = {0};
    HEADINFO_STRU tmp;
	uint32_t addr = 0;
	uint16_t i = 0;
	uint32_t num = 0;
	
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		num = gRecordIndex.cardNoIndex;
	}

	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		num = gRecordIndex.delCardNoIndex;
	}

	
    DBG("&&&&& the total sn = %d\r\n\r\n",num);

	for ( i=0; i<num; i++ )
	{
        memset ( &tmp,0x00,sizeof ( HEADINFO_STRU ) );
		memset ( buff,0x00,sizeof ( buff ) );		
		
        bsp_MRAM_BufferRead(&tmp, addr+i*sizeof ( HEADINFO_STRU ), sizeof(HEADINFO_STRU));		

		DBG("card%d = %x,sn = %02x,%02x,%02x,%02x\r\n ",i,tmp.headData.id,tmp.headData.sn[0],tmp.headData.sn[1],tmp.headData.sn[2],tmp.headData.sn[3]);
	}

//	for ( i=0; i<num; i++ )
//	{
//		memset ( buff,0x00,sizeof ( buff ) );
//		bsp_sf_ReadBuffer ( ( uint8_t* ) buff, data_addr+i * ( sizeof ( USERDATA_STRU ) ), sizeof ( USERDATA_STRU ) );
//		printf ( "the %d data ====================== \r\n",i );
//		dbh ( "data", buff, ( sizeof ( USERDATA_STRU ) ) );

//	}

}
void setProgramMode(PROGRAM_MODE mode)
{
    uint8_t ret = 1; 

    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");
        return;
    }

    gDevBaseParam.progamMode = mode;
    
    ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    
    if(ret == false)
    {
        DBG("write param error\r\n");
        return;            
    }
}


void setCardReaderType(CARD_READER_TYPE type)
{
    uint8_t ret = 1; 

    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");
        return;
    }

    gDevBaseParam.cardReaderType = type;
    
    ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    
    if(ret == false)
    {
        DBG("write param error\r\n");
        return;            
    }

}
void setDoorType(DOOR_TYPE type)
{
    uint8_t ret = 1; 

    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");
        return;
    }

    gDevBaseParam.doorType = type;
    
    ret = opParam(&gDevBaseParam,WRITE_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);
    
    if(ret == false)
    {
        DBG("write param error\r\n");
    }   
}

PROGRAM_MODE getProgramMode()
{
    uint8_t ret = 1; 

    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");      
    }
    
    return gDevBaseParam.progamMode;
}
CARD_READER_TYPE getCardReaderType()
{
    uint8_t ret = 1; 

    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");      
    }
    
    return gDevBaseParam.cardReaderType;
}

DOOR_TYPE getDoorType()
{
    uint8_t ret = 1; 

    ClearDevBaseParam();
    
    ret = opParam(&gDevBaseParam,READ_PRARM,sizeof(DEV_BASE_PARAM_STRU),DEVICE_BASE_PARAM_ADDR);

    if(ret == false)
    {
        DBG("read param error\r\n");       
    }
    
    return gDevBaseParam.doorType;
}






