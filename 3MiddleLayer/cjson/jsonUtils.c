/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : jsonUtils.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月19日
  最近修改   :
  功能描述   : JSON数据处理C文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月19日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "jsonutils"
#include "elog.h"

#include "jsonUtils.h"
#include "eth_cfg.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
//LOCAL_USER_STRU gLoalUserData;



/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : modifyJsonItem
 功能描述  : 修改或者新增JSON数据包中指令item的值
 输入参数  : const char *srcJson   json数据包
             const char *item    需要更新值的key 
             const char *value   需要更新的value 
             uint8_t isSubitem   =1 是 data内数据 
             char *descJson      更新后的json数据包
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : 张舵
SYSERRORCODE_E modifyJsonItem(const char *srcJson,const char *item,const char *value,uint8_t isSubitem,char *descJson);
    修改内容   : 新生成函数

*****************************************************************************/
SYSERRORCODE_E modifyJsonItem(const uint8_t *srcJson,const uint8_t *item,const uint8_t *value,uint8_t isSubitem,uint8_t *descJson)
{
    cJSON *root ,*dataObj;
    char *tmpBuf;

    if(!srcJson)
    {
        cJSON_Delete(root);
        log_d("error json data\r\n");
        return STR_EMPTY_ERR;
    }   
    
    root = cJSON_Parse((char *)srcJson);    //解析数据包
    
    if (!root)  
    {  
        cJSON_Delete(root);
        my_free(tmpBuf);
        tmpBuf=NULL;        
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    if(isSubitem == 1)
    {
        //根据协议，默认所有的子项是data
        dataObj = cJSON_GetObjectItem ( root, "data" );         
        cJSON_AddStringToObject(dataObj,(const char*)item,(const char*)value);
    }
    else
    {
        cJSON_AddStringToObject(root,(const char*)item,(const char*)value);
    }  

    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        cJSON_Delete(root);
         my_free(tmpBuf);
        tmpBuf=NULL;        
        log_d("cJSON_PrintUnformatted error \r\n");
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);


//    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);
    tmpBuf=NULL;
    
    return NO_ERR;

}



/*****************************************************************************
 函 数 名  : GetJsonItem
 功能描述  : 获取JSON字符串中指定项目的值
 输入参数  : const char *jsonBuff  json字符串
           const char *item      要读到的KEY
           uint8_t isSubitem     是否读DATA内的项目，=1 读data内项目；=0 读root下项目
 输出参数  : 无
 返 回 值  : char *

 修改历史      :
  1.日    期   : 2019年12月20日
    作    者   : 张舵
 char *GetJsonItem(const char *jsonBuff,const char *item,uint8_t isSubitem)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem)
{
	static uint8_t value[JSON_ITEM_MAX_LEN] = {0};
	
	cJSON* root,*json_item,*dataObj;
	

//    log_d("<<<<%s>>>>\r\n",jsonBuff);

    if(strlen((const char*)jsonBuff) == 0 || strlen((const char*)jsonBuff) > JSON_ITEM_MAX_LEN )
    {
        cJSON_Delete(root);
        log_d ( "invalid data\r\n");       
		return NULL;
    }
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包

	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
		return NULL;
	}
	else
	{
        if(isSubitem == 1)
        {
            //根据协议，默认所有的子项是data
            dataObj = cJSON_GetObjectItem ( root, "data" );  
            json_item = cJSON_GetObjectItem ( dataObj, (const char*)item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, (const char*)item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//避免溢出
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}
			else
			{
			    strcpy ( (char*)value, json_item->valuestring );
			}
		}
		else if ( json_item->type == cJSON_Number )
		{
			sprintf ( (char*)value,"%d",json_item->valueint );
		}
//		else if( json_item->type == cJSON_Array )
//		{

//            //  2.日    期   : 2020年4月11日
//            //    作    者   :  
//            //    修改内容   : 添加对数组的支持，返回值还不完善    
//            tmpArrayNum = cJSON_GetArraySize(json_item);

//            for(int n=0;n<tmpArrayNum;n++)
//            {
//                arrayElement = cJSON_GetArrayItem(json_item, n);                 
//                strcpy ((char*)value, arrayElement->valuestring );            
//                log_d("cJSON_Array = %s\r\n",arrayElement->valuestring );
//            }
//		}
		else
		{
			log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
			return NULL;
		}

	}

    cJSON_Delete(root);

        
	return value;
}

SYSERRORCODE_E PacketDeviceInfo ( const uint8_t* jsonBuff,const uint8_t* descJson)
{
	SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*newroot,*dataObj,*json_cmdid,*json_devcode,*identification;
    char *tmpBuf;
    char buf[8] = {0};
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);            
		return CJSON_PARSE_ERR;
	}
	else
	{
        json_cmdid = cJSON_GetObjectItem ( root, "commandCode" );
        json_devcode = cJSON_GetObjectItem ( root, "deviceCode" );
        identification = cJSON_GetObjectItem ( root, "data" );

        newroot = cJSON_CreateObject();
        dataObj = cJSON_CreateObject();
        if(!newroot || !dataObj)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
    		return CJSON_CREATE_ERR;
        }

        cJSON_AddStringToObject(newroot, "commandCode", json_cmdid->valuestring);
        cJSON_AddStringToObject(newroot, "deviceCode", json_devcode->valuestring);
        cJSON_AddStringToObject(newroot, "identification", identification->valuestring);

        cJSON_AddItemToObject(newroot, "data", dataObj);

        cJSON_AddStringToObject(dataObj, "version", (const char*)gDevinfo.SoftwareVersion);
        cJSON_AddStringToObject(dataObj, "appName", (const char*)gDevinfo.Model);

        memset(buf,0x00,sizeof(buf));
        sprintf(buf,"%d",gRecordIndex.cardNoIndex);
        cJSON_AddStringToObject(dataObj, "regRersion", buf);
        cJSON_AddStringToObject(dataObj, "regface", " ");
        cJSON_AddStringToObject(dataObj, "ip", (const char*)gDevinfo.GetIP());
                
        tmpBuf = cJSON_PrintUnformatted(newroot); 

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");
            cJSON_Delete(root);
            cJSON_Delete(newroot);         
            my_free(tmpBuf);            
            return CJSON_FORMAT_ERR;
        }    

        strcpy((char *)descJson,tmpBuf);

	}

    cJSON_Delete(root);
    cJSON_Delete(newroot);

    my_free(tmpBuf);
    tmpBuf=NULL;        
    

    return result;
}

SYSERRORCODE_E upgradeDataPacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    char up_status[7] = {0};  
    uint8_t value[300] = {0};
    uint8_t send[300] = {0};
    
    ef_get_env_blob ( "upData", value, sizeof ( value ), NULL );


    strcpy(up_status,(const char*)ef_get_env("up_status"));

    
    //升级失败
    if(memcmp(up_status,"101700",6) == 0)
    {
        result = modifyJsonItem((const uint8_t *)value,(const uint8_t *)"status",(const uint8_t *)"2",0,send);

    }
    else if(memcmp(up_status,"101711",6) == 0) //升级成功
    {    
        result = modifyJsonItem((const uint8_t *)value,(const uint8_t *)"status",(const uint8_t *)"1",0,send);

    }
    else if(memcmp(up_status,"101722",6) == 0) //升级成功
    {
        result = modifyJsonItem((const uint8_t *)value,(const uint8_t *)"status",(const uint8_t *)"1",0,send);

    }
    else if(memcmp(up_status,"101733",6) == 0) //禁止升级
    {
        result = modifyJsonItem((const uint8_t *)value,(const uint8_t *)"status",(const uint8_t *)"3",0,send);

    }
    else
    {
        //无升级动作
    }    
 
    strcpy((char *)descBuf,(const char*)send);

    log_d("ack = %s\r\n",descBuf);

    return result;

}

SYSERRORCODE_E saveUpgradeData(uint8_t *jsonBuff)
{
    SYSERRORCODE_E result = NO_ERR;
    
    cJSON* root,*newroot,*tmpdataObj,*json_devCode,*productionModel,*id,*json_cmd;
    cJSON* version,*versionType;
    char *tmpBuf;
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
    if ( !root )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );        
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    }

    newroot = cJSON_CreateObject();
    if(!newroot)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newroot);
        my_free(tmpBuf);            
        return CJSON_PARSE_ERR;
    }  
    
    json_devCode = cJSON_GetObjectItem ( root, "deviceCode" );   
    json_cmd = cJSON_GetObjectItem ( root, "commandCode" ); 
    
    tmpdataObj = cJSON_GetObjectItem ( root, "data" );        
    productionModel = cJSON_GetObjectItem ( tmpdataObj, "url" );
    id = cJSON_GetObjectItem ( tmpdataObj, "id" );
    version = cJSON_GetObjectItem ( tmpdataObj, "version" );

    versionType = cJSON_GetObjectItem ( tmpdataObj, "type" ); 

    if(json_devCode)
        cJSON_AddStringToObject(newroot, "deviceCode", json_devCode->valuestring);

    if(json_cmd)
        cJSON_AddStringToObject(newroot, "commandCode", json_cmd->valuestring);
    
    if(productionModel)
        cJSON_AddStringToObject(newroot, "productionModel", productionModel->valuestring);

    if(id)
        cJSON_AddNumberToObject(newroot, "id", id->valueint);

    if(version)
        cJSON_AddStringToObject(newroot, "version", version->valuestring);


        
    if(versionType)
        cJSON_AddNumberToObject(newroot, "versionType", versionType->valueint);  
            
    tmpBuf = cJSON_PrintUnformatted(newroot); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");

        cJSON_Delete(root);
        cJSON_Delete(newroot);      
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    }

    log_d("upData = %s,len = %d\r\n",tmpBuf,strlen ((const char*)tmpBuf));
    ef_set_env_blob("upData",(const char*)tmpBuf,strlen ((const char*)tmpBuf));


    cJSON_Delete(root);
    cJSON_Delete(newroot);

    my_free(tmpBuf);
    tmpBuf=NULL;        
    
    return result;    
}


SYSERRORCODE_E getTimePacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    cJSON *root;  
    char *tmpBuf;
    
    root = cJSON_CreateObject();
    if (!root)  
    {          
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        cJSON_Delete(root);
        my_free(tmpBuf);
        return CJSON_PARSE_ERR;
    } 
    
    cJSON_AddStringToObject(root,"commandCode","88888");
    cJSON_AddStringToObject(root,"deviceCode",gDevBaseParam.deviceCode.deviceSn);
     
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(tmpBuf == NULL)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        my_free(tmpBuf);
        cJSON_Delete(root);        
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descBuf,tmpBuf);


    log_d("getTimePacket = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);
    tmpBuf=NULL;        

    return result;
}

//SYSERRORCODE_E getHeartbeatPacket(uint8_t *descBuf)
//{
//    SYSERRORCODE_E result = NO_ERR;
//    cJSON *root;  
//    char *tmpBuf;
//    
//    root = cJSON_CreateObject();
//    if (!root)  
//    {          
//        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
//        cJSON_Delete(root);
//        my_free(root);        
//        my_free(tmpBuf);
//        return CJSON_PARSE_ERR;
//    } 
//    
//    cJSON_AddStringToObject(root,"commandCode","99999");
//    cJSON_AddStringToObject(root,"deviceCode",gDevBaseParam.deviceCode.deviceSn);
//     
//    tmpBuf = cJSON_PrintUnformatted(root); 

//    if(tmpBuf == NULL)
//    {
//        log_d("cJSON_PrintUnformatted error \r\n");
//        my_free(tmpBuf);
//        cJSON_Delete(root);
//        my_free(root);        
//        
//        return CJSON_FORMAT_ERR;
//    }    

//    strcpy((char *)descBuf,tmpBuf);


//    log_d("heartbeat = %s\r\n",tmpBuf);

//    cJSON_Delete(root);
//    my_free(root);        

//    my_free(tmpBuf);
//    tmpBuf=NULL;        

//    return result;
//}




uint8_t* packetBaseJson(uint8_t *jsonBuff,char status)
{
    static uint8_t value[200] = {0};
    
	cJSON* root,*dataObj,*newroot,*json_cmdCode,*json_ownerId,*json_cardNo,*json_ownerType,*json_residentialId,*json_buildingId,*json_roomId,*json_identification,*json_userName;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
	
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        
        cJSON_Delete(root);        
        cJSON_Delete(newroot);
        my_free(tmpBuf);
        tmpBuf=NULL;        
        
		return NULL;
	}
	else
	{
        newroot = cJSON_CreateObject();   
        if(!newroot)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
            tmpBuf=NULL;        
            
    		return NULL;
        }

        dataObj = cJSON_GetObjectItem ( root, "data" );
        
        json_cmdCode = cJSON_GetObjectItem ( root, "commandCode" );
        
        json_ownerId = cJSON_GetObjectItem ( dataObj, "ownerId" );       
        json_cardNo = cJSON_GetObjectItem ( dataObj, "cardNo" );
        json_ownerType = cJSON_GetObjectItem ( dataObj, "ownerType" );
        json_residentialId = cJSON_GetObjectItem ( dataObj, "residentialId" );     
        json_buildingId = cJSON_GetObjectItem ( dataObj, "buildingId" );
        json_roomId = cJSON_GetObjectItem ( dataObj, "roomId" );   
        json_identification = cJSON_GetObjectItem ( dataObj, "identification" );  
        json_userName = cJSON_GetObjectItem ( dataObj, "userName" );

        if(json_cmdCode)
            cJSON_AddStringToObject(newroot, "commandCode", json_cmdCode->valuestring);     
            
        if(json_ownerId)
            cJSON_AddNumberToObject(newroot, "ownerId", json_ownerId->valueint);
            
        
        cJSON_AddStringToObject(newroot, "deviceCode",gDevBaseParam.deviceCode.deviceSn);

        if(json_cardNo)
            cJSON_AddStringToObject(newroot, "cardNo", json_cardNo->valuestring);     
        if(json_identification)
            cJSON_AddStringToObject(newroot, "identification", json_identification->valuestring);     
        if(json_userName)
            cJSON_AddStringToObject(newroot, "userName", json_userName->valuestring);           
        if(json_ownerType)
            cJSON_AddNumberToObject(newroot, "ownerType", json_ownerType->valueint);
        if(json_residentialId)
            cJSON_AddNumberToObject(newroot, "residentialId", json_residentialId->valueint);     
            
        if(json_buildingId)
            cJSON_AddNumberToObject(newroot, "buildingId", json_buildingId->valueint);

        if(json_roomId)
            cJSON_AddNumberToObject(newroot, "roomId", json_roomId->valueint); 
            
        
        if(status == 1)
            cJSON_AddStringToObject(newroot, "status", "1");
        else
            cJSON_AddStringToObject(newroot, "status", "0");          

        
        tmpBuf = cJSON_PrintUnformatted(newroot); 


        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");

            cJSON_Delete(root);
            cJSON_Delete(newroot);      
            my_free(tmpBuf);
            tmpBuf=NULL;        
            
            return NULL;
        }    

        strcpy((char *)value,tmpBuf);

	}
	
    my_free(tmpBuf);
	cJSON_Delete(root);
    cJSON_Delete(newroot);
    tmpBuf=NULL;  
    
    return value;    
}


//响应模板下发打包函数
uint8_t* packetRespTemplateJson(uint8_t *jsonBuff,char status)
{
    static uint8_t value[200] = {0};
    
	cJSON* root,*newroot,*json_cmdCode,*json_templateId,*dataObj,*json_qrId,*json_identification;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
	
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        
        cJSON_Delete(root);        
        cJSON_Delete(newroot);
        my_free(tmpBuf);
        tmpBuf=NULL;        
        
		return NULL;
	}
	else
	{
        json_cmdCode = cJSON_GetObjectItem ( root, "commandCode" );
        dataObj = cJSON_GetObjectItem ( root, "data" );  

        
        json_qrId = cJSON_GetObjectItem ( dataObj, "qrId" );  
        json_templateId = cJSON_GetObjectItem ( dataObj, "templateId" );
        json_identification = cJSON_GetObjectItem ( dataObj, "identification" );    

        newroot = cJSON_CreateObject();
   
        if(!newroot)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            my_free(tmpBuf);            
            tmpBuf=NULL;        
            
    		return NULL;
        }

        if(json_cmdCode)
            cJSON_AddStringToObject(newroot, "commandCode", json_cmdCode->valuestring);     
            
        if(json_qrId)
            cJSON_AddNumberToObject(newroot, "qrId", json_qrId->valueint);
            
        if(json_templateId)
            cJSON_AddNumberToObject(newroot, "templateId", json_templateId->valueint);
            
        if(json_identification)
            cJSON_AddStringToObject(newroot, "identification", json_identification->valuestring);            
            
        
        cJSON_AddStringToObject(newroot, "deviceCode",gDevBaseParam.deviceCode.deviceSn);
        
        if(status == 1)
            cJSON_AddStringToObject(newroot, "status", "1");
        else
            cJSON_AddStringToObject(newroot, "status", "0");          

        
        tmpBuf = cJSON_PrintUnformatted(newroot); 


        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");

            cJSON_Delete(root);
            cJSON_Delete(newroot);      
            my_free(tmpBuf);
            tmpBuf=NULL;        
            
            return NULL;
        }    

        strcpy((char *)value,tmpBuf);

	}
	
    my_free(tmpBuf);
	cJSON_Delete(root);
    cJSON_Delete(newroot);
    tmpBuf=NULL;  
    
    return value;    
}



SYSERRORCODE_E packetSingleAddCardJson(uint8_t *jsonBuff,char status,uint8_t *descBuf)
{ 
    SYSERRORCODE_E result = NO_ERR;

	cJSON* root,*newroot,*newDataObj,*dataObj,*json_cardNo,*json_ownerId;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
	
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        
        cJSON_Delete(root);        
        cJSON_Delete(newroot);
        my_free(tmpBuf);
        tmpBuf=NULL;        
        descBuf = NULL;
		return CJSON_PARSE_ERR;
	}
	else
	{
        //根据协议，默认所有的子项是data
        dataObj = cJSON_GetObjectItem ( root, "data" );  
        json_ownerId = cJSON_GetObjectItem ( dataObj, (const char*)"userId" );	
        json_cardNo = cJSON_GetObjectItem ( dataObj, (const char*)"cardNo" );   
        
        newroot = cJSON_CreateObject();        
        newDataObj = cJSON_CreateObject();
   
        if(!newroot || !newDataObj)        
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
            cJSON_Delete(newDataObj);
            my_free(tmpBuf);            
            tmpBuf=NULL;      
            descBuf = NULL;            
    		return CJSON_PARSE_ERR;
        }

        cJSON_AddItemToObject(newroot, "data", newDataObj);        
            
        if(json_ownerId)
            cJSON_AddNumberToObject(newDataObj, "ownerId", json_ownerId->valueint);

        if(json_cardNo)
            cJSON_AddStringToObject(newDataObj, "cardNo", json_cardNo->valuestring);
            
        
        if(status == 1)
            cJSON_AddStringToObject(newDataObj, "status", "1");
        else
            cJSON_AddStringToObject(newDataObj, "status", "0");

        cJSON_AddStringToObject(newroot, "commandCode", (const char*)"1034");   
        cJSON_AddStringToObject(newroot, "deviceCode",gDevBaseParam.deviceCode.deviceSn); 
        
        tmpBuf = cJSON_PrintUnformatted(newroot); 

//        log_d("packetBaseJson = %s\r\n",tmpBuf);

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");

            cJSON_Delete(root);
            cJSON_Delete(newroot);
            cJSON_Delete(newDataObj);    
            my_free(tmpBuf);
            tmpBuf=NULL;
            descBuf = NULL;            
            return CJSON_FORMAT_ERR;
        }    

        strcpy((char *)descBuf,tmpBuf);

	}
	
    my_free(tmpBuf);
	cJSON_Delete(root);
    cJSON_Delete(newroot);
    tmpBuf=NULL;  

    return result;
}


void GetCardArray ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t *num,uint8_t descBuff[][8])
{    
    cJSON* root,*json_item,*dataObj;
    cJSON* arrayElement;
    int tmpArrayNum = 0;
    int i = 0;
    
    root = cJSON_Parse ( ( char* ) jsonBuff );    //解析数据包
    
    if ( !root )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(root);
          return ;      
        //return NULL;
    }
    else
    {
        //根据协议，默认所有的子项是data
        dataObj = cJSON_GetObjectItem ( root, "data" );       
        
        //根据协议，默认所有的子项是data
        json_item = cJSON_GetObjectItem ( dataObj, "cardNo" );  
        
        if( json_item->type == cJSON_Array )
        {
            tmpArrayNum = cJSON_GetArraySize(json_item);
            
            log_d("cardArrayNum = %d\r\n",tmpArrayNum);
            
            //每个人最多20张卡
            if(tmpArrayNum > 20)
            {
                tmpArrayNum = 20;
            }

            *num = tmpArrayNum;           
         

            for(i=0;i<tmpArrayNum;i++)
            {
                arrayElement = cJSON_GetArrayItem(json_item, i);                 
                strcpy ((char *)descBuff[i], arrayElement->valuestring ); 
                log_d("result :%d = %s\r\n",i,descBuff[i]); 
            }

        }
        else if( json_item->type == cJSON_String )
        {
            //一般走到这里，卡号就是空的
            if(strlen((const char*)json_item->valuestring) == 0)
            {
                 *num = 0;
                log_d("card no is empty \r\n");
                cJSON_Delete(root);
                my_free(root);
                
                return ;      
            }
        
            tmpArrayNum = 1;
            *num = tmpArrayNum;

            
			if ( strlen ( json_item->valuestring ) > 8 )
			{
				memcpy ( descBuff[0], json_item->valuestring,8 );
			}
			else
			{
			    strcpy ((char*)descBuff[0], json_item->valuestring ); 
			}

			log_d ( "json_item =  %s\r\n",descBuff[0]);

            
            
        }
        else
        {
            *num = 0;
            log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
            my_free(root);
            
          return ;      
        //return NULL;
        }
        
    } 
    
    cJSON_Delete(root);
    my_free(root);
}



uint8_t packetCard(uint8_t *cardID,uint8_t *descJson)
{ 
    SYSERRORCODE_E result = NO_ERR;
	cJSON* root;
    char *tmpBuf;
    char tmpCardID[9] = {0};

    root = cJSON_CreateObject();
    
    if(!root)
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        my_free(tmpBuf);            
        tmpBuf=NULL;        
		return CJSON_CREATE_ERR;
    }

    log_d("3 cardid %02x,%02x,%02x,%02x\r\n",cardID[0],cardID[1],cardID[2],cardID[3]);
    

    cJSON_AddStringToObject(root, "deviceCode", gDevBaseParam.deviceCode.deviceSn);
    log_d("deviceCode = %s\r\n",gDevBaseParam.deviceCode.deviceSn);    

    cJSON_AddStringToObject(root, "commandCode","10044");//刷卡记录上送指令
    
    cJSON_AddNumberToObject(root, "enterType", 2);//刷卡
    
    if(gConnectStatus==1)
    {
        cJSON_AddNumberToObject(root, "status", ON_LINE);   
    }
    else
    {
        cJSON_AddNumberToObject(root, "status", OFF_LINE);   
    }
    cJSON_AddStringToObject(root, "currentTime",(const char*)bsp_ds1302_readtime());
    
    log_d("4 cardid %02x,%02x,%02x,%02x\r\n",cardID[0],cardID[1],cardID[2],cardID[3]);

    memset(tmpCardID,0x00,sizeof(tmpCardID));

    bcd2asc((uint8_t *)tmpCardID,cardID, 8, 1);

    log_d("CARD NO. = %s\r\n",tmpCardID);
    
    cJSON_AddStringToObject(root, "cardNo", (const char*)tmpCardID);

    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        cJSON_Delete(root);
        my_free(tmpBuf);            
        tmpBuf=NULL;        
        
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);    

    cJSON_Delete(root);
    my_free(tmpBuf);
    tmpBuf=NULL;        
    
    return result;

}


uint8_t packetRegister(uint8_t *descJson)
{
    SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*newRoot;
    char *tmpBuf;
    uint8_t text[200] = {0};
    uint8_t asc[400] = {0};
    uint8_t bcdBuf[150] = {0};
    int len = 0;
    int tmp_len = 0;
    uint8_t key[16] = {0};
    uint8_t i = 0;


    root = cJSON_CreateObject();    
    newRoot = cJSON_CreateObject();
    
    if(!root || !newRoot )
    {
        log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
        cJSON_Delete(newRoot);
        my_free(tmpBuf);            
        tmpBuf=NULL;        
		return CJSON_CREATE_ERR;
    }
    

    cJSON_AddStringToObject(root, "productKey", (const char*)gDevBaseParam.devKey.productKey);
    log_d("productKey = %s\r\n",gDevBaseParam.devKey.productKey);    

    cJSON_AddStringToObject(root, "deviceCode",gDevBaseParam.deviceCode.deviceSn);
    log_d("deviceSn = %s\r\n",gDevBaseParam.deviceCode.deviceSn);        
    
    cJSON_AddStringToObject(root, "ip",(const char*)gDevinfo.GetIP());
    log_d("ip = %s\r\n",gDevinfo.GetIP());   
    
    cJSON_AddStringToObject(root, "firmwareVersions",(const char*)gDevinfo.SoftwareVersion);
    log_d("firmwareVersions = %s\r\n",gDevinfo.SoftwareVersion);   
    
    cJSON_AddStringToObject(root, "mac",(const char*)gDevBaseParam.localIP.mac);
    log_d("mac = %s\r\n",gDevBaseParam.localIP.mac); 

    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        cJSON_Delete(root);
        my_free(tmpBuf);            
        tmpBuf=NULL;        
        
        return CJSON_FORMAT_ERR;
    }  

    len = strlen((const char*)tmpBuf);

    log_d("tmpbuf = %s,len=%d\r\n",tmpBuf,len);

    memcpy(bcdBuf,tmpBuf,len);
    
    //补齐16的倍数    
    tmp_len = 16 * ( (len + 15) / 16 );
    
    for(i=len;i<tmp_len;i++)
    {
        bcdBuf[i] = 0x00;
    }

    
//    dbh("bcdBuf",bcdBuf, tmp_len);

    asc2bcd(key, gDevBaseParam.devKey.pubKey, 32, 1);

    Des3_2(key,bcdBuf,tmp_len,text,0);
    
    dbh("Des3_2",text, tmp_len);
    
    bcd2asc(asc, text, tmp_len*2, 1);

//    dbh("enc txt",text, tmp_len);

    log_d("asc = %s,len=%d\r\n",asc,tmp_len*2);
    cJSON_Delete(root);
    

    cJSON_AddStringToObject(newRoot, "deviceSecret", gDevBaseParam.devKey.pubKey);
    log_d("deviceSecret = %s\r\n",gDevBaseParam.devKey.pubKey);    

    cJSON_AddStringToObject(newRoot, "encryptionText",asc);
    log_d("encryptionText = %s\r\n",asc);   


    tmpBuf = cJSON_PrintUnformatted(newRoot); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        cJSON_Delete(newRoot);
        my_free(tmpBuf);            
        tmpBuf=NULL;        
        
        return CJSON_FORMAT_ERR;
    }  


    len = strlen((const char*)tmpBuf);

    log_d("tmpbuf = %s,len=%d\r\n",tmpBuf,len);    

    strcpy((char *)descJson,tmpBuf);  
    
    cJSON_Delete(newRoot);
    my_free(tmpBuf);
    tmpBuf=NULL;        
    
    return result;

}




