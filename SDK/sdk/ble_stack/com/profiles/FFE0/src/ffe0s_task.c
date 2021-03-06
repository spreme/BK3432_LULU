/**
 ****************************************************************************************
 *
 * @file   ffe0s_task.c
 *
 * @brief FFE0 Server Role Task Implementation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FFE0_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "co_utils.h"
#include "ke_mem.h"
#include "ffe0s.h"
#include "ffe0s_task.h"

#include "prf_utils.h"
#include "uart.h"

#include "ble_qiot_export.h"

static int ffe0s_enable_req_handler(ke_msg_id_t const msgid,
                                   struct ffe0s_enable_req const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_SAVED;
    uint8_t state = ke_state_get(dest_id);

    // check state of the task
    if(state == FFE0S_IDLE)
    {
        struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct ffe0s_enable_rsp* rsp = KE_MSG_ALLOC(FFE0S_ENABLE_RSP, src_id,
                dest_id, ffe0s_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            ke_msg_send(rsp);

            msg_status = KE_MSG_CONSUMED;
        }
       
    }

    return msg_status;
}

static void ffe0s_notify_ffe1_val(struct ffe0s_env_tag* ffe0s_env,struct ffe0s_ffe3_value_upd_req const *param)
{
	
	//UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *ffe1_value = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(ffe0s_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    ffe1_value->operation = GATTC_NOTIFY;
    ffe1_value->handle = ffe0s_get_att_handle(FFE0S_IDX_FFE3_VAL_VALUE);
    // pack measured value in database
    ffe1_value->length = param->length;
	uart_printf("\r\n");      
	for(int i=0;i<ffe1_value->length;i++)
		uart_printf("0x%x ",param->value[i]);
	uart_printf("\r\n");     	
	memcpy(&ffe1_value->value[0],param->value,ffe1_value->length);
		
    //send notification to peer device
    ke_msg_send(ffe1_value);
}

static int ffe0s_ffe1_value_upd_req_handler(ke_msg_id_t const msgid,
                                            struct ffe0s_ffe3_value_upd_req const *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
	// UART_PRINTF("%s\r\n",__func__);
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
	
    // check state of the task
    if(state == FFE0S_IDLE)
    {
        struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
	
        // update the battery level value
        ke_state_set(dest_id, FFE0S_BUSY);       

		ffe0s_notify_ffe1_val(ffe0s_env, param);
		ke_state_set(dest_id, FFE0S_IDLE);   
		msg_status = KE_MSG_CONSUMED;							       
    }
	else
	{
		msg_status = KE_MSG_SAVED;
	}

    return (msg_status);
  }

 ble_qiot_ret_status_t ble_send_notify(uint8_t *buf, uint8_t len)
{
    uint8_t state = ke_state_get(prf_get_task_from_id(TASK_ID_FFE0S));
	struct ffe0s_ffe3_value_upd_req param;
	
	UART_PRINTF("%s,%d\r\n",__func__,len);
    // check state of the task
    if(state == FFE0S_IDLE)
    {
        struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
	    param.conidx=0;
		param.length=len;
		memcpy(param.value,buf,len);
        // update the battery level value
        ke_state_set(prf_get_task_from_id(TASK_ID_FFE0S), FFE0S_BUSY);       
		
		ffe0s_notify_ffe1_val(ffe0s_env, &param);
		ke_state_set(prf_get_task_from_id(TASK_ID_FFE0S), FFE0S_IDLE);   
		return 0;
    }
	else
	{
	}

}
 
static int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t  att_idx = 0;
    // retrieve handle information
    uint8_t status = ffe0s_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == FFE0S_IDX_FFE3_VAL_NTF_CFG)
        {
            // CCC attribute length = 2
            cfm->length = 2;
        }
        // not expected request
        else
        {
            cfm->length = 0;
            status = ATT_ERR_WRITE_NOT_PERMITTED;
        }
    }

    cfm->status = status;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}



static int gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = ffe0s_get_att_idx(param->handle,  &att_idx);
		
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
        // Extract value before check
        uint16_t ntf_cfg = co_read16p(&param->value[0]);
		

		 //Send write response
	    cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
	    cfm->handle = param->handle;
	    cfm->status = status;
	    ke_msg_send(cfm);
        // Only update configuration if value for stop or notification enable
        if ((att_idx == FFE0S_IDX_FFE3_VAL_NTF_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {

            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                // Ntf cfg bit set to 1
                ffe0s_env->ntf_cfg[conidx] |= (FFE0_FFE1_LVL_NTF_SUP );
            }
            else
            {
                // Ntf cfg bit set to 0
                ffe0s_env->ntf_cfg[conidx] &= ~(FFE0_FFE1_LVL_NTF_SUP );
            }
					
        }
		else if (att_idx == FFE0S_IDX_FFE1_VAL_VALUE)
		{
			//uint8_t testbuff[2]={1,2};
			//ble_send_notify(testbuff,2);
			ble_device_info_write_cb(&param->value[0],param->length);
		}
        else if (att_idx == FFE0S_IDX_FFE2_VAL_VALUE)
		{
			ble_lldata_write_cb(&param->value[0],param->length);
		}
		else if (att_idx == FFE0S_IDX_FFE4_VAL_VALUE)
		{static int testcount=0;
			UART_PRINTF("ffw:%d,%d\r\n",param->length,testcount++);
			
			ble_ota_write_cb(&param->value[0],param->length);
		}else
        {
            status = PRF_APP_ERROR;
        }

    }

   

    return (KE_MSG_CONSUMED);
}   



static int gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n",__func__);
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = ffe0s_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
	UART_PRINTF("att_idx = %d\r\n",att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if ((att_idx == FFE0S_IDX_FFE3_VAL_NTF_CFG) )
        {
		
			 //Send read response
			cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
			cfm->handle = param->handle;
			cfm->status = status;
			cfm->length = 2;
			uint16_t ntf_cfg = (ffe0s_env->ntf_cfg[conidx] & FFE0_FFE1_LVL_NTF_SUP) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
			co_write16p(cfm->value, ntf_cfg);
		 	ke_msg_send(cfm);
        }
		
       
    }


    return (KE_MSG_CONSUMED);
}   

static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
  	UART_PRINTF("ffe0 %s\r\n",__func__);
	
//  	struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
    
    
        // continue operation execution
	/*	struct gattc_cmp_evt *evt = KE_MSG_ALLOC(FFE0S_GATTC_CMP_EVT,
        prf_dst_task_get(&(ffe0s_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
		evt->operation = param->operation;
		evt->status = param->status;
		evt->seq_num = param->seq_num;
			*/	
		ke_state_set(dest_id, FFE0S_IDLE); 
		//ke_msg_send(evt);					
    
		
    return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
const struct ke_msg_handler ffe0s_default_state[] =
{
    {FFE0S_ENABLE_REQ,              (ke_msg_func_t) ffe0s_enable_req_handler},
    {FFE0S_FFE1_VALUE_UPD_REQ,      (ke_msg_func_t) ffe0s_ffe1_value_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
	//{FFE0S_FFE1_READ_RSP,			(ke_msg_func_t) ffe0s_ffe1_read_rsp_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
		
};


/// Specifies the message handlers that are common to all states.
const struct ke_state_handler ffe0s_default_handler = KE_STATE_HANDLER(ffe0s_default_state);

#endif /* #if (BLE_FFE0_SERVER) */


 
