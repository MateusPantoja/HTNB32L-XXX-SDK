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

#include "HT_Sleep_Example.h"

extern void pmuSlpTestExtWakeupHibernate2(void);
extern void pmuSlpTestExtWakeupHibernate1(void);
extern void pmuSlpTestExtWakeupSleep2(void);
extern void pmuSlpTestExtWakeupSleep1(void);

/* Function prototypes  ------------------------------------------------------------------*/

/*!******************************************************************
 * \fn static void HT_SleepMode1(void)
 * \brief SleepMode1 test implementation.
 *
 * \param[in]  none
 * \param[out] none
 *
 * \retval none.
 *******************************************************************/
static void HT_SleepMode1(void);

/*!******************************************************************
 * \fn static void HT_SleepMode2(void)
 * \brief SleepMode2 test implementation.
 *
 * \param[in]  none
 * \param[out] none
 *
 * \retval none.
 *******************************************************************/
static void HT_SleepMode2(void);

/*!******************************************************************
 * \fn static void HT_HibernateMode1(void)
 * \brief HibernateMode1 test implementation.
 *
 * \param[in]  none
 * \param[out] none
 *
 * \retval none.
 *******************************************************************/
static void HT_HibernateMode1(void);

/*!******************************************************************
 * \fn static void HT_HibernateMode2(void)
 * \brief HibernateMode2 test implementation.
 *
 * \param[in]  none
 * \param[out] none
 *
 * \retval none.
 *******************************************************************/
static void HT_HibernateMode2(void);

/* ---------------------------------------------------------------------------------------*/

static void HT_SleepMode1(void) {
    BootFlag_e boot_flag;

    printf("Start Sleep1 Test......\r\n");
    pmuSlpTestExtWakeupSleep1();

    HT_BSP_Deinit();
    HT_BSP_Init();

	pmuRestoreBootFlag();
    boot_flag = pmuGetBootFlag();
	
    if(pmuGetSleepedFlag())    //sleeped
        printf("Test Finish: Sleep1 Success.-BootFrom=%d\r\n",boot_flag);
    else
        printf("Test Finish: Failed to Enter Sleep1.\r\n");

    HT_TRACE(UNILOG_PLA_DRIVER, main_entry_11, P_WARNING, 1, "Re Boot Up from %u -> Unilog Print OK",boot_flag);
	uniLogFlushOut(0);
	
    while(1);
}

static void HT_SleepMode2(void) {
    printf("Start Sleep2 Test......\r\n");
    pmuSlpTestExtWakeupSleep2();

    printf("Test Failed: Failed to Enter Sleep2.\r\n");
    while(1);
}

static void HT_HibernateMode1(void) {
    printf("Start Hibernate1 Test......\r\n");
    pmuSlpTestExtWakeupHibernate1();

    printf("Test Failed: Failed to Enter Hibernate1.\r\n");
    while(1);
}

static void HT_HibernateMode2(void) {
    printf("Start Hibernate2 Test......\r\n");
    pmuSlpTestExtWakeupHibernate2();

    printf("Test Failed: Failed to Enter Hibernate2.\r\n");
    while(1);
}

void HT_App(void) {
    char case_num;
    
    while(1) {
        slpManWakeSrc_e wkpsrc = slpManGetWakeupSrc();

        printf("Fonte de Wake %d\n\n", wkpsrc);

        printf("Case 0: Sleep1 Test\r\n");
        printf("Case 1: Sleep2 Test\r\n");
        printf("Case 2: Hibernate1 Test\r\n");
        printf("Case 3: Hibernate2 Test\r\n");
        printf("Input Case Number:");

        __set_BASEPRI(0);

        case_num = (char)fgetc(NULL);
        printf("%c\r\n",case_num);

        switch(case_num) {
            case '0':
                HT_SleepMode1();
                break;
            case '1':
                HT_SleepMode2();
                break;
            case '2':
                HT_HibernateMode1();
                break;
            case '3':
                HT_HibernateMode2();
                break;
            default: 
                printf("Error! Wrong option!\n");
                break;
        }

    }
}

/************************ HT Micron Semicondutores S.A *****END OF FILE****/
