/**
 *
 * Copyright (c) 2023 HT Micron Semicondutores S.A.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "HT_LDR_Task.h"
//#include <stdio.h>

osMessageQueueId_t ldrQueue;

static volatile uint32_t callback = 0;
static volatile uint32_t user_adc_channel = 0;

static adc_config_t adcConfig;


static StaticTask_t             ldr_thread; //green_led_thread;
static UINT8                    ldr_TaskStack[LDR_TASK_STACK_SIZE]; //greenLedTaskStack[LED_TASK_STACK_SIZE];

extern volatile uint8_t button_irqn; 
extern volatile HT_Button button_color;

/* Function prototypes  ------------------------------------------------------------------*/
static void HT_LDR_Thread(void *arg);


/* ---------------------------------------------------------------------------------------*/

static void HT_ADC_ConversionCallback(uint32_t result) {
    callback |= DEMO_ADC_CHANNEL;
    user_adc_channel = result;
}

static uint32_t HT_ADC_GetVoltageValue(uint32_t ad_value) {
    uint32_t value;
    value = HAL_ADC_CalibrateRawCode(ad_value);
    return (uint32_t)(value*16/3);
}

static void HT_ADC_Init(uint8_t channel) {
    ADC_GetDefaultConfig(&adcConfig);

    adcConfig.channelConfig.aioResDiv = ADC_AioResDivRatio3Over16; 

    ADC_ChannelInit(channel, ADC_UserAPP, &adcConfig, HT_ADC_ConversionCallback);
}


static void HT_LDR_Thread(void *arg) {

    uint32_t value = 0;
    HT_ADC_Init(DEMO_ADC_CHANNEL);
    
    while (1) {
        callback = 0;

        HT_ADC_StartConversion(DEMO_ADC_CHANNEL, ADC_UserAPP);

        while(callback != (DEMO_ADC_CHANNEL));

        value = HT_ADC_GetVoltageValue(user_adc_channel);
        printf("ADC Value: %dmv\n", value);
        osMessageQueuePut(ldrQueue, &value, 0, 0);

        osDelay(READING_TIME_SECONDS*1000);
    }

}

void HT_LDR_Task(void *arg) {
    ldrQueue = osMessageQueueNew(10, sizeof(uint32_t), NULL);
    
    osThreadAttr_t task_attr;

    memset(&task_attr,0,sizeof(task_attr));
    memset(ldr_TaskStack, 0xA5,LDR_TASK_STACK_SIZE);
    task_attr.name = "ldr_thread";
    task_attr.stack_mem = ldr_TaskStack;
    task_attr.stack_size = LDR_TASK_STACK_SIZE;
    task_attr.priority = osPriorityNormal;
    task_attr.cb_mem = &ldr_thread;
    task_attr.cb_size = sizeof(StaticTask_t);

    osThreadNew(HT_LDR_Thread, NULL, &task_attr);
}
