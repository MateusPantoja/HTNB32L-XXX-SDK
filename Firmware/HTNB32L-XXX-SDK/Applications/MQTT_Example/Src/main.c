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



static uint32_t uart_cntrl = (ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | 
                                ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE);

extern USART_HandleTypeDef huart1;

//volatile bool button_state = false;

#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(bool)

QueueHandle_t xFila;



void Task1(void *pvParameters) {
    bool button_state = false;
    bool state_past = false;

    while (1) {
      button_state = (bool) HT_GPIO_PinRead(BUTTON_INSTANCE, BUTTON_PIN);
      if(button_state != state_past) {
        printf("Enviando dado ...\n");
        xQueueSend(xFila, &button_state, portMAX_DELAY);
        state_past = button_state;
      }
      vTaskDelay(pdMS_TO_TICKS(100));      
    }
}

void Task2(void *pvParameters) {
    bool button_state = false;
    bool state = false;
    while (1) {
        if(xQueueReceive(xFila, &button_state, portMAX_DELAY)) printf("Recebido \n");
        if(button_state){
          state = !state;
          HT_GPIO_WritePin(LED_GPIO_PIN, LED_INSTANCE, state);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
  \fn          int main_entry(void)
  \brief       main entry function.
  \return
*/
void main_entry(void) {

    HAL_USART_InitPrint(&huart1, GPR_UART1ClkSel_26M, uart_cntrl, 115200);
    
    HT_GPIO_InitButton();
    HT_GPIO_InitLed();

    slpManNormalIOVoltSet(IOVOLT_3_30V);

    xFila = xQueueCreate(QUEUE_LENGTH, sizeof(int));

    if(xFila == NULL){
      printf("Error ao criar Fila\n");
    } else {
      printf("Exemplo FreeRTOS\n");

      xTaskCreate(Task1, "Blink", 128, NULL, 1, NULL);
      xTaskCreate(Task2, "Print", 128, NULL, 1, NULL);

      vTaskStartScheduler();
    
      printf("Nao deve chegar aqui.\n");

      while(1);

    }
   
}

/******** HT Micron Semicondutores S.A **END OF FILE*/