/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : bsp_dipSwitch.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��23��
  ����޸�   :
  ��������   : ���뿪ͷ����ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __BSP_DIPSWITCH_H
#define __BSP_DIPSWITCH_H

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
 #include "sys.h" 

 

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
/* ���뿪�ض�Ӧ��RCCʱ�� */
#define RCC_ALL_DIPSWITCH     RCC_AHB1Periph_GPIOF     

#define GPIO_PORT_DIPSWITCH       GPIOF

     
#define GPIO_PIN_DIP0       GPIO_Pin_2
#define GPIO_PIN_DIP1       GPIO_Pin_3
#define GPIO_PIN_DIP2       GPIO_Pin_4
#define GPIO_PIN_DIP3       GPIO_Pin_5


#define GPIO_PIN_SW1       GPIO_Pin_2
#define GPIO_PIN_SW2       GPIO_Pin_15

//˿ӡ���ˣ�ͨ�������������ȷ
//#define DIP0      PFin(2)   	
//#define DIP1 		PFin(3)		
//#define DIP2 		PFin(4)		
//#define DIP3		PFin(5)		

#define DIP3        PFin(2)   	
#define DIP2 		PFin(3)		
#define DIP1 		PFin(4)		
#define DIP0		PFin(5)		


#define SW2_LOW()	    GPIO_ResetBits(GPIOG, GPIO_Pin_2)
#define SW2_HI()	    GPIO_SetBits(GPIOG, GPIO_Pin_2)

#define SW1_LOW()	    GPIO_ResetBits(GPIOD, GPIO_Pin_15)
#define SW1_HI()	    GPIO_SetBits(GPIOD, GPIO_Pin_15)




/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

void bsp_dipswitch_init(void);
int16_t bsp_dipswitch_read(void);
void bsp_sw_init(void);



#endif

