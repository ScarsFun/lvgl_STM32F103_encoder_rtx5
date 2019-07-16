/**
 * @file XPT2046.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "encoder.h"
#include "drv_encoder.h"
#include <stdio.h>
#if USE_ENCODER

#include <stddef.h>
#include LV_DRV_INDEV_INCLUDE
#include LV_DRV_DELAY_INCLUDE

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the ENCODER
 */
void encoder_init(void)
{
  TIM4_init();
}

/**
 
 */
bool encoder_read (lv_indev_drv_t * drv, lv_indev_data_t*data) 
{
  data->enc_diff = TIM4_read();
  if(!enc_pressed()){
    data->state = LV_INDEV_STATE_PR;
  }
  else{
    data->state = LV_INDEV_STATE_REL;
  }
 // printf("ENC=%d BTN=%d\r\n",data->enc_diff, data->state);
  return false; /*No buffering so no more data read*/
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#endif
