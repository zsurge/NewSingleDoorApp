/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ����Ӳ���ײ�������������ļ���ÿ������ #include "bsp.h" ���������е���������ģ�顣
            bsp = Borad surport packet �弶֧�ְ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "bsp.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define HARDWARE_VERSION               "V1.0.1"
#define SOFTWARE_VERSION               "V1.0.1"

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

static void my_mem_init(void)
{
	mymem_init(SRAMIN);								//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);								//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM);	  					    //��ʼ��CCM�ڴ��
}

 static void DisplayDevInfo(void);
 
 
 
 static void DisplayDevInfo(void)
 {
     printf("\r\n==========Version==========\r\n");
     printf("Softversion :%s\r\n",gDevinfo.SoftwareVersion);
     printf("HardwareVersion :%s\r\n", gDevinfo.HardwareVersion);
     printf("Model :%s\r\n", gDevinfo.Model);
     printf("ProductBatch :%s\r\n", gDevinfo.ProductBatch);      
     printf("BulidDate :%s\r\n", gDevinfo.BulidDate);
     printf("DevSn :%s\r\n", gDevinfo.GetSn());
     printf("Devip :%s\r\n", gDevinfo.GetIP());
     printf("DevID :%s\r\n", gDevBaseParam.deviceCode.qrSn);

     printf("bit 0 = %d,bit 1 = %d,bit 2 = %d,bit 3 = %d\r\n",DIP0,DIP1,DIP2,DIP3);
 }

 void bsp_Init(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x30000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);            //��ʼ����ʱ����

    bsp_dipswitch_init();
    
	bsp_TIM6_Init();            //��ʱ��6��ʼ��
	
    bsp_InitUart();

    bsp_ds1302_init();            //ʱ��оƬ��ʼ��    
    
	bsp_LED_Init();		        //��ʼ��LED�˿�	  
	
    bsp_MRAM_Init();           //MR25
    
    STM_FLASH_Init();           //оƬ�ڲ�FLASH��ʼ��
    
    easyflash_init();           //�ⲿFLASH��ʼ����ʹ��easyflash       
    
    bsp_beep_init();            //��������ʼ��

    bsp_WiegandInit();          //Τ����������ʼ��

    bsp_sw_init();


    my_mem_init();                  //���ڴ���г�ʼ��  
    
    initDevParam();
    
    DisplayDevInfo();
  /* CmBacktrace initialize */
//   cm_backtrace_init("SingleDoorApp", HARDWARE_VERSION, SOFTWARE_VERSION);

}

