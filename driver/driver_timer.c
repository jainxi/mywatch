#include "driver_timer.h"
#include "stm32f1xx_hal.h"


void udelay(int us)
{

    extern TIM_HandleTypeDef        htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;

    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ticks = us * reload / (1000);  
    told = __HAL_TIM_GET_COUNTER(hHalTim);
    while (1)
    {
        tnow = __HAL_TIM_GET_COUNTER(hHalTim);
        if (tnow != told)
        {
            if (tnow > told)
            {
                tcnt += tnow - told;
            }
            else
            {
                tcnt += reload - told + tnow;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }

}



void mdelay(int ms)
{
    for (int i = 0; i < ms; i++)
        udelay(1000);
}





