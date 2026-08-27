/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "queue.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "beep.h"
#include "u8g2.h"
#include "Data.h"
/* USER CODE END Includes */

extern QueueHandle_t g_xQueueMenu;
extern TaskHandle_t xShowMenuTaskHandle;
extern QueueHandle_t g_xQueuekey;
extern u8g2_t u8g2;
// void OpenLightUI(void)
// {
// 	u8g2_DrawXBMP(&u8g2, 48, 16, 30, 30, light);

// }
// void CloseLightUI(void)
// {
// 	 u8g2_DrawBox(&u8g2, 0, 0, 128, 64);
// }
void ShowFlashLightTask(void *params)
{
	/* u8g2 Start */
	// u8g2_t u8g2;
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0, u8x8_byte_hw_i2c, u8g2_stm32_delay);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_chinese1);
//	u8g2_SetFont(&u8g2, u8g2_font_spleen32x64_mf);	
	// u8g2_SetFont(&u8g2, u8g2_font_fur35_tf);
	//u8g2_ClearBuffer(&u8g2);

	uint8_t light_flag = 0;
     Keydata	key_data1;
	
	while(1)
	{
		u8g2_ClearBuffer(&u8g2);	
		switch(light_flag)
			{
				case 0: u8g2_DrawXBMP(&u8g2, 48, 16, 30, 30, light);break;
				case 1: u8g2_DrawBox(&u8g2, 0, 0, 128, 64);break;
			}
		u8g2_SendBuffer(&u8g2);
		xQueueReceive(g_xQueuekey, &key_data1, portMAX_DELAY);
		
		if(key_data1.updata == 1)
		{
			key_data1.updata = 0;
			buzzer_buzz(50);
			light_flag = (light_flag==0)?1:0;
		}		
		if(key_data1.exdata == 1)
		{
			key_data1.exdata = 0;
			buzzer_buzz(50);
			vTaskResume(xShowMenuTaskHandle);
			vTaskSuspend(NULL);
		}		
		
	}
}

