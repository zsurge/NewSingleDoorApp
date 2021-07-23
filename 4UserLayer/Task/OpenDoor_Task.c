/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : OpenDoor_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��28��
  ����޸�   :
  ��������   : ������ͨѶ���������ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "OpenDoor_Task.h"
#include "bsp_dipSwitch.h"
#include "bsp_time.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 
#define OPENDOOR_TASK_PRIO		(tskIDLE_PRIORITY + 8) 
#define OPENDOOR_STK_SIZE 		(configMINIMAL_STACK_SIZE*2)


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *OpenDoorTaskName = "vOpenDoorTask"; 

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskOpenDoor = NULL;  

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskOpenDoor(void *pvParameters);



void CreateOpenDoorTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskOpenDoor,         
                (const char*    )OpenDoorTaskName,       
                (uint16_t       )OPENDOOR_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )OPENDOOR_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskOpenDoor);
}               



static void vTaskOpenDoor(void *pvParameters)
{ 
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(50); /* �������ȴ�ʱ��Ϊ100ms */     

    uint32_t devID = 0;

    while(1)
    { 
        if(gOpenDoorTimer.flag == 1 && gOpenDoorTimer.outTimer == 0)
        {
            gOpenDoorTimer.flag = 0;
            log_d("close 1 door\r\n");
            SW1_HI();                    
        }

        if(gOpenDoor2Timer.flag ==1 && gOpenDoor2Timer.outTimer == 0)
        {
            gOpenDoor2Timer.flag = 0;
            log_d("close 2 door\r\n");
            SW2_HI();                    
        }          

        xReturn = xQueueReceive( xCmdQueue,    /* ��Ϣ���еľ�� */
                                 (void *)&devID,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE == xReturn)
        {            
            log_d("recv id = %d\r\n",devID);
            
            if(devID == READER1)
            {            
                log_d("open 1 door\r\n");
                SW1_LOW();  
            }
            else if(devID == READER2)
            {
                log_d("open 2 door\r\n");
                SW2_LOW();
            }   
        }
        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);   
        vTaskDelay(30); 
        
    }  
}



