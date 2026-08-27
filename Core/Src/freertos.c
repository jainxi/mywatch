/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ShowTimeTask.h"
#include "timers.h"
#include "keydata.h"
#include "stdio.h"
#include "data.h"
#include "queue.h"
#include "ShowClock.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
TaskHandle_t xShowTimeTaskHandle;
extern void getquene(void *params);

extern void ShowTimeTask(void *params);
TaskHandle_t xGetQueneTaskHandle;
TaskHandle_t xShowClockTaskHandle;
TaskHandle_t xShowMenuTaskHandle;
TaskHandle_t xkey_taskHandle;
TaskHandle_t xShowSettingTaskHandle;
TaskHandle_t xShowDHT11TaskHandle;
TaskHandle_t xShowFlashLightTaskHandle;
TaskHandle_t xShowCalendarTaskHandle;
TimerHandle_t g_Timer;
TimerHandle_t g_Clock_Timer;
extern  Keydata key_data;
extern void key_task(void *params);
extern void ShowMenuTask(void *params);
extern void ShowClockTimeTask(void *params);
extern void ShowSetting_Task(void *params);
extern void ShowDHT11Task(void *params);
extern void ShowFlashLightTask(void *params);
extern void ShowCalendarTask(void *params);
// extern void ClockTimerCallBackFun(TimerHandle_t xTimer);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
 
}
/* USER CODE END 2 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    g_Timer = xTimerCreate("Timer1",
				1000,
				pdTRUE,
				NULL,
				(TimerCallbackFunction_t)TimerCallBackFun);
    g_Clock_Timer = xTimerCreate("Timer2",
				100,
				pdTRUE,
				NULL,
				(TimerCallbackFunction_t)ClockTimerCallBackFun);
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
 
 
  xTaskCreate(getquene, "getquene", 80, NULL, osPriorityNormal-1, &xGetQueneTaskHandle);
  xTaskCreate(key_task, "key",80, NULL, osPriorityNormal, &xkey_taskHandle);
  xTaskCreate(ShowTimeTask, "showtime", 128, NULL, osPriorityNormal, &xShowTimeTaskHandle);
  xTaskCreate(ShowMenuTask, "showmenu", 108, NULL, osPriorityNormal, &xShowMenuTaskHandle);
  xTaskCreate(ShowClockTimeTask, "showclock", 90, NULL, osPriorityNormal, &xShowClockTaskHandle);
  xTaskCreate(ShowSetting_Task, "showsetting", 256, NULL, osPriorityNormal, &xShowSettingTaskHandle);
  xTaskCreate(ShowDHT11Task, "showdht11", 70, NULL, osPriorityNormal, &xShowDHT11TaskHandle);
  xTaskCreate(ShowFlashLightTask, "showflashlight", 70, NULL, osPriorityNormal, &xShowFlashLightTaskHandle);
  xTaskCreate(ShowCalendarTask, "showcalendar", 256, NULL, osPriorityNormal, &xShowCalendarTaskHandle);
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
 
  
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Hal_readGPIO(uint8_t *a_pin,uint8_t *b_pin)
{
  *a_pin=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
  *b_pin=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
} 
// volatile int8_t ex_cnt=0;
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
  
// }
volatile int8_t ex_cnt=0;
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
// 	if(GPIO_Pin == GPIO_PIN_12)
//   {
//     // printf("进入中断");
//     uint8_t b_pin=HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
//     if(b_pin==1)
//     {
//        ex_cnt--;
//     }
//     else
//     {
//       ex_cnt++;
//     }
//   }
// }

// static  char pcWriteBuffer[200];

// {
//   vTaskList(pcWriteBuffer);
//   uint8_t i=0;
//   for(i=0; i<16; i++)
//   {
//     printf("%s\r\n",pcWriteBuffer);
//   }

// }
/* USER CODE END Application */

