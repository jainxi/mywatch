/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "event_groups.h"
#include "semphr.h"
#include "queue.h"
#include "gpio.h"
#include "beep.h"
#include "u8g2.h"
#include "Data.h"
#include "string.h"
#include "stdio.h"

extern u8g2_t u8g2;
extern QueueHandle_t g_xQueueMenu;
extern QueueHandle_t g_xQueuekey;
//extern SemaphoreHandle_t g_xSemMenu; 

extern TaskHandle_t xShowMenuTaskHandle;
extern TaskHandle_t xShowTimeTaskHandle;
extern TaskHandle_t xShowFlashLightTaskHandle;
extern TaskHandle_t xShowSettingTaskHandle;
extern TaskHandle_t xShowClockTaskHandle;
extern TaskHandle_t xShowCalendarTaskHandle;
extern TaskHandle_t xShowDHT11TaskHandle;
#define MENU_SPEED_X 2
#define	MENU_LEN_X 40
#define MENU_NUM 5
#define Dock_Speed 0.8
const char str[5][10] = {"cleder", "torch", "hum", "clock", "more"};
/* app's name */
str1 fly1 = {"fly1", NULL};
str1 dino1 = {"hum", NULL};
str1 test1 = {"torch", NULL};
str1 block1 = {"clock", NULL};
str1 setting1 = {"setting", NULL};

/* some data */
Image Left = {0, 0, 23, 10};
Image Right = {104, 0, 23, 10};
Image String = {53, 10, 0, 0};
Image Rec_select = {49, 16, 32, 32};
uint8_t dock_pos = 0;
uint8_t dock_status = 5;
uint8_t dock[5] = {45, 55, 65, 75, 85};
uint8_t dock_y = 58, dock_r = 3;  
int str_flag = 0;
int8_t length_offset_x = 0;
uint8_t length_flag = 0;
uint8_t length_flag_old = 0;
uint8_t menu_y[1] ={17};
int8_t menu_x =10;
uint8_t dock_x[3]={10,50,90};
static float dock_act_x=10;
/* draw app's icon */
void DrawApp(uint8_t app_id,int16_t x)
{
	switch (app_id)
	{
	case 0:
		u8g2_DrawXBMP(&u8g2, x,menu_y[0],  cleder.w, cleder.h, cleder.data);
		break;
	case 1:
		u8g2_DrawXBMP(&u8g2,  x, menu_y[0], torch.w, torch.h, torch.data);
		break;
	case 2:
		u8g2_DrawXBMP(&u8g2, x, menu_y[0], hum.w, hum.h, hum.data);
		break;
	case 3:
		u8g2_DrawXBMP(&u8g2, x, menu_y[0], clock.w, clock.h, clock.data);
		break;
	case 4:
		u8g2_DrawXBMP(&u8g2, x,menu_y[0], setting.w, setting.h, setting.data);
		break;
	
	}
}
float Select_change(float actual,float target,float speed)
{
	if((target-actual)>1)
	{
		actual+=(target-actual)*speed+1;
	}
	else if((target-actual)<-1)
	{
		actual+=(target-actual)*speed-1;
	}
	else
	{
		actual=target;
	}
	return actual;
}
void ShowUI(void)
{
	/* show_gameui */
	u8g2_DrawXBMP(&u8g2, Left.x, Left.y, Left.w, Left.h, LeftMove);
	u8g2_DrawXBMP(&u8g2, Right.x, Right.y, Right.w, Right.h, RightMove);
	
	

	if(length_flag!=length_flag_old)
	{
		length_offset_x=(length_flag>length_flag_old?MENU_LEN_X:(length_flag<length_flag_old?-MENU_LEN_X:0));
		length_flag_old=length_flag;
	}
	if(length_offset_x!=0)
	{
		if(length_offset_x>=-1&&length_offset_x<=1)
	{
		length_offset_x=0;
	}
	else
	{
		length_offset_x/=MENU_SPEED_X;
	}
	}
	

	for (int8_t i = -1; i < 4; i++)
	{
		int8_t item =length_flag+i;
		if(item<0)
		{continue;}
		if(item>4)
		{break;}
		DrawApp(item,menu_x+i*MENU_LEN_X+length_offset_x);
		// printf("%d\r\n",menu_x[0]+i*MENU_LEN_X);	
	}
	
	
	

	u8g2_DrawDisc(&u8g2, dock[dock_pos], dock_y, dock_r, U8G2_DRAW_ALL);
	for(int i = 0; i<5; i++)
	{
		u8g2_DrawCircle(&u8g2, dock[i], dock_y, dock_r, U8G2_DRAW_ALL);
	}	
	uint8_t cursor_line=dock_pos-length_flag;
	dock_act_x=Select_change(dock_act_x,dock_x[cursor_line]+length_offset_x,Dock_Speed);
	u8g2_DrawFrame(&u8g2, dock_act_x, Rec_select.y, Rec_select.w, Rec_select.h);
	
	u8g2_DrawStr(&u8g2, String.x, String.y, str[str_flag]);
}

void ShowMenuTask(void *params)
{

	u8g2_config();
	u8g2_FirstPage(&u8g2);
	do {
	u8g2_SendBuffer(&u8g2);
   	} while (u8g2_NextPage(&u8g2));
	
     Keydata key_data1;
	 

	while(1)
	{
		// taskENTER_CRITICAL();
		u8g2_ClearBuffer(&u8g2);
		ShowUI();
		u8g2_SendBuffer(&u8g2);
		// taskEXIT_CRITICAL();
		/* receive queue data and keep waitting */
	
			pdPASS == xQueueReceive(g_xQueuekey, &key_data1, 10);
	
		/* handle data */
			if(key_data1.rdata == 1)
		{	

				if(dock_pos<MENU_NUM-1)
				{
					if(dock_pos>length_flag+1)
					{
						length_flag++;
						if(dock_pos < MENU_NUM-1){dock_pos++;str_flag++;}
						key_data1.rdata = 0;

					}
					else
					{
						if(dock_pos < MENU_NUM-1){dock_pos++;str_flag++;}
						key_data1.rdata = 0;
					}
				}
				else
				{
					key_data1.rdata = 0;
				}


		}
		

		else if(key_data1.ldata == 1)
		{
			if(dock_pos>0)
			{
				if(dock_pos<length_flag+1)
				{
					length_flag--;
					if(dock_pos > 0){dock_pos--;str_flag--;}
					key_data1.ldata = 0;
				}
				else
				{
					if(dock_pos > 0){dock_pos--;str_flag--;}
					key_data1.ldata = 0;
				}
			}
			else
			{
				key_data1.ldata = 0;
			}

		}
		/* ststus machine : task scheduling  */
		else if(key_data1.exdata == 1)
		{
			buzzer_buzz(50);
			switch(dock_pos)
			{
				case 0: vTaskResume(xShowCalendarTaskHandle);vTaskSuspend(NULL);key_data1.exdata = 0;break;
				case 1: vTaskResume(xShowFlashLightTaskHandle);vTaskSuspend(NULL);key_data1.exdata = 0;break;
				case 2: vTaskResume(xShowDHT11TaskHandle);vTaskSuspend(NULL);key_data1.exdata = 0;break;
				case 3: vTaskResume(xShowClockTaskHandle);vTaskSuspend(NULL);key_data1.exdata = 0;break;
				case 4: vTaskResume(xShowSettingTaskHandle);vTaskSuspend(NULL);key_data1.exdata = 0;break;
			}
		}
		else if(key_data1.updata == 1)
		{
			/* SysSound */
			// buzzer_buzz(50);
			vTaskResume(xShowTimeTaskHandle);
			vTaskSuspend(NULL);
			key_data1.updata = 0;
		}
	}
}
