#include "bsp_led.h" 
#include "string.h"


//��ʼ��PF9��PF10Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void bsp_LED_Init(void)
{    	 
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_ALL_LED, ENABLE);//ʹ��GPIOFʱ��
    
      //GPIOFerr
      GPIO_InitStructure.GPIO_Pin = GPIO_PIN_ERRORLED;   //LED1��ӦIO��
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //��ͨ���ģʽ
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          //�������
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //100MHz
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
      GPIO_Init(GPIO_PORT, &GPIO_InitStructure);         //��ʼ��GPIO
      
      GPIO_SetBits(GPIO_PORT,GPIO_PIN_ERRORLED);
}


