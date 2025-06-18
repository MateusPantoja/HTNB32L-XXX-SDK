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

/* HTTP HOST */
#define TEST_HOST "http://api.openweathermap.org/"

/* HTTP path for the data that will be queried. */
#define TEST_SERVER_NAME "http://api.openweathermap.org/data/2.5/weather?q=porto+alegre&appid=3e53f1d247b84848790b704eaad25980&mode=xml&units=metric"

static volatile uint8_t simReady = 0;

uint8_t httpsSlpHandler = 0xff;
static StaticTask_t initTask;
static uint8_t appTaskStack[INIT_TASK_STACK_SIZE];
static volatile uint32_t Event;
static QueueHandle_t psEventQueueHandle;
static uint8_t gImsi[16] = {0};
static HttpClientContext gHttpClient = {0};

static uint32_t uart_cntrl = (ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE | 
                                ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE);

extern USART_HandleTypeDef huart1;

static void HT_SetConnectioParameters(void) {
    uint8_t cid = 0;
    PsAPNSetting apnSetting;
    int32_t ret;
    uint8_t networkMode = 0; //nb-iot network mode
    uint8_t bandNum = 1; //number of bands that will be configured
    uint8_t band = 28; //band 28 for TIM network provider

    ret = appSetBandModeSync(networkMode, bandNum, &band);
    if(ret != CMS_RET_SUCC) {
        while(1);
    }

    apnSetting.cid = 0;
    apnSetting.apnLength = strlen("iot.datatem.com.br");
    strcpy((char *)apnSetting.apnStr, "iot.datatem.com.br");
    apnSetting.pdnType = CMI_PS_PDN_TYPE_IP_V4V6;
    appSetAPNSettingSync(&apnSetting, &cid);
}

static int32_t HT_HttpGetData(char *getUrl, char *buf, uint32_t len) {
    int32_t ret = HTTP_ERROR;
    HttpClientData    clientData = {0};
    uint32_t count = 0, recvTemp = 0, dataLen = 0;

    clientData.respBuf = buf;
    clientData.respBufLen = len;

    ret = httpSendRequest(&gHttpClient, getUrl, HTTP_GET, &clientData);
    if (ret < 0)
        return ret;
    do {
        ret = httpRecvResponse(&gHttpClient, &clientData);
        if (ret < 0) {
            return ret;
        }
        
        if (recvTemp == 0)
            recvTemp = clientData.recvContentLength;
        
        dataLen = recvTemp - clientData.needObtainLen;
        count += dataLen;
        recvTemp = clientData.needObtainLen;
    
    } while (ret == HTTP_MOREDATA);
    
    if (count != clientData.recvContentLength || gHttpClient.httpResponseCode < 200 || gHttpClient.httpResponseCode > 404) {
        return -1;
    } else if (count == 0) {
        return -2;
    } else {
        return ret;
    }
}

static void sendQueueMsg(uint32_t msgId, uint32_t xTickstoWait) {
    eventCallbackMessage_t *queueMsg = NULL;
    queueMsg = malloc(sizeof(eventCallbackMessage_t));
    queueMsg->messageId = msgId;
    if (psEventQueueHandle) {
        if (pdTRUE != xQueueSend(psEventQueueHandle, &queueMsg, xTickstoWait)) {
            printf("xQueueSend error\n");
        }
    }
}

static int32_t registerPSUrcCallback(urcID_t eventID, void *param, uint32_t paramLen) {
    CmiSimImsiStr *imsi = NULL;
    CmiPsCeregInd *creg = NULL;
    uint8_t rssi = 0;
    NmAtiNetifInfo *netif = NULL;

    switch(eventID) {
        case NB_URC_ID_SIM_READY:
        {
            imsi = (CmiSimImsiStr *)param;
            memcpy(gImsi, imsi->contents, imsi->length);
            HT_TRACE(UNILOG_PLA_APP, https_example_register_PSUrcCallback_0, P_INFO, 2, "SIM ready(imsi=%s len=%d)", imsi->contents, imsi->length);
            simReady = 1;
            break;
        }
        case NB_URC_ID_MM_SIGQ:
        {
            rssi = *(uint8_t *)param;
            HT_TRACE(UNILOG_PLA_APP, https_example_register_PSUrcCallback_1, P_INFO, 1, "RSSI signal=%d", rssi);
            break;
        }
        case NB_URC_ID_PS_BEARER_ACTED:
        {
            HT_TRACE(UNILOG_PLA_APP, https_example_register_PSUrcCallback_2, P_INFO, 0, "Default bearer activated");
            break;
        }
        case NB_URC_ID_PS_BEARER_DEACTED:
        {
            HT_TRACE(UNILOG_PLA_APP, https_example_register_PSUrcCallback_3, P_INFO, 0, "Default bearer Deactivated");
            sendQueueMsg(QMSG_ID_NW_DISCONNECT, 0);
            break;
        }
        case NB_URC_ID_PS_CEREG_CHANGED:
        {
            creg = (CmiPsCeregInd *)param;
            HT_TRACE(UNILOG_PLA_APP, https_example_register_registerPSUrcCallback_4, P_INFO, 4, "CREG message act:%d celId:%d locPresent:%d state:%d", creg->act, creg->celId, creg->locPresent, creg->state);
            break;
        }
        case NB_URC_ID_PS_NETINFO:
        {
            netif = (NmAtiNetifInfo *)param;
            if (netif->netStatus == NM_NETIF_ACTIVATED)
                sendQueueMsg(QMSG_ID_NW_IPV4_READY, 0);
            break;
        }
        default:
            break;
    }
    return 0;
}

static void HT_HttpsAppTask (void *argument) {
    int32_t ret = HTTP_ERROR;
    eventCallbackMessage_t *queueItem = NULL;

    char *recvBuf = malloc(HTTP_RECV_BUF_SIZE);
	memset(recvBuf, 0, HTTP_RECV_BUF_SIZE);

    HAL_USART_InitPrint(&huart1, GPR_UART1ClkSel_26M, uart_cntrl, 115200);
    printf("HTNB32L-XXX HTTPS!\n");
    printf("Trying to connect...\n");

    gHttpClient.caCert= PNULL;
    gHttpClient.caCertLen= 0;
    gHttpClient.timeout_s = 200;

    psEventQueueHandle = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(eventCallbackMessage_t*));
    if (psEventQueueHandle == NULL) {
        HT_TRACE(UNILOG_PLA_APP, httpsAppTask_0, P_ERROR, 0, "psEventQueue create error!");
        if(recvBuf != NULL) {
            free(recvBuf);
        }
        return;
    }
    registerPSEventCallback(NB_GROUP_ALL_MASK, registerPSUrcCallback);

    while(!simReady);
    HT_SetConnectioParameters();

    while(1) {
        if (xQueueReceive(psEventQueueHandle, &queueItem, portMAX_DELAY)) {
            printf("Connected!\n");
             
            switch(queueItem->messageId) {
                case QMSG_ID_NW_IPV4_READY:
                {
                    HT_TRACE(UNILOG_PLA_APP, httpsAppTask_1, P_ERROR, 0, "IP got ready");
                    ret = httpConnect(&gHttpClient, TEST_HOST);
                    if (!ret) {
                        HT_HttpGetData(TEST_SERVER_NAME, recvBuf, HTTP_RECV_BUF_SIZE);
                        printf("\nHTTPS Data: \n%s\n", recvBuf);
                    } else {
                        HT_TRACE(UNILOG_PLA_APP, httpsAppTask_2, P_ERROR, 0, "http client connect error");
                        printf("HTTPS Error: %d\n", ret);
                    }

                    httpClose(&gHttpClient);
					char *showBuf = malloc(100);
					memset(showBuf, 0, 100);
					memcpy(showBuf,recvBuf,96);
					showBuf[96]='.';
					showBuf[97]='.';
					showBuf[98]='.';
					vTaskDelay(10000);
                    HT_STRING(UNILOG_PLA_STRING, httpsAppTask_3, P_INFO, "%s", (uint8_t*)showBuf);
                    break;
                 }				 
                 default:
                    break;
             }
             free(queueItem);
         }
     }
	free(recvBuf);
}

static void HT_AppInit(void *arg) {
    osThreadAttr_t task_attr;

    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_CONTROL) != 0) {
        HAL_UART_RecvFlowControl(false);
    }

    memset(&task_attr,0,sizeof(task_attr));
    memset(appTaskStack, 0xA5,INIT_TASK_STACK_SIZE);
    task_attr.name = "HT_HttpsExample";
    task_attr.stack_mem = appTaskStack;
    task_attr.stack_size = INIT_TASK_STACK_SIZE;
    task_attr.priority = osPriorityNormal;
    task_attr.cb_mem = &initTask;//task control block
    task_attr.cb_size = sizeof(StaticTask_t);//size of task control block

    osThreadNew(HT_HttpsAppTask, NULL, &task_attr);
}

void main_entry(void) {

    BSP_CommonInit();
    osKernelInitialize();
    
    // Apply own right power policy according to design
    slpManSetPmuSleepMode(true, SLP_HIB_STATE, false);
    slpManApplyPlatVoteHandle("HTTPS",&httpsSlpHandler);
    
    slpManPlatVoteDisableSleep(httpsSlpHandler, SLP_SLP2_STATE);
    
    setvbuf(stdout, NULL, _IONBF, 0);

    registerAppEntry(HT_AppInit, NULL);
    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

    while(1);
}


/************************ HT Micron Semicondutores S.A *****END OF FILE****/
