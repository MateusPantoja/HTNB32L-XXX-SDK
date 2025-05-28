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

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>

#include "hal_adc.h"
#include "HT_adc_qcx212.h"
#include <stdio.h>
#include "string.h"
#include "adc_qcx212.h"
#include "HT_bsp.h"
#include "stdint.h"

#include "slpman_qcx212.h"
#include "pad_qcx212.h"
#include "HT_gpio_qcx212.h"
#include "ic_qcx212.h"
#include "HT_ic_qcx212.h"
#include "HT_GPIO_Demo.h"



#define USART_BUFFER_SIZE 100
#define DEMO_ADC_CHANNEL ADC_ChannelAio2  


static volatile uint32_t callback = 0;
static volatile uint32_t user_adc_channel = 0;
static adc_config_t adcConfig;

extern USART_HandleTypeDef huart1;

static uint8_t rx_buffer[USART_BUFFER_SIZE] = {0};
uint8_t controlByte[1] = {0};

QueueHandle_t xFilaFrequencia;
QueueHandle_t xTimeBtn;
QueueHandle_t xAdc;


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


void uart_receive_cmd(void *pvParameters){
    while(1) {
        printf("Freq led 1 >>\n");
        uint32_t idx = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));

        while(1){
            HAL_USART_ReceivePolling(&huart1, controlByte, 1);
                
                if (controlByte[0] == '\n') {
                    break;
                }

                if (idx < USART_BUFFER_SIZE - 1) {  // Evita ultrapassar os limites do buffer
                    rx_buffer[idx++] = controlByte[0];
                }
        }
        rx_buffer[idx] = '\0';
        printf("Valor de Freq ->: %s\n", (char *)rx_buffer);
        
        int nova_freq = atoi((char *)rx_buffer);

        if (xQueueSend(xFilaFrequencia, &nova_freq, portMAX_DELAY) == pdTRUE) {
                printf("Frequência enviada para a fila: %d Hz\n", nova_freq);
        } else {
            printf("Falha ao enviar frequência para fila.\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void control_led2(void *pvParameters) {
   bool state = false;
    int frequencia = 1;

    while (1) {
        // Espera por um novo valor de frequência na fila
        if (xQueueReceive(xFilaFrequencia, &frequencia, portMAX_DELAY) == pdTRUE) {
            printf("Frequência configurada para: %d Hz\n", frequencia);

            if (frequencia == 0) {
                // Desliga o LED e espera nova frequência
                state = false;
                HT_GPIO_WritePin(LED2_GPIO_PIN, LED2_INSTANCE, state);

                while (1) {
                    int nova_freq = 0;
                    if (xQueueReceive(xFilaFrequencia, &nova_freq, portMAX_DELAY) == pdTRUE) {
                        frequencia = nova_freq;
                        printf("Frequência configurada para: %d Hz\n", frequencia);
                        break;
                    }
                }
            }

            // Piscar enquanto frequência > 0
            while (frequencia > 0) {
                state = !state;
                HT_GPIO_WritePin(LED2_GPIO_PIN, LED2_INSTANCE, state);

                int periodo_ms = 1000 / frequencia;
                vTaskDelay(pdMS_TO_TICKS(periodo_ms / 2));

                int nova_freq = 0;
                if (xQueueReceive(xFilaFrequencia, &nova_freq, 0) == pdTRUE) {
                    frequencia = nova_freq;
                    printf("Frequência configurada para: %d Hz\n", frequencia);
                    break;
                }
            }
        }
    }
}

void read_button(void *pvParameters){
    volatile uint32_t value = 0;
    uint32_t count = 0;

    while (1)
    {
        value = HT_GPIO_PinRead(BUTTON_INSTANCE, BUTTON_PIN);
        count = 0;
        while(value == 0){
            printf("btn pressionado por %d s\n", count);
            count++;
            vTaskDelay(pdMS_TO_TICKS(1000));
            value = HT_GPIO_PinRead(BUTTON_INSTANCE, BUTTON_PIN);
        }
        if(count != 0){
            if (xQueueSend(xTimeBtn, &count, portMAX_DELAY) == pdTRUE) {
                printf("Tempo atualizado para %ds\n", count);
            } else {
                printf("Falha ao enviar tempo para fila.\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));      
    }
    
}

void control_led3(void *pvParameters){

    uint32_t ntempo = 1;
    while (1)
    {
        if (xQueueReceive(xTimeBtn, &ntempo, 0) == pdTRUE) {
            printf("Tempo configurado para %ds\n", ntempo);
        }
        HT_GPIO_WritePin(LED3_GPIO_PIN, LED3_INSTANCE, 0);
        vTaskDelay(pdMS_TO_TICKS(ntempo*1000));
        HT_GPIO_WritePin(LED3_GPIO_PIN, LED3_INSTANCE, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    

}

void read_adc(void *pvParameters){
    uint32_t adc_value = 0;

    while (1)
    {
        callback = 0;

        HT_ADC_StartConversion(DEMO_ADC_CHANNEL, ADC_UserAPP);

        while(callback != (DEMO_ADC_CHANNEL));

        adc_value = HT_ADC_GetVoltageValue(user_adc_channel);
        
        if(adc_value < 2500){
            HT_GPIO_WritePin(LED1_GPIO_PIN, LED1_INSTANCE, 0);
        } else {
            HT_GPIO_WritePin(LED1_GPIO_PIN, LED1_INSTANCE, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}


/**
  \fn          int main_entry(void)
  \brief       main entry function.
  \return
*/
void main_entry(void) {
    
    uint32_t uart_cntrl = (ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | 
                                ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE);

    BSP_CommonInit();

    HAL_USART_InitPrint(&huart1, GPR_UART1ClkSel_26M, uart_cntrl, 115200);
    
    HT_GPIO_InitLed1();
    HT_GPIO_InitLed2();
    HT_GPIO_InitLed3();

    HT_GPIO_InitButton();


    slpManNormalIOVoltSet(IOVOLT_3_30V);

    
    
    xFilaFrequencia = xQueueCreate(5, sizeof(int));  // Fila de até 5 inteiros
    if (xFilaFrequencia == NULL) {
        printf("Erro ao criar a fila de frequências!\n");
        while(1);
    }

    xTimeBtn = xQueueCreate(5, sizeof(int));  // Fila de até 5 inteiros
    if (xFilaFrequencia == NULL) {
        printf("Erro ao criar a fila de times!\n");
        while(1);
    }

    printf("Exemplo FreeRTOS\n");

    HT_ADC_Init(DEMO_ADC_CHANNEL);


    xTaskCreate(control_led2, "controle_led2", 128, NULL, 10, NULL);
    xTaskCreate(uart_receive_cmd, "Uart_Rcv", 1024, NULL, 1, NULL);
    xTaskCreate(read_button, "LeituraBtn", 128, NULL, 1, NULL);
    xTaskCreate(control_led3, "controle_led3", 128, NULL, 1, NULL);
    xTaskCreate(read_adc, "read_adc", 512, NULL, 5, NULL);
    
    vTaskStartScheduler();
    
   
  
    printf("Nao deve chegar aqui.\n");

    while(1);


}

/******** HT Micron Semicondutores S.A **END OF FILE*/