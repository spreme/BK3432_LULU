/*
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ble_qiot_config.h"
#include "ble_qiot_export.h"
#include "ble_qiot_service.h"
#include "ble_qiot_import.h"


#include "rwip_config.h"             // SW configuration

#include "ffe0s.h"
#include "ffe0s_task.h"
#include "prf_utils.h"
#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "ke_timer.h"
#include "ffe0s_task.h"                 // Application security Definition
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "wdt.h"
#include "rtc.h"
#include "att.h"
#define LLSYNC_LOG_TAG "LLSYNC"

/* Attributes State Machine */
enum {
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,
    IDX_CHAR_CFG_C,

    IDX_CHAR_D,
    IDX_CHAR_VAL_D,

    HRS_IDX_NB,
};

#define PROFILE_NUM        1
#define PROFILE_APP_IDX    0
#define LLSYNC_APP_ID      0x55
#define SAMPLE_DEVICE_NAME "l"
#define SVC_INST_ID        0

/* The max length of characteristic value. When the GATT client performs a write or prepare write operation,
 *  the data length must be less than LLSYNC_CHAR_VAL_LEN_MAX.
 */
#define LLSYNC_CHAR_VAL_LEN_MAX 128
#define PREPARE_BUF_MAX_SIZE    1024
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

#define ADV_CONFIG_FLAG (1 << 0)

static uint8_t adv_config_done = 0;

uint16_t llsync_handle_table[HRS_IDX_NB];

typedef struct {
    uint8_t *prepare_buf;
    int      prepare_len;
    uint16_t handle;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

static uint8_t raw_adv_data[32] = {
    /* flags */
    0x02,
    0x01,
    0x06,
    /* service uuid */
    0x03,
    0x03,
    0xE0,
    0xFF,
};

ble_qiot_ret_status_t ble_advertising_start(adv_info_s *adv)
{	
	uint8_t usr_adv_data[31] = {0};
    uint8_t len              = 0;
    uint8_t index            = 0;
	
    // Check if the advertising procedure is already is progress
    if (ke_state_get(TASK_APP) == APPM_READY)
    {				
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = APP_ADV_CHMAP;
        cmd->intv_min 		= APP_ADV_INT_MIN;
        cmd->intv_max 		= APP_ADV_INT_MAX;	
        
		cmd->op.code = GAPM_ADV_UNDIRECT;
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = 0;
        cmd->info.host.scan_rsp_data_len  = 0;
		

	//if(adv->manufacturer_info.adv_data[0]==0x22)
	//{
	//	cmd->op.code = GAPM_ADV_DIRECT;
	//}else
	//{
	//	cmd->op.code = GAPM_ADV_UNDIRECT;
	//}

    memcpy(usr_adv_data, &adv->manufacturer_info.company_identifier, sizeof(uint16_t));
    len = sizeof(uint16_t);
    memcpy(usr_adv_data + len, adv->manufacturer_info.adv_data, adv->manufacturer_info.adv_data_len);
    len += adv->manufacturer_info.adv_data_len;

    index                 = 7;
    raw_adv_data[index++] = len + 1;
    raw_adv_data[index++] = 0xFF;
    memcpy(raw_adv_data + index, usr_adv_data, len);
    index += len;

    raw_adv_data[index++] = strlen(SAMPLE_DEVICE_NAME) + 1;
    raw_adv_data[index++] = 0x09;
    memcpy(raw_adv_data + index, SAMPLE_DEVICE_NAME, strlen(SAMPLE_DEVICE_NAME));
    index += strlen(SAMPLE_DEVICE_NAME);
	
     memcpy(&cmd->info.host.adv_data[0],raw_adv_data,index);
     cmd->info.host.adv_data_len += index;
    
        // Send the message
        ke_msg_send(cmd);
	 	UART_PRINTF("appm start advertising\r\n");

		wdt_enable(0x3fff);

        // Set the state of the task to APPM_ADVERTISING
        ke_state_set(TASK_APP, APPM_ADVERTISING);	

		//ke_timer_set(APP_PERIOD_TIMER, TASK_APP, 1500);	
    }
	return BLE_QIOT_RS_OK;
    // else ignore the request
}

ble_qiot_ret_status_t ble_advertising_stop(void)
{
    if (ke_state_get(TASK_APP) == APPM_ADVERTISING)
    {
        // Go in ready state
        ke_state_set(TASK_APP, APPM_READY);

        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        // Send the message
        ke_msg_send(cmd);

		wdt_disable_flag = 1;
    }
    return 0;
}









void ble_services_add(const qiot_service_init_s *p_service)
{
    // do nothing
    return;
}


#ifdef __cplusplus
}
#endif
