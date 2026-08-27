/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "queue.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "beep.h"
#include "driver_dht11.h"
#include "u8g2.h"
#include "Data.h"
//#include "ShowCalendar.h"

/* USER CODE END Includes */

extern TaskHandle_t xShowMenuTaskHandle;
extern QueueHandle_t g_xQueueMenu;
extern QueueHandle_t g_xQueuekey;
extern int8_t temp_limit1;
extern int8_t temp_limit2;
void ShowDHT11Task(void *params)
{
	DHT11_Init();
	// buzzer_init();

	u8g2_t u8g2;
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0, u8x8_byte_hw_i2c, u8g2_stm32_delay);
	u8g2_InitDisplay(&u8g2); 
	u8g2_SetPowerSave(&u8g2, 0); 
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_wqy16_t_chinese1);

	u8g2_SendBuffer(&u8g2);
	
	Keydata	key_data1;
	int hum, temp;
    int hum1, hum2, temp1, temp2;

	while(1)
	{	
		u8g2_ClearBuffer(&u8g2);			
		if (DHT11_Read(&hum, &temp) !=0 ){
			//printf("\n\rdht11 read err!\n\r");
			DHT11_Init();
		}
		else{
			temp1 = temp%10;	//low bit
			temp2 = temp/10;   //high bit
						
			hum1 = hum%10;		//low bit	
			hum2 = hum/10;   //high bit
			
			u8g2_DrawXBMP(&u8g2, 10, 20, 20, 40, BigNum[temp2]);
			u8g2_DrawXBMP(&u8g2, 35, 20, 20, 40, BigNum[temp1]);
			
			u8g2_DrawXBMP(&u8g2, 75, 20, 20, 40, BigNum[hum2]);
			u8g2_DrawXBMP(&u8g2, 100, 20, 20, 40, BigNum[hum1]);
		}
		u8g2_DrawStr(&u8g2, 15, 15, "temp");
		u8g2_DrawStr(&u8g2, 85, 15, "Hum");
		
		u8g2_SendBuffer(&u8g2);
	

		xQueueReceive(g_xQueuekey, &key_data1, 0);
		if((temp1+temp2*10)>(temp_limit2+temp_limit1*10))
		{
			buzzer_buzz(100);
		}
		vTaskDelay(300);
		
		if(key_data1.exdata == 1)
		{
			buzzer_buzz(100);
			vTaskResume(xShowMenuTaskHandle);
			vTaskSuspend(NULL);
			key_data1.exdata = 0;
		}
	}
}
