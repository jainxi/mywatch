/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
//#include "semphr.h"
#include "queue.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "beep.h"
#include "u8g2.h"
#include "Data.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
/* some extern data */
extern TaskHandle_t xShowMenuTaskHandle;
extern QueueHandle_t g_xQueueMenu;
extern QueueHandle_t g_xQueuekey;
extern u8g2_t u8g2;
extern TaskHandle_t xShowTimeTaskHandle;
#define PAGE_SETTING_LIST    0   
#define PAGE_TEMP_LIMIT      1   
#define PAGE_BEEPER_SET      2  
#define PAGE_ABOUT           3   
#define PAGE_EXIT            4   
#define task_num 7
uint8_t page_flag = PAGE_SETTING_LIST;
uint8_t select_flag = 0;
uint8_t task_y[1]={18};
uint8_t select_y[3] = {18,38,58};
uint8_t task_x = 15;
uint8_t select_x =20;
uint8_t endflag=1;
uint8_t queueflag = 0;
uint8_t win_start=0;
uint8_t numx[2] = {50,58};
uint8_t numy = 30;
int8_t temp_limit1 = 4;
int8_t temp_limit2 = 0;
uint8_t selectnum = 0;
uint8_t switch_status = 0;
uint8_t win_start_flag = 0;
static float cursor_act_y=18-10,cursor_tar_y;
static float cursor_act_w=80+1,cursor_tar_w;
// static float caidan_act_y;
// static float caidan_tar_y;
static int8_t show_offset=0;
static uint8_t win_start_old;
extern int power_button;
#define CURSOR_SPEED 0.58f
#define LINE_SPEED 2
#define LINE_HEIGHT 20

char *menu_name[7] = {"Temp_limit","Beeper_Set","About","Exit","Back","Another","Item6"};
void DrawMenuItem(uint8_t index,uint8_t y)
{


	if(index >= sizeof(menu_name)/sizeof(char*))
	{
		return;
	}
	u8g2_DrawStr(&u8g2, task_x, y, menu_name[index]);

}
float Menu_CurveMigration(float Actual_Value, float Target_Value, float Act_Speed)
{
	if ((Target_Value - Actual_Value) > 1)
	{
		Actual_Value += (Target_Value - Actual_Value) * Act_Speed + 1;
	}
	else if ((Target_Value - Actual_Value) < -1)
	{
		Actual_Value += (Target_Value - Actual_Value) * Act_Speed - 1;
	}
	else
	{
		Actual_Value = Target_Value;
	}

	return Actual_Value;
}
void Show_settingUI(void)
{
	uint8_t str_len[5];

	
	for(uint8_t i = 0; i < 3; i++)
	{
		str_len[i] = u8g2_GetStrWidth(&u8g2, menu_name[win_start+i]);
		// printf("str_len[0] = %d\r\n",str_len[0]);
	}
	uint8_t cursor_line = select_flag - win_start;
	cursor_tar_y = select_y[cursor_line]-10;
	cursor_tar_w = str_len[cursor_line]+1;

if(win_start != win_start_old)
{
    show_offset = (win_start - win_start_old) *LINE_HEIGHT;
    win_start_old = win_start;
}

if(show_offset != 0)
{
    if(show_offset >= -1 && show_offset <= 1)
    {
        show_offset = 0;
    }
    else
    {
        show_offset /=  LINE_SPEED;   
    }
}

for(int8_t i = -1; i < 6; i++)
{
    int8_t item = win_start + i;
    if(item < 0)
        continue;
    if(item >= task_num)  
        break;

    int16_t y_draw = task_y[0] + i * LINE_HEIGHT + show_offset;
    DrawMenuItem(item, y_draw);
}

	// for (uint8_t i = 0; i < 3; i++)
	// 	{
	// 		DrawMenuItem(win_start+i,task_y[i]);
	// 	}
		
	cursor_act_w = Menu_CurveMigration(cursor_act_w, cursor_tar_w, CURSOR_SPEED);
	cursor_act_y = Menu_CurveMigration(cursor_act_y, cursor_tar_y, CURSOR_SPEED);
	u8g2_DrawXBMP(&u8g2, 1,(uint8_t)cursor_act_y+show_offset, 6,8, Num_6x8[11]);
	u8g2_DrawBox(&u8g2, task_x-1,(uint8_t)cursor_act_y+show_offset, (uint8_t)cursor_act_w,14);
	u8g2_SetDrawColor(&u8g2, 0);
	DrawMenuItem(win_start+cursor_line,select_y[cursor_line]+show_offset);
	u8g2_SetDrawColor(&u8g2, 1);
}

void Show_settingBeeperSetUI(void)
{
	u8g2_DrawHLine(&u8g2, 45, 22, 40);
	u8g2_DrawHLine(&u8g2, 45, 40, 40);
	if(switch_status == 0)
	{
		u8g2_DrawDisc(&u8g2, 45, 31, 9, U8G2_DRAW_ALL);
		u8g2_DrawStr(&u8g2, 75, 34,  "on");  
		u8g2_DrawCircle(&u8g2, 85, 31, 9, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);	
		power_button = 0;
	}
	if(switch_status == 1)
	{
		u8g2_DrawDisc(&u8g2, 85, 31, 9, U8G2_DRAW_ALL);
		u8g2_DrawStr(&u8g2, 41, 36,  "off"); 
		u8g2_DrawCircle(&u8g2, 45, 31, 9, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_LOWER_LEFT);
		power_button = 1;
	}
	
}

void Show_settingTempLimitUI(void)
{
	 u8g2_DrawStr(&u8g2, 30, task_y[0], "THRESHOLD");
	 u8g2_DrawXBMP(&u8g2, numx[0], numy, 6,8, Num_6x8[temp_limit1]);
	 u8g2_DrawXBMP(&u8g2, numx[1], numy, 6,8, Num_6x8[temp_limit2]);
	 u8g2_DrawXBMP(&u8g2, numx[1]+10, numy, 8,11, mysetting[0]);
	 u8g2_DrawFrame(&u8g2, numx[selectnum]-1, numy-2, 8, 13);	
	
}

void Show_settingAbout(void)
{
	u8g2_DrawStr(&u8g2, 27, 20, "By Mr.Shen");
	u8g2_DrawStr(&u8g2, 29, 35, "2026-7-31");
}

void ShowSetting_Task(void *params)
{
	/* suspend_other_task */
	// vTaskSuspend(xShowMenuTaskHandle);
	//vTaskSuspend(xShowTimeTaskHandle);
	//vTaskSuspend(xShowClockTaskHandle);
	//vTaskSuspend(xShowDHT11TaskHandle);
	//vTaskSuspend(xShowFlashLightTaskHandle);
	//vTaskSuspend(xShowWoodenFishTaskHandle);
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0, u8x8_byte_hw_i2c, u8g2_stm32_delay);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_chinese1);
	Keydata key_data1;
	while(1)
	{
		u8g2_ClearBuffer(&u8g2);
		switch (page_flag)
		{
		case PAGE_SETTING_LIST:
			Show_settingUI();
			break;
		case PAGE_TEMP_LIMIT:
			Show_settingTempLimitUI();
			break;
		case PAGE_BEEPER_SET:
			Show_settingBeeperSetUI();
			break;
		case PAGE_ABOUT:
			Show_settingAbout();
			break;
		}
		u8g2_SendBuffer(&u8g2);	
		// vTaskDelay(100);
	
	
			pdPASS == xQueueReceive(g_xQueuekey, &key_data1, 10);
	
		if(key_data1.rdata == 1&&page_flag==PAGE_SETTING_LIST)
		{
		// 	endflag = 0;
		// 	if(select_flag<task_num-1)
		// 	{
		// 		// uint8_t next_sel=select_flag+1;
		// 		if(select_flag>win_start+1)
		// 		{
		// 			ui_up((int32_t *)&task_y[0], 2);ui_up((int32_t *)&task_y[1], 2);ui_up((int32_t *)&task_y[2], 2);ui_up((int32_t *)&task_y[3], 2);ui_up((int32_t *)&task_y[4], 2);
		// 		// ui_up((int32_t*)&select_y[select_flag], 2);
		// 			queueflag++;
		// 			if (queueflag==10)
		// 		{
		// 			win_start++;
		// 			endflag = 1;
		// 			queueflag = 0;
		// 			key_data1.rdata = 0;
		// 			select_flag++;
		// 			buzzer_buzz(50);

		// 		}
		// 		}
		// 	  else 
		// 	{
		// 		if(select_flag<task_num-1)
		// 		{
		// 			select_flag++;
		// 		}
		// 		key_data1.rdata = 0;
		// 		endflag = 1;
		// 		buzzer_buzz(50);			
		// 	}

		// }
		// else
		// {
		// 	endflag = 1;
		// 	key_data1.rdata = 0;
		// }
		if(select_flag<task_num-1)
			{
				if(select_flag>win_start+1)
				{

					win_start++;
					select_flag++;
					key_data1.rdata = 0;
					// buzzer_buzz(50);
					
				}
				else 
				{
					select_flag++;
					key_data1.rdata = 0;
				    // buzzer_buzz(50);
				}
			
			}
			else
			{
				key_data1.rdata = 0;
			}

	}
	else if (key_data1.rdata == 1&&page_flag==PAGE_TEMP_LIMIT)
	{

		selectnum=1;
		key_data1.rdata = 0;
	}
	else if (key_data1.ldata == 1&&page_flag==PAGE_TEMP_LIMIT)
	{
		selectnum=0;
		key_data1.ldata = 0;
	}
	else if(key_data1.ldata == 1&&page_flag==PAGE_SETTING_LIST)
	{	
		// endflag = 0;
		// if(select_flag>0)
		// {
		// 	// uint8_t next_sel=select_flag-1;
		// 	if(select_flag<win_start+1)
		// 	{
		// 		ui_down((int32_t *)&task_y[0], 2);ui_down((int32_t *)&task_y[1], 2);ui_down((int32_t *)&task_y[2], 2);ui_down((int32_t *)&task_y[3], 2);ui_down((int32_t *)&task_y[4], 2);
		// 		queueflag++;
		// 		if (queueflag==10)
		// 		{
		// 			win_start--;
		// 			endflag = 1;
		// 			queueflag = 0;
		// 			key_data1.ldata = 0;
		// 			select_flag--;
		// 			buzzer_buzz(50);
		// 		}
	
		// 	}
			
		// else 
		// 	{
				
		// 		if(select_flag>0)
		// 		{
		// 			select_flag--;
		// 		}
		// 		key_data1.ldata = 0;
		// 		endflag = 1;
		// 		buzzer_buzz(50);
				
		// 	}
		//  }
		//  else
		//  {
		// 	endflag = 1;
		// 	key_data1.ldata = 0;
		//  }

		if (select_flag>0)
		{
			if (select_flag<win_start+1)
			{
				win_start--;
				select_flag--;
				key_data1.ldata = 0;
				// buzzer_buzz(50);
			}
			else 
			{
				select_flag--;
				key_data1.ldata = 0;
				// buzzer_buzz(50);
			}
			
		}
		else
		{
			key_data1.ldata = 0;
		}
		

	}
		else if(key_data1.updata == 1&&page_flag==PAGE_SETTING_LIST)
		{
			switch (select_flag)
			{
			case 0:page_flag = PAGE_TEMP_LIMIT;key_data1.updata = 0;break;
			case 1:page_flag = PAGE_BEEPER_SET;key_data1.updata = 0;break;
			case 2:page_flag = PAGE_ABOUT;key_data1.updata = 0;break;
			default:break;
			}
		}
		else if(key_data1.updata == 1&&page_flag==PAGE_TEMP_LIMIT)
		{
			if(selectnum==0)
			{
				temp_limit1++;
				if(temp_limit1>9)temp_limit1=0;
				key_data1.updata = 0;
			}
			else if(selectnum==1)
			{
				temp_limit2++;
				if(temp_limit2>9)temp_limit2=0;
				key_data1.updata = 0;
			}
		}
	
		else if(key_data1.repeatdata == 1&&page_flag==PAGE_TEMP_LIMIT)
		{
			if(selectnum==0)
			{
				temp_limit1--;
				if(temp_limit1<0)temp_limit1=9;
				key_data1.repeatdata = 0;
				
			}
			else if(selectnum==1)
			{
				temp_limit2--;
				if(temp_limit2<0)temp_limit2=9;
				key_data1.repeatdata = 0;
			}
				
		}
		else if(key_data1.updata == 1&&page_flag==PAGE_BEEPER_SET)
		{
			switch_status^=1;
			key_data1.updata = 0;
		}
		else if(key_data1.exdata == 1)
		{

			/* SysSound */
			switch (page_flag)
			{
		     case 0:  buzzer_buzz(50);
			vTaskResume(xShowMenuTaskHandle);
			// vTaskResume(xShowTimeTaskHandle);
			vTaskSuspend(NULL);
			key_data1.exdata = 0;
			break;
			case 1:  buzzer_buzz(50);
			page_flag = PAGE_SETTING_LIST;
			key_data1.exdata = 0;
			break;
			case 2:  buzzer_buzz(50);
			page_flag = PAGE_SETTING_LIST;
			key_data1.exdata = 0;
			break;
			case 3: buzzer_buzz(50);
			page_flag = PAGE_SETTING_LIST;
			key_data1.exdata = 0;
			break;
			}
		}
		

}

}

