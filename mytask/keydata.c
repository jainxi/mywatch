#include "keydata.h"
#include "main.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "data.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "ShowTimeTask.h"
#include "string.h"
#include "stdio.h"


#define KEY_UP				0x04
#define KEY_SINGLE		0x08
#define KEY_DOUBLE		0x10
#define KEY_LONG			0x20
#define KEY_REPEAT		0x40
#define r_raw					0x21
#define l_raw					0x41
#define KEY_PRESSED				1
#define KEY_UNPRESSED			0
#define KEY_TIME_DOUBLE	 200
#define KEY_TIME_LONG			700
#define KEY_TIME_REPEAT			100
Keydata key_data={0}; 
uint8_t keyflag;
extern uint8_t get_key(void);
QueueHandle_t g_xQueuekey;
extern QueueHandle_t g_xQueueMenu;
uint8_t ketgetstate(void)
{
   if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == 0)
   {
       return KEY_PRESSED;
   }
   return KEY_UNPRESSED;
}
void key_tick(void)
{
  static uint8_t currentstate;
  static uint16_t time;
  static uint8_t count;
  static uint8_t S;
  if(time>0)
  {
    time--;
  }
 count ++;
	if (count >= 20)
	{
		count = 0;
   currentstate=ketgetstate();

   switch(S)
   {
    case 0:
    if(currentstate==KEY_PRESSED)
    {
      S=1;
      time=KEY_TIME_LONG;
    }
    break;
    case 1:
    if(currentstate==KEY_UNPRESSED)
    {
      time=KEY_TIME_DOUBLE;
      S=2;
    }
    else if(time==0)
    {
      keyflag|=KEY_LONG;
      // printf("检测长按\r\n");
      S=4;
      time=KEY_TIME_REPEAT;
    }
    break;
    case 2:
    if(currentstate==KEY_PRESSED)
    {
      keyflag|=KEY_DOUBLE;
      S=3;
    }
    else if(time==0)
    {
      keyflag|=KEY_SINGLE;
      // printf("检测单击\r\n");
      S=0;
    }
    break;
    case 3:
    if(currentstate==KEY_UNPRESSED)
    {
      S=0;
    }
    break;
    case 4:
    if(currentstate==KEY_UNPRESSED)
    {
      S=0;
    }
    else if(time==0)
    {
      keyflag|=KEY_REPEAT;
      time=KEY_TIME_REPEAT;
      S=4;
    }


   }

  }
	
}
extern volatile int8_t ex_cnt;

int8_t get_encode_step(void)
{
       static int8_t lastraw;
        int8_t theraw;
        theraw=ex_cnt>>2;
        int8_t diff=theraw-lastraw;
        lastraw=theraw;
        return diff;
}
void key_task(void *params)
{
    const TickType_t xDelay5ms = pdMS_TO_TICKS(5);
	TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
   static uint8_t lastkeyflag=0;
   int16_t temp;
    while(1)
    {
        
        // taskENTER_CRITICAL();
      //  key_tick();
      temp=get_encode_step();
      //  temp=ex_cnt;
      //   ex_cnt=0;
        // taskEXIT_CRITICAL();
        if(keyflag!=lastkeyflag)
        {
        
           if(xQueueSend(g_xQueueMenu, &keyflag, 0)== pdPASS)
           {
              //  printf("队列打印成功keyflag=%d\r\n",keyflag);
               lastkeyflag=keyflag;
           }

        }
        if(temp>0)
        {
          uint8_t flag=r_raw;
          if(xQueueSend(g_xQueueMenu, (const void *)&flag, 0)== pdPASS)
          {
              // printf("队列打印右转flag=%d\r\n",flag);
          }
          else
          {
              // printf("失败");
          }
        }
        else if(temp<0)
        {
          uint8_t flag=l_raw;
          if(xQueueSend(g_xQueueMenu, (const void *)&flag, 0)== pdPASS)
          {
              // printf("队列打印左转flag=%d\r\n",flag);
          }
          else
          {
              // printf("失败");
          }
        }

        vTaskDelayUntil(&xLastWakeTime, xDelay5ms);
    }

}

void getquene(void *params)
{

   uint8_t rec_flag=0;
   g_xQueuekey=xQueueCreate(1,sizeof(key_data));
   while(1)
   {
  
    if(pdPASS == xQueueReceive(g_xQueueMenu, &rec_flag, portMAX_DELAY))
    {
       memset(&key_data,0,sizeof(key_data));
       if(rec_flag==KEY_SINGLE)
       {
           keyflag&=~KEY_SINGLE;
		    	key_data.updata=1;
          
          pdPASS==  xQueueSend( g_xQueuekey, (const void *)&key_data, 0);

	   }
      else if(rec_flag==KEY_DOUBLE)
       {
             keyflag&=~KEY_DOUBLE;
			      key_data.exdata=1;
           
            xQueueSend( g_xQueuekey, (const void *)&key_data, 0);
	   }
     else if(rec_flag==KEY_LONG)
       {
             keyflag&=~KEY_LONG;
            key_data.longdata=1;
            // printf("检测长按\r\n");
            xQueueSend( g_xQueuekey, (const void *)&key_data, 0);
            }
      else if(rec_flag==KEY_REPEAT)
       {
             keyflag&=~KEY_REPEAT;
            key_data.repeatdata=1;
           printf("检测不停长按\r\n");
            xQueueSend( g_xQueuekey, (const void *)&key_data, 0);
            }
       else if(rec_flag==r_raw)
    {    
      
        key_data.rdata=1;
        // printf("检测右转\r\n");
        xQueueSend( g_xQueuekey, (const void *)&key_data, 0);
    }
    else if(rec_flag==l_raw)
    {
      // printf("检测zuo转\r\n");
        key_data.ldata=1;
        xQueueSend( g_xQueuekey, (const void *)&key_data, 0);
    }
    }
   }
}

