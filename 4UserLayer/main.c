/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ������ģ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�
*****************************************************************************
Ӳ����Դ���䣺
1.����1 ���Դ���
2.����2 RS485 ������2��
3.����3 RS485 ������1
4.���뿪��Ĭ��Ϊȫ1
 DIP0 = 1  �Ž�         = 0    ͨ��բ
 DIP1 = 1         = 0    
 DIP2 = 1         = 0    
 DIP3 = 1  ����ģʽ        = 0   ����ģʽ
******************************************************************************/


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "main"
#include "elog.h"

#include "def.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//�������ȼ� 
#define APPCREATE_TASK_PRIO		(tskIDLE_PRIORITY)
#define APPCREATE_STK_SIZE 		(configMINIMAL_STACK_SIZE*16)

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
const char *AppCreateTaskName = "vAppCreateTask"; 

//������
static TaskHandle_t xHandleTaskAppCreate = NULL;     

SemaphoreHandle_t gxMutex = NULL;
EventGroupHandle_t xCreatedEventGroup = NULL;
QueueHandle_t xCmdQueue = NULL; 
QueueHandle_t xCardIDQueue = NULL;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void AppTaskCreate(void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);
static void EasyLogInit(void);


int main(void)
{   
    //Ӳ����ʼ��
    bsp_Init(); 

    EasyLogInit();  
    
	/* ��������ͨ�Ż��� */
	AppObjCreate();
    
    //����AppTaskCreate����
    xTaskCreate((TaskFunction_t )AppTaskCreate,     
                (const char*    )AppCreateTaskName,   
                (uint16_t       )APPCREATE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )APPCREATE_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskAppCreate);   

    
    /* �������ȣ���ʼִ������ */
    vTaskStartScheduler();    
    
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
    //�����ٽ���
    taskENTER_CRITICAL();    

    //������ʼ��
    StartEthernet(); 
    
    //LED��
//    CreateLedTask();                //5 2

    //�����ư�ͨѶ  

    if(DIP0 == 1)
    {
        //�Ž�
        CreateOpenDoorTask();       //1 8        
    }
    else if(DIP0 == 0)
    {
        //ͨ��բ
        CreateCommTask();           //1 8     
    }
    else
    {
        //�Ž�
        CreateOpenDoorTask();       //1 8      
    }

    //Τ��������
    //CreateReaderTask();         //0   1    

    //RS485������
    CreateRs485ReaderTask();     // 2 1    


    //�����ݴ���
    CreateDataProcessTask();        //3   6
    

    //MQTTͨѶ
    CreateMqttTask();               //4   5

    //���Ź� ֻҪ�ڹ���ģʽ�²�����
    //if(gDevBaseParam.progamMode != PROGRAMMODE_TEST)
    if(DIP3 == 1)
    {
        CreateWatchDogTask();
    }
    
    //ɾ������
    vTaskDelete(xHandleTaskAppCreate); //ɾ��AppTaskCreate����

    //�˳��ٽ���
    taskEXIT_CRITICAL();   

}




/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* �����¼���־�� */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("�����¼���־��ʧ��\r\n");
    }

	/* ���������ź��� */
    gxMutex = xSemaphoreCreateMutex();
	
	if(gxMutex == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("���������ź���ʧ��\r\n");
    }    

    //����Ϣ���У����ˢ���򿨺��·�����
    xCardIDQueue = xQueueCreate((UBaseType_t ) CARD_QUEUE_LEN,/* ��Ϣ���еĳ��� */
                              (UBaseType_t ) sizeof(READER_BUFF_STRU *));/* ��Ϣ�Ĵ�С */
    if(xCardIDQueue == NULL)
    {
        App_Printf("create xCardIDQueue error!\r\n");
    }
    

//    if(gDevBaseParam.progamMode == PROGRAMMODE_DOOR || gDevBaseParam.progamMode == PROGRAMMODE_TEST)   
//    {
//        xCmdQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
//                                  (UBaseType_t ) QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
//        if(xCmdQueue == NULL)
//        {
//            App_Printf("create xCmdQueue error!\r\n");
//        }
//    }
//    else if(gDevBaseParam.progamMode == PROGRAMMODE_CHANNEL)
//    {
//        xCmdQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
//                                  (UBaseType_t ) sizeof(CMD_BUFF_STRU *));/* ��Ϣ�Ĵ�С */
//        if(xCmdQueue == NULL)
//        {
//            App_Printf("create xCmdQueue error!\r\n");
//        }
//    }

    if ( DIP0 == 0 ) //ͨ��բ
    {
        xCmdQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                                  (UBaseType_t ) sizeof(CMD_BUFF_STRU *));/* ��Ϣ�Ĵ�С */
        if(xCmdQueue != NULL)
        {
            App_Printf("create xCmdQueue success!\r\n");
        }
    }
    else
    {
        xCmdQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                                  (UBaseType_t ) QUEUE_SIZE);/* ��Ϣ�Ĵ�С */
        if(xCmdQueue != NULL)
        {
            App_Printf("create xCmdQueue success!\r\n");
        }
    }


    
}



/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
    char  buf_str[512 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* �����ź��� */
	xSemaphoreTake(gxMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(gxMutex);
}




static void EasyLogInit(void)
{
    /* initialize EasyLogger */
     elog_init();
     /* set EasyLogger log format */
     elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
     elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_TIME);

     
     /* start EasyLogger */
     elog_start();  
}




