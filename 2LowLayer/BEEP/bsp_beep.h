#ifndef __bsp_BEEP_H
#define __bsp_BEEP_H	 
#include "sys.h"
#include "stdlib.h"	  
#include "delay.h"

//LED�˿ڶ���

//#define RCC_ALL_BEEP     (RCC_AHB1Periph_GPIOF)

//#define GPIO_PORT_BEEP    GPIOF
//#define GPIO_PIN_BEEP    GPIO_Pin_8


#define RCC_ALL_BEEP     (RCC_AHB1Periph_GPIOE)

#define GPIO_PORT_BEEP    GPIOE
#define GPIO_PIN_BEEP    GPIO_Pin_3





//LED�˿ڶ���
//#define BEEP PFout(8)	// ����������IO 
#define BEEP PEout(3)	// ����������IO 


void bsp_beep_init(void);//��ʼ��		 		


void Sound(u16 frq);
void Sound2(u16 time);
void play_music(void);
void play_successful(void);
void play_failed(void);


#endif

















