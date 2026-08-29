/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "u8g2.h"
#include "beep.h"
#include "Data.h"
#include "keydata.h"
#include "stdio.h"
/* USER CODE END Includes */

/* other task handle */
extern float Select_change(float actual,float target,float speed);
 QueueHandle_t g_xQueueMenu;
extern QueueHandle_t g_xQueuekey;
extern TaskHandle_t xShowMenuTaskHandle;
extern TaskHandle_t xShowTimeTaskHandle;
extern TaskHandle_t xShowFlashLightTaskHandle;
extern TaskHandle_t xShowSettingTaskHandle;
extern TaskHandle_t xShowCalendarTaskHandle;
extern TaskHandle_t xShowClockTaskHandle;
extern TaskHandle_t xShowDHT11TaskHandle;
extern TimerHandle_t g_Timer;
extern u8g2_t u8g2;
/* some data */
#define BOX_R 1
uint8_t time_flag = 0;
Keydata key_data1= {0};
int8_t sec_unit, sec_decade, min_unit, min_decade, hour_unit, hour_decade,month_unit,month_decade,day_unit,day_decade;
uint8_t time_select_x[10]={8, 35, 71, 98,56,66,90,99,112,121};
uint8_t time_select_y[10]={15, 15, 15, 15,2,2,2,2,2,2};
uint8_t time_select_w[10]={20, 20, 20, 20,6,6,6,6,6,6};
uint8_t time_select_h[10]={40, 40, 40, 40,8,8,8,8,8,8};
int8_t time_select_flag = 0;
float time_select_act_x=8;
float time_select_act_w=20;
float time_select_act_h=40;
float time_select_act_y=15;
float time_select_tar_x;
float time_select_tar_w;
float time_select_tar_h;
float time_select_tar_y;
#define TIME_SELECT_SPEED 0.6f
typedef struct Time_param{
    int x[4];
	int y;
	int w;
	int h;
	int x_arg;
}T;
T time = { {8, 35, 71, 98}, 15, 20, 40, 98};
Image Box1 = {62, 22, 4, 4,};
Image Box2 = {62, 39, 4, 4,};
static uint8_t ui_status = 0;

#define ROLL_MS 250     
#define BN_BYR 3        
typedef struct {
    uint8_t disp;  
    uint8_t next; 
    uint32_t start;    
    uint8_t rolling;    
} RollCell;
static RollCell roll_cell[4] = { {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0} }; 

static uint8_t roll_value_of(uint8_t idx)
{
    switch (idx) {
    case 0: return (uint8_t)min_decade;
    case 1: return (uint8_t)min_unit;
    case 2: return (uint8_t)sec_decade;
    default: return (uint8_t)sec_unit;
    }
}


static void DrawDigitSlice(uint8_t glyph, int x, int cellY0, int cellY1, int gtop, int w)
{
    int r_start, r_end, rows, draw_y;
    r_start = cellY0 - gtop;
    if (r_start < 0) r_start = 0;
    if (r_start > 40) r_start = 40;
    r_end = cellY1 - gtop;
    if (r_end < 0) r_end = 0;
    if (r_end > 40) r_end = 40;
    rows = r_end - r_start;
    if (rows <= 0) return;
    draw_y = gtop + r_start;
    if (draw_y < cellY0) draw_y = cellY0;
    u8g2_DrawXBMP(&u8g2, x, draw_y, w, rows, BigNum[glyph] + (uint32_t)r_start * BN_BYR);
}


static void DrawRollingDigit(uint8_t idx, int x, int y, int w, int h)
{
    RollCell *c = &roll_cell[idx];
    uint8_t v = roll_value_of(idx);
    uint32_t now = (uint32_t)xTaskGetTickCount();
    uint32_t t, ease;
    int shift, gtopOld, gtopNew;

    if (c->rolling == 0) {
        if (v == c->disp) {                
            u8g2_DrawXBMP(&u8g2, x, y, w, h, BigNum[c->disp]);
            return;
        }
        c->next = v;                      
        c->start = now;
        c->rolling = 1;
    }

    t = now - c->start;                     
    if (t >= ROLL_MS) {                    
        c->disp = c->next;
        c->rolling = 0;
        u8g2_DrawXBMP(&u8g2, x, y, w, h, BigNum[c->disp]);
        return;
    }


    ease = 3u * t * t * ROLL_MS - 2u * t * t * t;
    shift = (int)((uint32_t)h * ease / ((uint32_t)ROLL_MS * ROLL_MS * ROLL_MS));
    gtopOld = y - shift;
    gtopNew = y + h - shift;

    u8g2_SetClipWindow(&u8g2, x, y, x + w, y + h);
    DrawDigitSlice(c->disp, x, y, y + h, gtopOld, w);
    DrawDigitSlice(c->next, x, y, y + h, gtopNew, w);
    u8g2_SetMaxClipWindow(&u8g2);
}
void ShowtimeUI(void)
{
	    u8g2_DrawXBMP(&u8g2, 0, 0, 23, 10, ShowPower);
		u8g2_DrawXBMP(&u8g2, 90, 2, 6, 8, Num_6x8[month_decade]);
		u8g2_DrawXBMP(&u8g2, 99, 2, 6, 8, Num_6x8[month_unit]);
		u8g2_DrawXBMP(&u8g2, 105, 3, 6, 8, mysetting1[0]);
		u8g2_DrawXBMP(&u8g2, 112, 2, 6, 8, Num_6x8[day_decade]);
		u8g2_DrawXBMP(&u8g2, 121, 2, 6, 8, Num_6x8[day_unit]);
		// u8g2_DrawXBMP(&u8g2, 105, 0, 23, 10, ShowGame);
		/* draw time */
		DrawRollingDigit(3, time.x[3], time.y, time.w, time.h);   /* sec_unit */
		DrawRollingDigit(2, time.x[2], time.y, time.w, time.h);   /* sec_decade */
		u8g2_DrawRBox(&u8g2, Box1.x, Box1.y, Box1.w, Box1.h, BOX_R);
		u8g2_DrawRBox(&u8g2, Box2.x, Box2.y, Box2.w, Box2.h, BOX_R);		
		DrawRollingDigit(1, time.x[1], time.y, time.w, time.h);   /* min_unit */
		DrawRollingDigit(0, time.x[0], time.y, time.w, time.h);   /* min_decade */

		u8g2_DrawXBMP(&u8g2, 56, 2, 6, 8, Num_6x8[hour_decade]);

		u8g2_DrawXBMP(&u8g2, 66, 2, 6, 8, Num_6x8[hour_unit]);
}

void ShowsetTimeUI(void)
{
	u8g2_DrawXBMP(&u8g2, 0, 0, 23, 10, ShowPower);
	u8g2_DrawXBMP(&u8g2, 90, 2, 6, 8, Num_6x8[month_decade]);
		u8g2_DrawXBMP(&u8g2, 99, 2, 6, 8, Num_6x8[month_unit]);
		u8g2_DrawXBMP(&u8g2, 105, 3, 6, 8, mysetting1[0]);
		u8g2_DrawXBMP(&u8g2, 112, 2, 6, 8, Num_6x8[day_decade]);
		u8g2_DrawXBMP(&u8g2, 121, 2, 6, 8, Num_6x8[day_unit]);
		// u8g2_DrawXBMP(&u8g2, 105, 0, 23, 10, ShowGame);
		/* draw time */
		u8g2_DrawXBMP(&u8g2, time.x[3], time.y, time.w, time.h, BigNum[sec_unit]);
		u8g2_DrawXBMP(&u8g2, time.x[2], time.y, time.w, time.h, BigNum[sec_decade]);
		u8g2_DrawRBox(&u8g2, Box1.x, Box1.y, Box1.w, Box1.h, BOX_R);
		u8g2_DrawRBox(&u8g2, Box2.x, Box2.y, Box2.w, Box2.h, BOX_R);		
		u8g2_DrawXBMP(&u8g2, time.x[1], time.y, time.w, time.h, BigNum[min_unit]);
		u8g2_DrawXBMP(&u8g2, time.x[0], time.y, time.w, time.h, BigNum[min_decade]);

		u8g2_DrawXBMP(&u8g2, 56, 2, 6, 8, Num_6x8[hour_decade]);

		u8g2_DrawXBMP(&u8g2, 66, 2, 6, 8, Num_6x8[hour_unit]);
		 time_select_tar_x=time_select_x[time_select_flag]-3;
		 time_select_tar_w=time_select_w[time_select_flag]+5;
		 time_select_tar_h=time_select_h[time_select_flag]+3;
		 time_select_tar_y=time_select_y[time_select_flag]-2;
	    time_select_act_x=Select_change(time_select_act_x,time_select_tar_x,TIME_SELECT_SPEED);
		time_select_act_w=Select_change(time_select_act_w,time_select_tar_w,TIME_SELECT_SPEED);
		time_select_act_h=Select_change(time_select_act_h,time_select_tar_h,TIME_SELECT_SPEED);
		time_select_act_y=Select_change(time_select_act_y,time_select_tar_y,TIME_SELECT_SPEED);
		u8g2_DrawFrame(&u8g2,  time_select_act_x,time_select_act_y, time_select_act_w,time_select_act_h);
}
void ShowTimeTask(void *params)
{
	// buzzer_init();
	//xSemaphoreTake(g_xSemTicks, portMAX_DELAY);

	/* suspend_other_task */
	vTaskSuspend(xShowMenuTaskHandle);
	vTaskSuspend(xShowSettingTaskHandle);
   	vTaskSuspend(xShowClockTaskHandle);
   	vTaskSuspend(xShowDHT11TaskHandle);
	vTaskSuspend(xShowFlashLightTaskHandle);
	vTaskSuspend(xShowCalendarTaskHandle);

	// //vTaskSuspend(xShowWoodenFishTaskHandle);
	
	 
	

	/* create_queue */
      
      g_xQueueMenu = xQueueCreate(3, sizeof(uint8_t));
	/* u8g2 Start */
	// u8g2_t u8g2;
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0, u8x8_byte_hw_i2c, u8g2_stm32_delay);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	u8g2_ClearDisplay(&u8g2);
//	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_chinese1);
	u8g2_SetFont(&u8g2, u8g2_font_fur35_tf);
	
	   if(g_Timer != NULL)
	{
		xTimerStart(g_Timer, 0);
	}


	while(1)
	{	
		// taskENTER_CRITICAL();
		u8g2_ClearBuffer(&u8g2);
		
		/* draw */
		switch (ui_status)
		{
		case 0:ShowtimeUI();
			break;
		
		case 1:ShowsetTimeUI();
			break;
		}
				
		u8g2_SendBuffer(&u8g2);
		// taskEXIT_CRITICAL();

		vTaskDelay(pdMS_TO_TICKS(20));
		/* handle queue data */
		{
			BaseType_t xReceived = xQueueReceive(g_xQueuekey, &key_data1, 0);
			if(xReceived == pdPASS && key_data1.updata == 1&&ui_status == 0)
			{	
				buzzer_buzz(50);			                     
				vTaskResume(xShowMenuTaskHandle);
				// vTaskResume( xShowSettingTaskHandle);
				vTaskSuspend(NULL);
				key_data1.updata = 0;
			}
		else if(key_data1.longdata == 1&&ui_status == 0)
			{
				buzzer_buzz(30);	
				ui_status = 1;
				key_data1.longdata = 0;
			}
		else if (key_data1.longdata == 1&&ui_status == 1)
		{
			buzzer_buzz(30);	
			ui_status = 0;
			key_data1.longdata = 0;
		}
		else if(key_data1.rdata == 1&&ui_status == 1)
		{
			time_select_flag++;
			buzzer_buzz(30);	
			if(time_select_flag > 9)
			{
				time_select_flag = 0;
			}
			key_data1.rdata = 0;
		}
		else if(key_data1.ldata == 1&&ui_status == 1)
		{
			buzzer_buzz(30);	
			time_select_flag--;
			if(time_select_flag < 0)
			{
				time_select_flag = 9;
			}
			key_data1.ldata = 0;
		}
		else if(key_data1.updata&&ui_status==1)
		{
			switch(time_select_flag)
			{
			case 0: min_decade++; if (min_decade > 5) { min_decade = 0; } key_data1.updata = 0; break;
			case 1: min_unit++; if (min_unit > 9) { min_unit = 0; }key_data1.updata = 0;break;
			case 2:sec_decade++; if (sec_decade > 5) { sec_decade = 0; } key_data1.updata = 0;break;
			case 3:sec_unit++; if (sec_unit > 9) { sec_unit = 0; }key_data1.updata = 0; break;
			case 4:hour_decade++; if (hour_decade > 2) { hour_decade = 0; } key_data1.updata = 0;break;
			case 5:hour_unit++; if (hour_unit > 9) { hour_unit = 0; } key_data1.updata = 0;break;
			case 6:month_decade++; if (month_decade > 1) { month_decade = 0; } key_data1.updata = 0;break;
			case 7:month_unit++; if (month_unit > 9) { month_unit = 0; } key_data1.updata = 0;break;
			case 8:day_decade++; if (day_decade > 3) { day_decade = 0; } key_data1.updata = 0;break;
			case 9:day_unit++; if (day_unit > 9) { day_unit = 0; } key_data1.updata = 0;break;
		     }
		}
		else if (key_data1.exdata&ui_status==1)
		{
			switch(time_select_flag)
			{
			case 0: min_decade--; if (min_decade < 0) { min_decade = 5; } key_data1.exdata = 0; break;
			case 1: min_unit--; if (min_unit < 0) { min_unit = 9; }key_data1.exdata = 0;break;
			case 2:sec_decade--; if (sec_decade < 0) { sec_decade = 5; } key_data1.exdata = 0;break;
			case 3:sec_unit--; if (sec_unit < 0) { sec_unit = 9; }key_data1.exdata = 0; break;
			case 4:hour_decade--; if (hour_decade < 0) { hour_decade = 2; } key_data1.exdata = 0;break;
			case 5:hour_unit--; if (hour_unit < 0) { hour_unit = 9; } key_data1.exdata = 0;break;
			case 6:month_decade--; if (month_decade < 0) { month_decade = 1; } key_data1.exdata = 0;break;
			case 7:month_unit--; if (month_unit < 0) { month_unit = 9; } key_data1.exdata = 0;break;
			case 8:day_decade--; if (day_decade < 0) { day_decade = 3; } key_data1.exdata = 0;break;
			case 9:day_unit--; if (day_unit < 0) { day_unit = 9; } key_data1.exdata = 0;break;
		     }
		}
	}
}
}

/******************TimerCallBackFun*******************/
void TimerCallBackFun(TimerHandle_t xTimer)
{  
	/* handle time data */
	if(ui_status == 0)
	{
	sec_unit++;		
	if(sec_unit > 9)    { sec_unit = 0;    sec_decade++; }
	if(sec_decade > 5)  { sec_decade = 0;  min_unit++;   }
	if(min_unit > 9)    { min_unit = 0;    min_decade++; }
	if(min_decade > 5)  { min_decade = 0;  hour_unit++;  }
	if(hour_unit > 9)
	{
		hour_unit = 0;
		hour_decade++;
	}
	if(hour_decade > 2 || (hour_decade == 2 && hour_unit > 3))
	{
		hour_decade = 0;
		hour_unit = 0;
	}
 }
 else if (ui_status == 1)
 {
	return;
 }
 
}
