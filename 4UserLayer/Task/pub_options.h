/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : pub_options.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : FreeRTOS中事件及通知等公共参数的定义
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
#ifndef __PUB_OPTIONS_H__
#define __PUB_OPTIONS_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

//使用拨码开关来区分工作模式
//bit1	ON: 通道          OFF: 门禁
//bit2	ON: 二门门禁	    OFF:一门门禁
//bit3	ON: 测试模式	    OFF：工作模式
//bit4	ON: 485读卡器	    OFF:韦根读卡器




 
#define  QUEUE_LEN    10     /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4      /* 队列的中每个消息的大小*/

#define  CARD_QUEUE_LEN    10     /* 队列的长度，最大可包含多少个消息 */

#define READER1         1           
#define READER2         2

#define READMODE        1 
#define SORT_CARD_MODE 2
#define REMOTE_OPEN_MODE 3
#define DEL_CARD_MODE 4
#define MANUAL_SORT     (10)

    
//事件标志
#define TASK_BIT_0	 (1 << 0)
#define TASK_BIT_1	 (1 << 1)
#define TASK_BIT_2	 (1 << 2)
#define TASK_BIT_3	 (1 << 3)
#define TASK_BIT_4	 (1 << 4)
#define TASK_BIT_5	 (1 << 5)   
    
#define TASK_BIT_ALL ( TASK_BIT_0 | TASK_BIT_1| TASK_BIT_2)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

#define MAX_CMD_LEN

 typedef struct CMD_BUFF
 {
    uint8_t cmd_len; 
    uint8_t cmd[32];
 }CMD_BUFF_STRU;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/ 
//事件句柄
extern EventGroupHandle_t xCreatedEventGroup;
extern SemaphoreHandle_t gxMutex;
extern QueueHandle_t xCmdQueue; 
extern QueueHandle_t xCardIDQueue; 




extern CMD_BUFF_STRU gCmd_buff;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/



#endif /* __PUB_OPTIONS_H__ */

