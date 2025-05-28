
/**
 *
 * Copyright (c) 2024 HT Micron Semicondutores S.A.
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

#include "HT_USART_Demo.h"

extern USART_HandleTypeDef huart1;
extern USART_HandleTypeDef huart2;

static uint8_t rx_buffer[USART_BUFFER_SIZE] = {0};
static uint8_t rx_buffer_at[USART_BUFFER_SIZE] = {0};


volatile uint8_t rx_callback = 0;
volatile uint8_t tx_callback = 0;

volatile uint8_t rx_callback_at = 0;
volatile uint8_t tx_callback_at = 0;


extern uint8_t *usart_tx_buffer;
extern uint8_t *usart_rx_buffer;
extern uint32_t usart_tx_buffer_size;
extern uint32_t usart_rx_buffer_size;

uint32_t uart_cntrl = (ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE);

    
void HT_USART_Callback(uint32_t event) {
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE)
        rx_callback = 1;
    
    if(event & ARM_USART_EVENT_TX_COMPLETE)
        tx_callback = 1;
}


void HT_USART_App(void) {

    ht_printf("HTNB32L-XXX USART Example\n");
    //HAL_USART_SendPolling(&huart2, "teste\n", 6);
    
   // ht_printf_at("Teste do pantoja\n");
    
    while(1) {
        uint8_t controlByte[1] = {0};

        HAL_USART_IRQnEnable(&huart1, (USART_IER_RX_DATA_REQ_Msk | USART_IER_RX_TIMEOUT_Msk | USART_IER_RX_LINE_STATUS_Msk));
        HAL_USART_Receive_IT(rx_buffer, USART_BUFFER_SIZE-1);
        ht_printf("Digite o cmd AT: \n");

        while(!rx_callback);
        rx_callback = 0;
        
        //rx_buffer[USART_BUFFER_SIZE] = '\r';
        //rx_buffer[USART_BUFFER_SIZE+1] = '\n';
        //rx_buffer[USART_BUFFER_SIZE+2] = '\0';

        ht_printf("Received: %s\n, ", (char *)rx_buffer);

        int count = 0;
        while(1){
            if(rx_buffer[count] == '\0' ) break;
            count ++;
        }
        
        HAL_USART_SendPolling(&huart2, rx_buffer, count + 1);
        ht_printf("%d\n", count);
        
        uint32_t idx = 0;

        while (1)
        {
            idx = 0;
            memset(rx_buffer_at, 0, sizeof(rx_buffer_at));

            while (1)
            {
                HAL_USART_ReceivePolling(&huart2, controlByte, 1);
                
                if (controlByte[0] == '\n') {
                    break;
                }

                if (idx < USART_BUFFER_SIZE - 1) {  // Evita ultrapassar os limites do buffer
                    rx_buffer_at[idx++] = controlByte[0];
                }

            }
            
            rx_buffer_at[idx] = '\0'; // Finaliza a string recebida

            if (strncmp(rx_buffer_at, "OK", 2) == 0 || strncmp(rx_buffer_at, "ERROR", 5) == 0) {
                break; // Sai do loop se for OK ou ERROR
            }
            
            ht_printf("Recebido: %s\n, ", (char *)rx_buffer_at);
        }
        
        
        ht_printf("Comando retornou: %s\n, ", (char *)rx_buffer_at);
      
        memset(controlByte, 0 , sizeof(controlByte));
        memset(rx_buffer_at, 0, sizeof(rx_buffer_at));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        HAL_USART_IRQnDisable(&huart1, (USART_IER_RX_DATA_REQ_Msk | USART_IER_RX_TIMEOUT_Msk | USART_IER_RX_LINE_STATUS_Msk));

    }
    
}

/************************ HT Micron Semicondutores S.A *****END OF FILE****/
