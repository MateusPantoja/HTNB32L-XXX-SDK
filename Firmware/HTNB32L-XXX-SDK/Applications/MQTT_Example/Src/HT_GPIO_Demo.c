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

#include "HT_GPIO_Demo.h"

volatile uint8_t gpio_exti = 1;

void HT_GPIO_Callback(void);
void HT_GPIO_InitButton(void);
void HT_GPIO_InitLed3(void);
void HT_GPIO_InitLed2(void);
void HT_GPIO_InitLed1(void);

void HT_GPIO_Callback(void) {
  //Save current irq mask and diable whole port interrupts to get rid of interrupt overflow
  uint16_t portIrqMask = GPIO_SaveAndSetIRQMask(BUTTON_INSTANCE);

  if (HT_GPIO_GetInterruptFlags(BUTTON_INSTANCE) & (1 << BUTTON_PIN)) {
      gpio_exti ^= 1;
      HT_GPIO_ClearInterruptFlags(BUTTON_INSTANCE, 1 << BUTTON_PIN);
      delay_us(10000000);
  }

  HT_GPIO_RestoreIRQMask(BUTTON_INSTANCE, portIrqMask);
}

void HT_GPIO_InitButton(void) {
  GPIO_InitType GPIO_InitStruct = {0};

  GPIO_InitStruct.af = PAD_MuxAlt0;
  GPIO_InitStruct.pad_id = BUTTON_PAD_ID;
  GPIO_InitStruct.gpio_pin = BUTTON_PIN;
  GPIO_InitStruct.pin_direction = GPIO_DirectionInput;
  GPIO_InitStruct.pull = PAD_InternalPullUp;
  GPIO_InitStruct.instance = BUTTON_INSTANCE;
  GPIO_InitStruct.exti = GPIO_EXTI_DISABLED;
 // GPIO_InitStruct.interrupt_config = GPIO_InterruptFallingEdge;

  HT_GPIO_Init(&GPIO_InitStruct);

  // Enable IRQ
  //HT_XIC_SetVector(PXIC_Gpio_IRQn, HT_GPIO_Callback);
  //HT_XIC_EnableIRQ(PXIC_Gpio_IRQn);
}

void HT_GPIO_InitLed3(void) {
  GPIO_InitType GPIO_InitStruct = {0};

  GPIO_InitStruct.af = PAD_MuxAlt0;
  GPIO_InitStruct.pad_id = LED3_PAD_ID;
  GPIO_InitStruct.gpio_pin = LED3_GPIO_PIN;
  GPIO_InitStruct.pin_direction = GPIO_DirectionOutput;
  GPIO_InitStruct.init_output = 0;
  GPIO_InitStruct.pull = PAD_AutoPull;
  GPIO_InitStruct.instance = LED3_INSTANCE;
  GPIO_InitStruct.exti = GPIO_EXTI_DISABLED;

  HT_GPIO_Init(&GPIO_InitStruct);
}

void HT_GPIO_InitLed2(void) {
  GPIO_InitType GPIO_InitStruct = {0};

  GPIO_InitStruct.af = PAD_MuxAlt0;
  GPIO_InitStruct.pad_id = LED2_PAD_ID;
  GPIO_InitStruct.gpio_pin = LED2_GPIO_PIN;
  GPIO_InitStruct.pin_direction = GPIO_DirectionOutput;
  GPIO_InitStruct.init_output = 0;
  GPIO_InitStruct.pull = PAD_AutoPull;
  GPIO_InitStruct.instance = LED2_INSTANCE;
  GPIO_InitStruct.exti = GPIO_EXTI_DISABLED;

  HT_GPIO_Init(&GPIO_InitStruct);
}

void HT_GPIO_InitLed1(void) {
  GPIO_InitType GPIO_InitStruct = {0};

  GPIO_InitStruct.af = PAD_MuxAlt0;
  GPIO_InitStruct.pad_id = LED1_PAD_ID;
  GPIO_InitStruct.gpio_pin = LED1_GPIO_PIN;
  GPIO_InitStruct.pin_direction = GPIO_DirectionOutput;
  GPIO_InitStruct.init_output = 0;
  GPIO_InitStruct.pull = PAD_AutoPull;
  GPIO_InitStruct.instance = LED1_INSTANCE;
  GPIO_InitStruct.exti = GPIO_EXTI_DISABLED;

  HT_GPIO_Init(&GPIO_InitStruct);
}
/*
void HT_GPIO_App(void) {

  ht_printf("GPIO Example Start!\n");
  
  HT_GPIO_InitButton();
  HT_GPIO_InitLed();

  slpManNormalIOVoltSet(IOVOLT_3_30V);

  volatile uint32_t value = 0;

  while(1) {

    value = HT_GPIO_PinRead(BUTTON_INSTANCE, BUTTON_PIN);

    ht_printf("Lendo valor Button ");
    ht_printf("valor - %d \n", value);

    if (!value)
        HT_GPIO_WritePin(LED_GPIO_PIN, LED_INSTANCE, LED_ON);
    else
        HT_GPIO_WritePin(LED_GPIO_PIN, LED_INSTANCE, LED_OFF);
    
    delay_us(10000000);
  
  }
}
*/
/************************ HT Micron Semicondutores S.A *****END OF FILE****/
