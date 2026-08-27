
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core

//#include "driver_passive_buzzer.h"

static TimerHandle_t g_TimerSound;
int power_button=0;
void PassiveBuzzer_Control(int on)
{
	if(on)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	}
	
}


static void SystemSoundTimer_Func( TimerHandle_t xTimer )
{
	PassiveBuzzer_Control(0);
}


void buzzer_init(void)
{
//	/* 初始化蜂鸣器 */
//	PassiveBuzzer_Init();	
    PassiveBuzzer_Control(0);
//	
	/* 创建定时器 */

	g_TimerSound = xTimerCreate( "SystemSound", 
							200,
							pdFALSE,
							NULL,
							SystemSoundTimer_Func);
}



void buzzer_buzz(int time_ms)
{
	if (power_button==1)
	{
		return;
	}
	
	if(power_button == 0)
	{
		PassiveBuzzer_Control(1);		
		/* 启动定时器 */
		xTimerChangePeriod(g_TimerSound, time_ms, 0);		
	}

}

