/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Key_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��25��
  ����޸�   :
  ��������   : ������������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��25��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "key_task.h"
#include "bsp_key.h"
#include "bsp_dipSwitch.h"
#include "bsp_ds1302.h"
#include "easyflash.h"
#include "tool.h"
#include "bsp_beep.h"
#include "localdata.h"
#include "test.h"


#define LOG_TAG    "keyTask"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define KEY_STK_SIZE        (configMINIMAL_STACK_SIZE)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *keyTaskName = "vKeyTask";     

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskKey = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskKey(void *pvParameters);
//static void check_msg_queue(void);



void CreateKeyTask(void)
{
    //����
    xTaskCreate((TaskFunction_t )vTaskKey,         
                (const char*    )keyTaskName,       
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )KEY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskKey); 
}

static void vTaskKey(void *pvParameters)
{    
	uint8_t ucKeyCode;
	
    uint32_t g_memsize;


    while(1)
    {
        ucKeyCode = bsp_Key_Scan(0);      


		if (ucKeyCode != KEY_NONE)
		{              
			switch (ucKeyCode)
			{
				/* K1������ ��ӡ����ִ����� */
				case KEY0_PRES:	             
                    log_d("KEY0_PRES\r\n");
					break;				
				/* K2�����£���ӡ���ڲ������� */
				case KEY1_PRES:                 
                    log_d("KEY1_PRES\r\n");//			        
					break;         
				
				/* �����ļ�ֵ������ */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}
		
		vTaskDelay(20);
	}   

}


//��ѯMessage_Queue�����е��ܶ���������ʣ���������
//void check_msg_queue(void)
//{
//    
//	u8 msgq_remain_size;	//��Ϣ����ʣ���С
//    u8 msgq_total_size;     //��Ϣ�����ܴ�С
//    
//    taskENTER_CRITICAL();   //�����ٽ���
//    msgq_remain_size=uxQueueSpacesAvailable(xCmdQueue);//�õ�����ʣ���С
//    msgq_total_size=uxQueueMessagesWaiting(xCmdQueue)+uxQueueSpacesAvailable(xCmdQueue);//�õ������ܴ�С���ܴ�С=ʹ��+ʣ��ġ�
//	printf("Total Size = %d, Remain Size = %d\r\n",msgq_total_size,msgq_remain_size);	//��ʾDATA_Msg��Ϣ�����ܵĴ�С

//    taskEXIT_CRITICAL();    //�˳��ٽ���
//}






