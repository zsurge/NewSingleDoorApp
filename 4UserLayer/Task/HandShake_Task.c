/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : HandShake_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月26日
  最近修改   :
  功能描述   : 握手任务处理
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月26日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "handshake_task.h"
#include "easyflash.h"
#include "stdlib.h"
#include "tool.h"
#include "bsp_ds1302.h"
#include "ini.h"


#define LOG_TAG    "handShake"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define HANDSHAKE_STK_SIZE  (configMINIMAL_STACK_SIZE*2)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *handShakeTaskName = "vHandShakeTask";      //看门狗任务名


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskHandShake = NULL;      //LED灯


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskHandShake(void *pvParameters);



static void vTaskHandShake(void *pvParameters)
{

    //读取本地时间
    log_i("bsp_ds1302_readtime= %s\r\n",bsp_ds1302_readtime());

    //读取模板数据

//  eraseUserDataAll();
    
    initRecordIndex();
    
    initDevBaseParam();

    initTemplateParam();
    
    vTaskDelay(500);
    vTaskDelete( NULL ); //删除自己
}

void CreateHandShakeTask(void)
{  
    //跟android握手
    xTaskCreate((TaskFunction_t )vTaskHandShake,
    (const char*    )handShakeTaskName,       
    (uint16_t       )HANDSHAKE_STK_SIZE, 
    (void*          )NULL,              
    (UBaseType_t    )HANDSHAKE_TASK_PRIO,    
    (TaskHandle_t*  )&xHandleTaskHandShake); 
}







