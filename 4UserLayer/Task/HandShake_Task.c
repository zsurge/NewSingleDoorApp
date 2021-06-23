/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : HandShake_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��26��
  ����޸�   :
  ��������   : ����������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��26��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
 *----------------------------------------------*/
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define HANDSHAKE_STK_SIZE  (configMINIMAL_STACK_SIZE*2)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *handShakeTaskName = "vHandShakeTask";      //���Ź�������


/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskHandShake = NULL;      //LED��


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskHandShake(void *pvParameters);



static void vTaskHandShake(void *pvParameters)
{

    //��ȡ����ʱ��
    log_i("bsp_ds1302_readtime= %s\r\n",bsp_ds1302_readtime());

    //��ȡģ������

//  eraseUserDataAll();
    
    initRecordIndex();
    
    initDevBaseParam();

    initTemplateParam();
    
    vTaskDelay(500);
    vTaskDelete( NULL ); //ɾ���Լ�
}

void CreateHandShakeTask(void)
{  
    //��android����
    xTaskCreate((TaskFunction_t )vTaskHandShake,
    (const char*    )handShakeTaskName,       
    (uint16_t       )HANDSHAKE_STK_SIZE, 
    (void*          )NULL,              
    (UBaseType_t    )HANDSHAKE_TASK_PRIO,    
    (TaskHandle_t*  )&xHandleTaskHandShake); 
}







