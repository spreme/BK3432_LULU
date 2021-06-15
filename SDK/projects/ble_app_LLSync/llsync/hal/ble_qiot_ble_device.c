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

#include "ble_qiot_export.h"
#include "ble_qiot_service.h"
#include "ble_qiot_import.h"


// add ble qiot head file here
#include "ble_qiot_config.h"
#include "ble_qiot_log.h"
#include "ble_qiot_llsync_device.h"
#include "co_utils.h"
#include "flash.h"
#include "wdt.h"
// divece info which defined in explorer platform
#if 0
#define PRODUCT_ID  "13SJVR7LAS"
#define DEVICE_NAME "ble01"
#define SECRET_KEY  "Kd1fkCPvaoYWShcsmpeS9g=="
#else
#define PRODUCT_ID  "YU4OWQOEOP"
#define DEVICE_NAME "better"
#define SECRET_KEY  "DG/7QyUk1BmMORK2zHZgdQ=="
#endif
#if 0
int ble_get_product_id(char *product_id)
{

    memcpy(product_id, PRODUCT_ID, strlen(PRODUCT_ID));

    return 0;
}

int ble_get_device_name(char *device_name)
{
    memcpy(device_name, DEVICE_NAME, strlen(DEVICE_NAME));

    return strlen(DEVICE_NAME);
}

int ble_get_psk(char *psk)
{
    memcpy(psk, SECRET_KEY, strlen(SECRET_KEY));

    return 0;
}

int ble_get_mac(char *mac)
{
  //  char *address = (char *)esp_bt_dev_get_address();
  //  memcpy(mac, address, 6);
	  for(char i=0;i<6;i++)
		mac[i]=co_default_bdaddr.addr[5-i];
    return 0;
}
#endif
int ble_get_device_info(ble_device_info * device_info)
{
	ble_device_info info;
	extern int memchk(const uint8_t *buf, int len);

	flash_read(FLASH_SPACE_TYPE_MAIN, BLE_QIOT_DEVICE_INFO_FLASH_ADDR/4, sizeof(ble_device_info), (uint8_t *)&info);
	if (0 == memchk((const uint8_t *)&info, sizeof(ble_device_info))) {
		
		memcpy(device_info->product_id, PRODUCT_ID, strlen(PRODUCT_ID));
		memcpy(device_info->device_name, DEVICE_NAME, strlen(DEVICE_NAME));
		memcpy(device_info->psk, SECRET_KEY, strlen(SECRET_KEY));

    }else
    {
		memcpy(( uint8_t *)device_info, (uint8_t *)&info, sizeof(ble_device_info));
	}
	for(char i=0;i<6;i++)
	{
		device_info->mac[i]=co_default_bdaddr.addr[5-i];
	}
	return 0;

	
}
int ble_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
{
#if 1
	uint8_t backup_buffer[BLE_QIOT_RECORD_FLASH_PAGESIZE];
	uint32_t sect_addr;
	sect_addr = (flash_addr/BLE_QIOT_RECORD_FLASH_PAGESIZE)*BLE_QIOT_RECORD_FLASH_PAGESIZE;
	flash_read(FLASH_SPACE_TYPE_MAIN, sect_addr/4, BLE_QIOT_RECORD_FLASH_PAGESIZE, backup_buffer);
	
	flash_erase_sector(FLASH_SPACE_TYPE_MAIN, sect_addr/4);

	memcpy(backup_buffer+(flash_addr-sect_addr),write_buf,write_len);
	ble_qiot_log_i("==================ble_write_flash: %x", sect_addr);
	ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_INFO, "ginfo1", backup_buffer, write_len);
	flash_write(FLASH_SPACE_TYPE_MAIN, sect_addr/4, BLE_QIOT_RECORD_FLASH_PAGESIZE, backup_buffer);
	memset(backup_buffer,0,write_len);
	flash_read(FLASH_SPACE_TYPE_MAIN, sect_addr/4, write_len, backup_buffer);
		ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_INFO, "ginfo2", backup_buffer, write_len);
#else
   	flash_write(FLASH_SPACE_TYPE_MAIN, flash_addr/4, write_len, (uint8_t *)write_buf);
#endif
    return write_len;
}

int ble_read_flash(uint32_t flash_addr, char *read_buf, uint16_t read_len)
{
     flash_read(FLASH_SPACE_TYPE_MAIN , flash_addr/4, read_len, (uint8_t *)read_buf);
    return read_len;
}


ble_timer_t ble_timer_create(uint8_t type, ble_timer_cb timeout_handle)
{
     ble_timer_t p_timer;

	 
    return p_timer;
}

ble_qiot_ret_status_t ble_timer_start(ble_timer_t timer_id, uint32_t period)
{
	
    return BLE_QIOT_RS_OK;
}

ble_qiot_ret_status_t ble_timer_stop(ble_timer_t timer_id)
{

    return BLE_QIOT_RS_OK;
}

ble_qiot_ret_status_t ble_timer_delete(ble_timer_t timer_id)
{

    return BLE_QIOT_RS_OK;
}

// return ATT MTU
uint16_t ble_get_user_data_mtu_size(void)
{
    return 128;
}

uint8_t ble_ota_is_enable(const char *version)
{
    ble_qiot_log_e("ota version: %s, enable ota", version);
    return BLE_OTA_ENABLE;
}

uint32_t ble_ota_get_download_addr(void)
{
    ble_qiot_log_i("otafile download address: %d", BLE_QIOT_OTA_BACKUP_ADDR);
    return BLE_QIOT_OTA_BACKUP_ADDR;
}
int ble_ota_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
{
    int ret = 0;
    
    flash_write(FLASH_SPACE_TYPE_MAIN,flash_addr/4,write_len,(uint8_t *)write_buf);
	ble_qiot_log_i("write flash: %d", write_len);
	return write_len;
}
void ble_ota_start_cb(void)
{
    ble_qiot_log_i("ble ota start callback\r\n");
    return;
}

ble_qiot_ret_status_t ble_ota_valid_file_cb(uint32_t file_size, char *file_version)
{
    ble_qiot_log_i("user valid file, size %d, file_version: %s\r\n", file_size, file_version);
    return BLE_QIOT_RS_OK;
}



void ble_ota_stop_cb(uint8_t result)
{
	extern uint8_t ota_reset;
    ble_qiot_log_i("ble ota stop callback, result %d\r\n", result);
    if (result == BLE_QIOT_OTA_SUCCESS) {
		
		ble_qiot_log_i("wait for reset!!!\r\n");
		ota_reset=1;
    }else
    {
		 uint32_t earse_addr = BLE_QIOT_OTA_BACKUP_ADDR;
	    for(uint8_t i=0; i<92; i++)
	    {
	        flash_erase_sector(FLASH_SPACE_TYPE_MAIN, earse_addr/4);
	        earse_addr += 512;
	    }	
	}
    return;
}
#ifdef __cplusplus
}
#endif
