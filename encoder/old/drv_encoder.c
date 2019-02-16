#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "drv_encoder.h"
#include <stdio.h>

static int16_t encoder_count=0;

void TIM4_init(void){
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    //
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 |  GPIO_Pin_5;   //PB5 for shaft button
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
    //debounce filter

    TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICFilter=0x06;
    //TIM_ICInitStructure.TIM_ICPrescaler=TIM_ICPSC_DIV4;
    //TIM_ICInitStructure.TIM_ICSelection=TIM_ICSelection_DirectTI;
   //TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInit(TIM4,&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel=TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICFilter=0x06;
	  //TIM_ICInitStructure.TIM_ICPrescaler=TIM_ICPSC_DIV4;
    //TIM_ICInitStructure.TIM_ICSelection=TIM_ICSelection_DirectTI;
    //TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM4,&TIM_ICInitStructure);

     // Setup TIM4 for encoder input
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12,TIM_ICPolarity_Rising, TIM_ICPolarity_Falling);
    TIM4->EGR = 1;           // Generate an update event
    
   
    TIM_Cmd(TIM4, ENABLE);
}

uint8_t enc_pressed (void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
}

int16_t TIM4_read(void){
  encoder_count = (int16_t)TIM4->CNT ;
  TIM4->CNT = 0;
	printf("TIM:%d", encoder_count);
  return encoder_count/4;
 
}
