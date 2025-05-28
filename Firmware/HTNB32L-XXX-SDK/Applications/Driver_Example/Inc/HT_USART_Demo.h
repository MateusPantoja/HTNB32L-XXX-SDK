/*

  _    _ _______   __  __ _____ _____ _____   ____  _   _
 | |  | |__   __| |  \/  |_   _/ ____|  __ \ / __ \| \ | |
 | |__| |  | |    | \  / | | || |    | |__) | |  | |  \| |
 |  __  |  | |    | |\/| | | || |    |  _  /| |  | | . ` |
 | |  | |  | |    | |  | |_| || |____| | \ \| |__| | |\  |
 |_|  |_|  |_|    |_|  |_|_____\_____|_|  \_\\____/|_| \_|
 =================== Advanced R&D ========================

 Copyright (c) 2024 HT Micron Semicondutores S.A.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

*/

/*!
 * \file HT_USART_Demo.h
 * \brief USART demonstration app. 
 * \author HT Micron Advanced R&D,
 *         Hêndrick Bataglin Gonçalves, Christian Roberto Lehmen,  Matheus da Silva Zorzeto, Felipe Kalinski Ferreira,
 *         Leandro Borges, Mauricio Carlotto Ribeiro, Henrique Kuhn, Cleber Haack, Eduardo Mendel
 * 
 * \link https://github.com/htmicron
 * \version 1.0
 * \date March 06, 2024
 */

#ifndef __HT_USART_DEMO_H__
#define __HT_USART_DEMO_H__

#include "main.h"
#include "HT_Peripheral_Config.h"
#include "htnb32lxxx_hal_usart.h"
#include "Driver_USART.h"

/* Defines  ------------------------------------------------------------------*/

#define USART_BUFFER_SIZE 100

/* Functions  ----------------------------------------------------------------*/

/*!******************************************************************
 * \fn void HT_USART_App(void)
 * \brief USART example main function.
 *
 * \param[in]  none
 * \param[out] none
 *
 * \retval none
 *******************************************************************/
void HT_USART_App(void);

#endif /*__HT_USART_DEMO_H__*/

/************************ HT Micron Semicondutores S.A *****END OF FILE****/
