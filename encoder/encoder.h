/**
 * @file XPT2046.h
 *
 */

#ifndef ENCODER_H
#define ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "lv_drv_conf.h"
#endif

#if USE_ENCODER

#include <stdint.h>
#include <stdbool.h>
#include "../lvgl/src/lv_hal/lv_hal_indev.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void encoder_init(void);
bool encoder_read(lv_indev_drv_t * drv, lv_indev_data_t*data);

/**********************
 *      MACROS
 **********************/

#endif /* USE_XPT2046 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XPT2046_H */
