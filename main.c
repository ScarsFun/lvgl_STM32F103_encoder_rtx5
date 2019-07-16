/*-------------------------------------------------------------------------- *
 *                                                                           * 
 *                                                                           *
 *               ____                     _____                              *
 *              / ___|  ___ __ _ _ __ ___|  ___|   _ _ __                    *
 *              \___ \ / __/ _` | '__/ __| |_ | | | | '_ \                   *
 *               ___) | (_| (_| | |  \__ \  _|| |_| | | | |                  *
 *              |____/ \___\__,_|_|  |___/_|   \__,_|_| |_|                  *
 *                                                                           *
 *                                                                           *
 *   15/07/2019                                                              * 
 *                                                                           * 
 *                                                                           * 
 *   https://github.com/ScarsFun                                             * 
 *   littleVgl DEMO.                                                         * 
 *                                                                           *
 *   LittleVgl (6.0)                                                         *
 *   Custom STM32F103RC Board (72Mhz, 64K ram, 256K Flash) .                 * 
 *   ILI9341 display over SPI with DMA. rotary encoder.                      *
 *   KEIL RTX v5.                                                            * 
 *   LVGL event driven refresh. Standby after 10 Secs Encoder inactivity.    *
 *                                                                           * 
 *                                                                           * 
 *                                                                           * 
 *                                                                           * 
 *       ILI9341 SPI CONNECTIONS:                                            * 
 *       TFT_RESET                   PA2                                     * 
 *       TFT_DC                      PA3                                     * 
 *       TFT_CS                      PA4                                     * 
 *       TFT_SCK                     PA5                                     * 
 *       TFT_MISO                    PA6                                     * 
 *       TFT_MOSI                    PA7                                     * 
 *                                                                           * 
 *       ROTARY ENCODER CONNECTIONS:                                         * 
 *       CLK                         PB7                                     * 
 *       DT                          PB6                                     *
 *       SWITCH                      PB5                                     *
 *                                                                           * 
 *       PWM LED                     PB0                                     * 
 *       Blinky LED                  PB8                                     * 
 *                                                                           * 
 *                                                                           * 
 *---------------------------------------------------------------------------*/

#define osObjectsPublic // define objects in main module
#include <stdlib.h>
#include "cmsis_os2.h"
#include "stm32f10x.h"
#include "lvgl/lvgl.h"
#include "ili9341/core.h"
#include "ili9341/lv_driver.h"
#include "encoder/encoder.h"
#include "pwm/pwm.h"
#include "misc/led.h"

static lv_group_t* g; //An Object Group
static lv_indev_t* encoder_indev; //The input device
const uint32_t led_intensity[10] = { 1, 14, 68, 207, 490, 990, 1793, 2998, 4718, 70199 };

lv_obj_t* chart1, *slider, *spinbox;
lv_chart_series_t* dl2_1;

const osThreadAttr_t lvgl_refresh_Thread_attr = {
    .stack_size = 1024 // Create the thread stack with a size of 1024 bytes
};

const osThreadAttr_t main_app_attr = {
    .stack_size = 1280 // Create the thread stack with a size of 1024 bytes
};

void timer1_callback(void* param);
void lvgl_refresh_Thread(void* argument);

osThreadId_t T_lvgl_refresh_Thread;
osMutexId_t lvgl_upd_mutex;
osTimerId_t timer0_id, timer1_id;
osEventFlagsId_t LVGL_rfr_evt_id;
osStatus_t status;

void button_event_cb(lv_obj_t* btn, lv_event_t event)
{
    // button is clicked
    if (event == LV_EVENT_CLICKED) {
			if (lv_btn_get_state(btn) == 0)
		{
        TIM3->CCR3 = 0;
        lv_group_remove_obj(slider);
        lv_obj_set_hidden(slider, true);
    }
    else {

        TIM3->CCR3 = led_intensity[lv_slider_get_value(slider) - 1];
        lv_group_add_obj(g, slider);
        lv_obj_set_hidden(slider, false);
    }
	}
}

static void slider_event_cb(lv_obj_t* slider, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
        TIM3->CCR3 = led_intensity[lv_slider_get_value(slider) - 1];
}

static void checkb_event_cb(lv_obj_t* checkb, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) //A check box is clicked
    {
        if (lv_cb_is_checked(checkb)) {
            status = osTimerStart(timer0_id, 1000);
        }
        else
            osTimerStop(timer0_id);
    }
}

static void spinbox_event_cb(lv_obj_t* spinbox, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED) {
        uint16_t spinbox_value = lv_spinbox_get_value(spinbox);
        if (spinbox_value == 0) {
            osTimerStop(timer1_id);
            GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
        }
        else {
            osTimerStop(timer1_id);
            osTimerStart(timer1_id, spinbox_value * 50);
        }
    }
}

static void gui_create(void)
{
    lv_theme_t* th = lv_theme_zen_init(240, NULL);
    lv_theme_set_current(th);
    lv_obj_t* scr = lv_cont_create(NULL, NULL);
    lv_scr_load(scr);

    // Create Power toggle button
    lv_obj_t* btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_size(btn, 30, 30);
    lv_btn_set_toggle(btn, true);
    lv_obj_set_event_cb(btn, button_event_cb);
    lv_obj_t* btn_label = lv_label_create(btn, NULL);
    lv_label_set_text(btn_label, LV_SYMBOL_POWER);
    lv_group_add_obj(g, btn);
    lv_obj_set_pos(btn, 15, 15);

    //Create a slider
    slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_size(slider, 120, 25);
    lv_obj_set_event_cb(slider, slider_event_cb);
    lv_obj_align(slider, btn, LV_ALIGN_OUT_RIGHT_TOP, 30, 5);
    lv_bar_set_range(slider, 1, 10);
    lv_obj_set_hidden(slider, true);
		
    static lv_style_t spinBoxStyle;
    lv_style_copy(&spinBoxStyle, th->style.spinbox.bg);
    spinBoxStyle.text.font = &lv_font_roboto_28;

    spinbox = lv_spinbox_create(lv_scr_act(), NULL);
    lv_spinbox_set_style(spinbox, LV_SPINBOX_STYLE_BG, &spinBoxStyle);

    lv_spinbox_set_digit_format(spinbox, 2, 0);
    lv_spinbox_set_range(spinbox, 0, 99);
    lv_obj_set_size(spinbox, 110, 55);
    lv_obj_align(spinbox, slider, LV_ALIGN_OUT_RIGHT_TOP, 10, -10);
    lv_obj_set_event_cb(spinbox, spinbox_event_cb);
    lv_group_add_obj(g, spinbox);

    lv_obj_t* checkb = lv_cb_create(lv_scr_act(), NULL); //check box
    lv_cb_set_text(checkb, "GRAPH");
    lv_group_add_obj(g, checkb); //Add to the group
    lv_obj_set_event_cb(checkb, checkb_event_cb);
    lv_obj_align(checkb, btn, LV_ALIGN_IN_BOTTOM_LEFT, 0, 40);

    lv_obj_t* label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "LittleVgl V6.0");
    lv_obj_align(label, checkb, LV_ALIGN_IN_BOTTOM_LEFT, 90, 15);

    chart1 = lv_chart_create(lv_scr_act(), NULL);
    lv_obj_set_size(chart1, 310, 130);
    lv_obj_set_pos(chart1, 4, 100);
    lv_chart_set_series_width(chart1, 2);
    lv_chart_set_type(chart1, LV_CHART_TYPE_POINT | LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart1, 0, 120);
    lv_chart_set_div_line_count(chart1, 4, 0);
    dl2_1 = lv_chart_add_series(chart1, LV_COLOR_RED);

    lv_group_set_wrap(g, true);
		
}
void timer1_callback(void* param)
{
    lv_chart_set_next(chart1, dl2_1, rand() % 120);
    lv_chart_refresh(chart1);
    osEventFlagsSet(LVGL_rfr_evt_id, 0x0001);
}
void timer2_callback(void* param)
{
    GPIOB->ODR ^= (1 << 8); //toggle PB8 LED
}

// LVGL refresh thread , Standby after 10secs rotary encoder inactivity
void lvgl_refresh_Thread(void* argument)
{
    uint32_t index;

    for (;;) {
        osEventFlagsWait(LVGL_rfr_evt_id, 0x0001, NULL, osWaitForever);
        osMutexAcquire(lvgl_upd_mutex, osWaitForever);
        for (index = 0; index < 1000; index++) {
            lv_task_handler();
            lv_tick_inc(10);
            osDelay(10);
        }
        osMutexRelease(lvgl_upd_mutex);
    }
}

void app_main(void* argument)
{

	lv_init();
    ILI9341_init();
    PWM_Init();
    LED_PB8_init();

    static lv_disp_buf_t disp_buf;
    static lv_color_t buf_1[LV_HOR_RES_MAX * 10];
    lv_disp_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * 10);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = ILI9341_flush;
    lv_disp_drv_register(&disp_drv);

    encoder_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    encoder_indev = lv_indev_drv_register(&indev_drv);

    g = lv_group_create();
    lv_indev_set_group(encoder_indev, g);

    gui_create();

    lvgl_upd_mutex = osMutexNew(NULL);
    LVGL_rfr_evt_id = osEventFlagsNew(NULL);
    timer0_id = osTimerNew(timer1_callback, osTimerPeriodic, (void*)0, NULL);
    timer1_id = osTimerNew(timer2_callback, osTimerPeriodic, (void*)0, NULL);
    T_lvgl_refresh_Thread = osThreadNew(lvgl_refresh_Thread, NULL, &lvgl_refresh_Thread_attr);
    osEventFlagsSet(LVGL_rfr_evt_id, 0x0001);
    for (;;) {
    }
}

int main(void)
{
    // System Initialization
    SystemCoreClockUpdate();
    osKernelInitialize(); // Initialize CMSIS-RTOS
    osThreadNew(app_main, NULL, &main_app_attr); // Create application main thread
    osKernelStart(); // Start thread execution
    for (;;) {
    }
}
