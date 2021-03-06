/**
 ****************************************************************************************
 *
 * @file appm_task.c
 *
 * @brief RW APP Task implementation
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"          // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
#include "app_task.h"              // Application Manager Task API
#include "app.h"                      // Application Manager Definition
#include "gapc_task.h"            // GAP Controller Task API
#include "gapm_task.h"          // GAP Manager Task API
#include "gattc_task.h"
#include "arch.h"                    // Platform Definitions

#include "ke_timer.h"             // Kernel timer
#include "app_fcc0.h"              // fff0 Module Definition
#include "fcc0s_task.h"
#include "app_dis.h"              // Device Information Module Definition
#include "diss_task.h"
#include "app_batt.h"             // Battery Module Definition
#include "bass_task.h"
#include "app_oads.h"             
#include "oads_task.h"              
#include "gpio.h"
#include "uart.h"
#include "BK3432_reg.h"
#include "icu.h"
#include "reg_ble_em_cs.h"
#include "lld.h"
#include "wdt.h"
#include "utc_clock.h"
#include "tuya_ble_unix_time.h"
#include "tm1638.h"
/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static uint8_t appm_get_handler(const struct ke_state_handler *handler_list,
                                ke_msg_id_t msgid,
                                void *param,
                                ke_task_id_t src_id)
{
    // Counter
    uint8_t counter;

    // Get the message handler function by parsing the message table
    for (counter = handler_list->msg_cnt; 0 < counter; counter--)
    {
			
        struct ke_msg_handler handler = (*(handler_list->msg_table + counter - 1));
			
        if ((handler.id == msgid) ||
            (handler.id == KE_MSG_DEFAULT_HANDLER))
        {
            // If handler is NULL, message should not have been received in this state
            ASSERT_ERR(handler.func);

            return (uint8_t)(handler.func(msgid, param, TASK_APP, src_id));
        }
    }

    // If we are here no handler has been found, drop the message
    return (KE_MSG_CONSUMED);
}

/*
 * MESSAGE HANDLERS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles ready indication from the GAP. - Reset the stack
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    // Application has not been initialized
    ASSERT_ERR(ke_state_get(dest_id) == APPM_INIT);

    // Reset the stack
    struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD,
                                              TASK_GAPM, TASK_APP,
                                              gapm_reset_cmd);

    cmd->operation = GAPM_RESET;

    ke_msg_send(cmd);

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	UART_PRINTF("param->operation = 0x%x, param->status = 0x%x \r\n", param->operation, param->status);
    switch(param->operation)
    {
        // Reset completed
        case (GAPM_RESET):
        {
            if(param->status == GAP_ERR_NO_ERROR)
            {
                // Set Device configuration
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
	                                                                   TASK_GAPM, TASK_APP,
                                                                   gapm_set_dev_config_cmd);
                // Set the operation
                cmd->operation = GAPM_SET_DEV_CONFIG;
                // Set the device role - Peripheral
                cmd->role      = GAP_ROLE_PERIPHERAL;
                // Set Data length parameters
                cmd->sugg_max_tx_octets = BLE_MIN_OCTETS;
                cmd->sugg_max_tx_time   = BLE_MIN_TIME;
								
		 		cmd->max_mtu = 131;//BLE_MIN_OCTETS;
                //Do not support secure connections
                cmd->pairing_mode = GAPM_PAIRING_LEGACY;
                
                // load IRK
                memcpy(cmd->irk.key, app_env.loc_irk, KEY_LEN);

		        app_env.next_svc = 0;

                // Send message
                ke_msg_send(cmd);
            }
            else
            {
                ASSERT_ERR(0);
            }
        }
        break;
        case (GAPM_PROFILE_TASK_ADD):
        {
            // Add the next requested service
            if (!appm_add_svc())
            {
                // Go to the ready state
                ke_state_set(TASK_APP, APPM_READY);
							
				#if BLE_ADVERTISING
				appm_start_advertising();
				#endif
            }
        }
        break;
        // Device Configuration updated
        case (GAPM_SET_DEV_CONFIG):
        {
            ASSERT_INFO(param->status == GAP_ERR_NO_ERROR, param->operation, param->status);

            // Go to the create db state
            ke_state_set(TASK_APP, APPM_CREATE_DB);
            // Add the first required service in the database
            // and wait for the PROFILE_ADDED_IND
            appm_add_svc();
        }
        break;	

        case (GAPM_ADV_NON_CONN):
        case (GAPM_ADV_UNDIRECT):
        case (GAPM_ADV_DIRECT):
		case (GAPM_UPDATE_ADVERTISE_DATA):
        case (GAPM_ADV_DIRECT_LDC):
		{
			if (param->status == GAP_ERR_TIMEOUT)
			{
                ke_state_set(TASK_APP, APPM_READY);
				
				//device not bonded, start general adv
				#if BLE_ADVERTISING
				appm_start_advertising();
				#endif
            }
		}
        break;

        default:
        {
            // Drop the message
        }
        break;
    }

    return (KE_MSG_CONSUMED);
}

static int gapc_get_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_get_dev_info_req_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    switch(param->req)
    {
        case GAPC_DEV_NAME:
        {
            struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                                    src_id, dest_id,
                                                    gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
            cfm->req = param->req;
            cfm->info.name.length = appm_get_dev_name(cfm->info.name.value);

            // Send message
            ke_msg_send(cfm);
        } break;

        case GAPC_DEV_APPEARANCE:
        {
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                                    src_id, dest_id,
                                                    gapc_get_dev_info_cfm);
            cfm->req = param->req;
            
            // No appearance
            cfm->info.appearance = 0;

            // Send message
            ke_msg_send(cfm);
        } break;

        case GAPC_DEV_SLV_PREF_PARAMS:
        {
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                    								src_id, dest_id,
                                                    gapc_get_dev_info_cfm);
            cfm->req = param->req;
            // Slave preferred Connection interval Min
			cfm->info.slv_params.con_intv_min = BLE_UAPDATA_MIN_INTVALUE;
			// Slave preferred Connection interval Max
			cfm->info.slv_params.con_intv_max = BLE_UAPDATA_MAX_INTVALUE;
			// Slave preferred Connection latency
			cfm->info.slv_params.slave_latency = BLE_UAPDATA_LATENCY;
			// Slave preferred Link supervision timeout
			cfm->info.slv_params.conn_timeout  = BLE_UAPDATA_TIMEOUT;  // 6s (600*10ms)

            // Send message
            ke_msg_send(cfm);
        } break;

        default: /* Do Nothing */
			break;
    }


    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles GAPC_SET_DEV_INFO_REQ_IND message.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_set_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gapc_set_dev_info_req_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	// Set Device configuration
	struct gapc_set_dev_info_cfm* cfm = KE_MSG_ALLOC(GAPC_SET_DEV_INFO_CFM, src_id, dest_id,
                                                 gapc_set_dev_info_cfm);
	// Reject to change parameters
	cfm->status = GAP_ERR_REJECTED;
	cfm->req = param->req;
	// Send message
	ke_msg_send(cfm);

	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles connection complete event from the GAP. Enable all required profiles
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
                                           struct gapc_connection_req_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{	
	UART_PRINTF("%s\r\n", __func__);
	
    app_env.conidx = KE_IDX_GET(src_id);
    // Check if the received Connection Handle was valid
    if (app_env.conidx != GAP_INVALID_CONIDX)
    {
        // Retrieve the connection info from the parameters
        app_env.conhdl = param->conhdl;

        // Send connection confirmation
        struct gapc_connection_cfm *cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                gapc_connection_cfm);

        cfm->auth = GAP_AUTH_REQ_NO_MITM_NO_BOND;
        // Send the message
        ke_msg_send(cfm);

        /*--------------------------------------------------------------
         * ENABLE REQUIRED PROFILES
         *--------------------------------------------------------------*/
         
        // Enable Battery Service
        app_batt_enable_prf(app_env.conhdl);
		
        // We are now in connected State
        ke_state_set(dest_id, APPM_CONNECTED);
		
		#if UPDATE_CONNENCT_PARAM
		ke_timer_set(APP_PARAM_UPDATE_REQ_IND,TASK_APP,100); 
		#endif	
        ke_timer_set(APP_GATTC_EXC_MTU_CMD,TASK_APP,20);
	        
    }
    else
    {
        // No connection has been establish, restart advertising
		#if BLE_ADVERTISING
		appm_start_advertising();
		#endif
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles GAP controller command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	UART_PRINTF("gapc_cmp_evt_handler operation = %x\r\n",param->operation);
	switch(param->operation)
	{
    	case (GAPC_UPDATE_PARAMS):  //0x09
    	{
			if (param->status != GAP_ERR_NO_ERROR)
        	{
            	UART_PRINTF("gapc update params fail !\r\n");
			}
			else
			{
				UART_PRINTF("gapc update params ok !\r\n");
			}
			
    	} break;

		case (GAPC_SECURITY_REQ): //0x0c
		{
			if (param->status != GAP_ERR_NO_ERROR)
	        {
	            UART_PRINTF("gapc security req fail !\r\n");
	        }
	        else
	        {
	            UART_PRINTF("gapc security req ok !\r\n");
	        }
		}break;
		case (GAPC_BOND): // 0xa
    	{
	        if (param->status != GAP_ERR_NO_ERROR)
	        {
	            UART_PRINTF("gapc bond fail !\r\n");
	        }
	        else
	        {
	            UART_PRINTF("gapc bond ok !\r\n");
	        }
    	}break;
		
		case (GAPC_ENCRYPT): // 0xb
		{
			if (param->status != GAP_ERR_NO_ERROR)
			{
				UART_PRINTF("gapc encrypt start fail !\r\n");
			}
			else
			{
				UART_PRINTF("gapc encrypt start ok !\r\n");
			}
		}
		break;
		

    	default:
    	  break;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles disconnection complete event from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
	UART_PRINTF("disconnect link reason = 0x%x\r\n",param->reason);
	
    // Go to the ready state
    ke_state_set(TASK_APP, APPM_READY);

	wdt_disable_flag = 1;

	// Restart Advertising
	#if BLE_ADVERTISING
	appm_start_advertising();
	#endif
	
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles profile add indication from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapm_profile_added_ind_handler(ke_msg_id_t const msgid,
                                          struct gapm_profile_added_ind *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Current State
    uint8_t state = ke_state_get(dest_id);

    if (state == APPM_CREATE_DB)
    {
        switch (param->prf_task_id)
        {
            default: 
			break;
        }
    }
    else
    {
        ASSERT_INFO(0, state, src_id);
    }

    return KE_MSG_CONSUMED;
}


/*******************************************************************************
 * Function: app_period_timer_handler
 * Description: app period timer process
 * Input: msgid -Id of the message received.
 *		  param -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance (TASK_GAP).
 *		  ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int app_period_timer_handler(ke_msg_id_t const msgid,
                                          void *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
   	UART_PRINTF("%s\r\n", __func__);


    return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
 * @brief Handles reception of all messages sent from the lower layers to the application
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int appm_msg_handler(ke_msg_id_t const msgid,
                            void *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id)
{
    // Retrieve identifier of the task from received message
    ke_task_id_t src_task_id = MSG_T(msgid);
    // Message policy
    uint8_t msg_pol          = KE_MSG_CONSUMED;


    switch (src_task_id)
    {
        case (TASK_ID_GAPC):
        {
            // else drop the message
        } break;

        case (TASK_ID_GATTC):
        {
            // Service Changed - Drop
        } break;

        case (TASK_ID_FCC0S):
        {
            // Call the Health Thermometer Module
            msg_pol = appm_get_handler(&app_fcc0_table_handler, msgid, param, src_id);
        } break;
				
        case (TASK_ID_DISS):
        {
            // Call the Device Information Module
            msg_pol = appm_get_handler(&app_dis_table_handler, msgid, param, src_id);
        } break;

        case (TASK_ID_BASS):
        {
            // Call the Battery Module
            msg_pol = appm_get_handler(&app_batt_table_handler, msgid, param, src_id);
        } break;

        case (TASK_ID_OADS):
        {
            // Call the Health Thermometer Module
            msg_pol = appm_get_handler(&app_oads_table_handler, msgid, param, src_id);
        } break;

        default:
        {
        } break;
    }

    return (msg_pol);
}


/*******************************************************************************
 * Function: gapc_update_conn_param_req_ind_handler
 * Description: Update request command processing from slaver connection parameters
 * Input: msgid   -Id of the message received.
 *		  param   -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance
 *		  src_id  -ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int gapc_update_conn_param_req_ind_handler (ke_msg_id_t const msgid, 
									const struct gapc_param_update_req_ind  *param,
                 					ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

	UART_PRINTF("slave send param_update_req\r\n");
	struct gapc_conn_param  up_param;
	
	up_param.intv_min   = BLE_UAPDATA_MIN_INTVALUE;
	up_param.intv_max   = BLE_UAPDATA_MAX_INTVALUE; 
	up_param.latency    = BLE_UAPDATA_LATENCY;  
	up_param.time_out   = BLE_UAPDATA_TIMEOUT; 
	
	appm_update_param(&up_param);
	
	return KE_MSG_CONSUMED;
}

 
/*******************************************************************************
 * Function: gapc_le_pkt_size_ind_handler
 * Description: GAPC_LE_PKT_SIZE_IND
 * Input: msgid   -Id of the message received.
 *		  param   -Pointer to the parameters of the message.
 *		  dest_id -ID of the receiving task instance
 *		  src_id  -ID of the sending task instance.
 * Return: If the message was consumed or not.
 * Others: void
*******************************************************************************/
static int gapc_le_pkt_size_ind_handler (ke_msg_id_t const msgid, 
									const struct gapc_le_pkt_size_ind  *param,
                 					ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
   	UART_PRINTF("%s \r\n", __func__);
	UART_PRINTF("1max_rx_octets = %d\r\n",param->max_rx_octets);
	UART_PRINTF("1max_rx_time = %d\r\n",param->max_rx_time);
	UART_PRINTF("1max_tx_octets = %d\r\n",param->max_tx_octets);
	UART_PRINTF("1max_tx_time = %d\r\n",param->max_tx_time);
	
	return KE_MSG_CONSUMED;
}

/**
 ****************************************************************************************
 * @brief  GAPC_PARAM_UPDATED_IND
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_param_updated_ind_handler (ke_msg_id_t const msgid, 
									const struct gapc_param_updated_ind  *param,
                 					ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    UART_PRINTF("%s \r\n", __func__);
	UART_PRINTF("con_interval = %d\r\n",param->con_interval);
	UART_PRINTF("con_latency = %d\r\n",param->con_latency);
	UART_PRINTF("sup_to = %d\r\n",param->sup_to);
	
	return KE_MSG_CONSUMED;
}


/**
 ****************************************************************************************
 * @brief  GATTC_MTU_CHANGED_IND
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,
                                     struct gattc_mtu_changed_ind const *ind,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n",__func__);
	UART_PRINTF("ind->mtu = %d,seq = %d\r\n",ind->mtu,ind->seq_num);
	ke_timer_clear(APP_GATTC_EXC_MTU_CMD,TASK_APP);
 	return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief   GAPC_PARAM_UPDATE_REQ_IND
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gapc_param_update_req_ind_handler(ke_msg_id_t const msgid,
                                struct gapc_param_update_req_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n", __func__);
	// Prepare the GAPC_PARAM_UPDATE_CFM message
    struct gapc_param_update_cfm *cfm = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM,
                                             src_id, dest_id,
                                             gapc_param_update_cfm);
	 
	cfm->ce_len_max = 0xffff;
	cfm->ce_len_min = 0xffff;
	cfm->accept = true; 

	// Send message
    ke_msg_send(cfm);
	 
	return (KE_MSG_CONSUMED);
}

/**
 
*******************************************************************************
*********
 * @brief  GATTC_EXC_MTU_CMD
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 
*******************************************************************************
*********
 */
static int gattc_mtu_exchange_req_handler(ke_msg_id_t const msgid,
        struct gattc_exc_mtu_cmd const *req,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n", __func__);
	struct gattc_exc_mtu_cmd *cmd = KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
	                                KE_BUILD_ID(TASK_GATTC, app_env.conidx),
	                                TASK_APP,gattc_exc_mtu_cmd);
	cmd->operation = GATTC_MTU_EXCH;
	cmd->seq_num = 0;
	ke_msg_send(cmd);

	return (KE_MSG_CONSUMED);
}
/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */
uint32_t rec_key_tick = 0;

void rec_key_callback(void)
{
	beep_flag = 0;
	if(set_val_flag == 0)
	{
		if(gpio_get_input(RECORD_KEY) <= 0)
		{
			
			if(rec_key_tick <= 10)
			{
//				led_control(LED_OFF,LED_ON,10);
				SEG9[12]|=0X40;
				
				UART_PRINTF("play sound !!\r\n");

				play_record_control();
				
//				led_control(LED_ON,LED_OFF,0);
				SEG9[12]&=~0X40;
			}
			else
			{
				gpio_set(SOUND_REC, RECORD_OFF);
				UART_PRINTF("rec over !!\r\n");
				
				SEG9[12]&=~0X40;
				
	//			flash_erase(FLASH_SPACE_TYPE_NVR,BLE_SAVE_ADDR,FLASH_SIZE_ONE);
				flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR);
				flash_write(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *)&save_info);

//				led_control(LED_ON,LED_OFF,0);
				lock_timeout = LOCK_TIMEOUT_TIME;
			}
			rec_key_tick = 0;
		}
		else
		{			
			rec_key_tick++;
			
			SEG9[12]|=0X40;
			
			if(rec_key_tick > 10)
			{
				gpio_set(SOUND_REC, RECORD_ON);
				save_info.record_time++;
//				led_control(LED_OFF,LED_ON,10);
			}
			if(rec_key_tick >= 110)
			{
				rec_key_tick = 0;
				gpio_set(SOUND_REC, RECORD_OFF);
				UART_PRINTF("rec over !!\r\n");
//				flash_erase(FLASH_SPACE_TYPE_NVR,BLE_SAVE_ADDR,FLASH_SIZE_ONE);
				flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR);
				flash_write(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *)&save_info);
//				led_control(LED_ON,LED_OFF,0);
				lock_timeout = LOCK_TIMEOUT_TIME;
			}
			else
			{
	//			UART_PRINTF("@@@@@@@@@@@@@@@@@@@@@@@ REC_KEY_TASK exit\r\n");

				ke_timer_set(REC_KEY_TASK, TASK_APP, 10);
			}
		}
	}
}

enum KEY_STATE_E
{
	KEY_SET_E = 0x1,
	KEY_UP_E = 0x2,
	KEY_DOWM_E = 0x4,
	KEY_LOCK_E = 0x8,
	KEY_FEED_E = 0x10,	
	KEY_RECORD_E = 0x20,
	KEY_OK_E = 0x40,
};


uint8_t get_key_state()
{	
	uint8_t key_flag_e = 0;
		
	if(gpio_get_input(SET_KEY))
	{
		key_flag_e = key_flag_e | KEY_SET_E;
		//return KEY_SET_E;	
		SEG9[12]|=0X04;
	}
	else
	{
		SEG9[12]&=~0x04;
	}
	
	if(gpio_get_input(UP_KEY))
	{
		key_flag_e = key_flag_e | KEY_UP_E;
		if(set_val_flag == 0 && lock_flag == 0)
			SEG9[13]|=0XF;
		else if(set_val_flag)
			SEG9[12]|=0X20;			
	}
	else
	{
		SEG9[12]&=~0X20;
		SEG9[13]&=~0XF;
	}
	
	if(gpio_get_input(DOWN_KEY) <= 0)
	{
		key_flag_e = key_flag_e | KEY_DOWM_E;
		if(set_val_flag == 0 && lock_flag == 0)
			SEG9[12]|=0X01;
		else if(set_val_flag)
			SEG9[12]|=0X02;			
	}
	else
	{
		SEG9[12]&=~0x03;
	}
	
	if(gpio_get_input(RECORD_KEY) && lock_flag == 0)
	{
		key_flag_e = key_flag_e | KEY_RECORD_E;	
		SEG9[12]|=0X40;
	}
	else
	{
		SEG9[12]&=~0X40;
	}
	
	#ifdef LOCK_KEY_E
	if(gpio_get_input(LOCK_KEY))
	{
		key_flag_e = key_flag_e | KEY_LOCK_E;
		if(get_meal_flag == 1)
		{
			SEG9[6]|=0X80;
		}
		if(get_meal_flag == 3)
		{
			SEG9[6]&=~0X80;			
		}
	}
	#endif
	
	#ifdef FEED_KEY_E
	if(gpio_get_input(FEED_KEY) && lock_flag == 0)
	{
		key_flag_e = key_flag_e | KEY_FEED_E;
		SEG9[12]|=0X08;
	}
	else
	{
		SEG9[12]&=~0X08;
	}
	#endif
	
	#ifdef OK_KEY_E
	if(gpio_get_input(OK_KEY) && lock_flag == 0)
	{
		key_flag_e = key_flag_e | KEY_OK_E;	
		SEG9[12]|=0X10;
	}
	else
	{
		SEG9[12]&=~0x10;
	}
	#endif
		
	return key_flag_e;
}

uint32_t set_key_tick = 0;				//????????????
uint32_t dowm_key_tick = 0;				//??????????
uint32_t up_key_tick = 0;				//??????????
uint32_t lock_key_tick = 0;				//????????????
uint32_t feed_key_tick = 0;				//????????????
uint32_t record_key_tick = 0;			//????????????

uint32_t ok_key_tick = 0;

void key_scan_callback(void)
{
//	UART_PRINTF("key_scan_callback\r\n");
	static uint16_t reset_time = 0;

	keep_dowm_flag = get_key_state();
	if(keep_dowm_flag)
	{
		ke_timer_set(KEY_SCAN_TASK, TASK_APP, 10);	
	}
	
	if(key_scan_flag == 0)
	{
		if(gpio_get_input(SET_KEY) <= 0)
		{
			if(set_key_tick <= KEY_SHORT_TIME && set_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_SET_S;
				UART_PRINTF("key_flag = KEY_SET_S\r\n");
			}
			else if(set_key_tick > KEY_LONG_TIME_SET)
			{
				key_flag = KEY_SET_L_UP;
				UART_PRINTF("key_flag = KEY_SET_L_UP\r\n");
			}
			set_key_tick = 0;
		}
		else if(keep_dowm_flag == KEY_SET_E || key_flag == KEY_SET_L)
		{
			set_key_tick++;
			if(set_key_tick == KEY_LONG_TIME_SET)
			{
				key_flag = KEY_SET_L;
				UART_PRINTF("key_flag = KEY_SET_L\r\n");
//				set_key_tick = 0;
			}
		}
		
		if(gpio_get_input(UP_KEY) <= 0)
		{
			if(up_key_tick <= KEY_SHORT_TIME && up_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_UP_S;
				UART_PRINTF("key_flag = KEY_UP_S\r\n");
				
			}
			else if(up_key_tick > KEY_LONG_TIME)
			{
				key_flag = KEY_UP_L_UP;
				UART_PRINTF("key_flag = KEY_UP_L_UP\r\n");
				
			}
			up_key_tick = 0;		
		}
		else if(keep_dowm_flag == KEY_UP_E || key_flag == KEY_UP_L)
		{
			up_key_tick++;
			if(up_key_tick == KEY_LONG_TIME)
			{
				key_flag = KEY_UP_L;
				UART_PRINTF("key_flag = KEY_UP_L\r\n");
//				up_key_tick = 0;
			}
		}
		
		if(gpio_get_input(DOWN_KEY))
		{
			if(dowm_key_tick <= KEY_SHORT_TIME && dowm_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_DOWN_S;
				UART_PRINTF("key_flag = KEY_DOWN_S\r\n");
				
			}
			else if(dowm_key_tick > KEY_LONG_TIME)
			{
				key_flag = KEY_DOWN_L_UP;
				UART_PRINTF("key_flag = KEY_DOWN_L_UP\r\n");

			}
			dowm_key_tick = 0;		
		}
		else if(keep_dowm_flag == KEY_DOWM_E || key_flag == KEY_DOWM_L)
		{
			dowm_key_tick++;
			if(dowm_key_tick == KEY_LONG_TIME)
			{
				key_flag = KEY_DOWM_L;
				UART_PRINTF("key_flag = KEY_DOWM_L\r\n");
//				dowm_key_tick = 0;
			}
		}

		if(gpio_get_input(RECORD_KEY) <= 0)
		{
			if(record_key_tick <= KEY_SHORT_TIME && record_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_RECORD_S;
				UART_PRINTF("key_flag = KEY_RECORD_S\r\n");
			}
			else if(record_key_tick > KEY_LONG_TIME_SET)
			{
				key_flag = KEY_RECORD_L_UP;
				UART_PRINTF("key_flag = KEY_RECORD_L_UP\r\n");
			}
			record_key_tick = 0;
		}
		else if(keep_dowm_flag == KEY_RECORD_E || key_flag == KEY_RECORD_L)
		{
			record_key_tick++;
			if(record_key_tick == KEY_LONG_TIME_SET)
			{
				key_flag = KEY_RECORD_L;
				UART_PRINTF("key_flag = KEY_RECORD_L\r\n");
			}
		}

			
		#ifdef LOCK_KEY_E
		if(gpio_get_input(LOCK_KEY) <= 0)
		{
			if(lock_key_tick <= KEY_SHORT_TIME && lock_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_LOCK_S;
				UART_PRINTF("key_flag = KEY_LOCK_S\r\n");
			}
			else if(lock_key_tick > KEY_LONG_TIME_SET)
			{
				key_flag = KEY_LOCK_L_UP;
				UART_PRINTF("key_flag = KEY_LOCK_L_UP\r\n");
			}
			lock_key_tick = 0;
		}
		else if(keep_dowm_flag == KEY_LOCK_E || key_flag == KEY_LOCK_L)
		{
			lock_key_tick++;
			if(lock_key_tick == KEY_LONG_TIME_SET)
			{
				key_flag = KEY_LOCK_L;
				UART_PRINTF("key_flag = KEY_LOCK_L\r\n");	
//				lock_key_tick = 0;
			}
		}
		#endif
		
		
		#ifdef FEED_KEY_E
		if(gpio_get_input(FEED_KEY) <= 0)
		{
			if(feed_key_tick <= KEY_SHORT_TIME && feed_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_FEED_S;
				UART_PRINTF("key_flag = KEY_FEED_S\r\n");
			}
			else if(feed_key_tick > KEY_LONG_TIME)
			{
				key_flag = KEY_FEED_L_UP;
				UART_PRINTF("key_flag = KEY_FEED_L_UP\r\n");
			}
			feed_key_tick = 0;
		}
		else if(keep_dowm_flag == KEY_FEED_E || key_flag == KEY_FEED_L)
		{
			feed_key_tick++;
			if(feed_key_tick == KEY_LONG_TIME)
			{
				key_flag = KEY_FEED_L;
				UART_PRINTF("key_flag = KEY_FEED_L\r\n");
//				feed_key_tick = 0;
			}
		}
		#endif	
	}
	
		#ifdef OK_KEY_E
		if(gpio_get_input(OK_KEY) <= 0)
		{
			if(ok_key_tick <= KEY_SHORT_TIME && ok_key_tick != 0)				//????????????1s
			{
				key_flag = KEY_OK_S;
				UART_PRINTF("key_flag = KEY_OK_S\r\n");				
			}
			else if(ok_key_tick > KEY_LONG_TIME_SET)
			{
				key_flag = KEY_OK_L_UP;
				UART_PRINTF("key_flag = KEY_OK_L_UP\r\n");
			}
			ok_key_tick = 0;
		}
		else if(keep_dowm_flag == KEY_OK_E || key_flag == KEY_OK_L)
		{
			ok_key_tick++;
			if(ok_key_tick == KEY_LONG_TIME_SET)
			{
				key_flag = KEY_OK_L;
				UART_PRINTF("key_flag = KEY_OK_L\r\n");
				
//				ok_key_tick = 0;
			}
		}
		#endif

	
	#ifndef FEED_KEY_E
	if(gpio_get_input(UP_KEY) && gpio_get_input(SET_KEY))
//	if(gpio_get_input(UP_KEY) && gpio_get_input(SET_KEY) && gpio_get_input(OK_KEY))
	{
		if(set_time_flag == 0 && set_val_flag == 0 && lock_flag == 0)
		{
			feed_one_flag = 1;
		}
	}
	#endif
	
	if(lock_flag)
	{
		#if defined PT01K_BK
		if(gpio_get_input(SET_KEY))
//		#elif defined LNH_01
//		if(gpio_get_input(UP_KEY) && gpio_get_input(DOWN_KEY))
		#endif
		{
			reset_time++;
			if(reset_time >= 100)
			{
				reset_flag = 1;
				reset_time = 0;
			}			
		}
		else
		{
			reset_time = 0;
		}
	}
		
}

uint8_t reverse = 1;
uint8_t old_min = 66;
uint8_t save_time = 0;
uint8_t old_sec = 66;

int i;
void utc_callback(void)
{
//	tuya_ble_time_struct_data_t t_struct;
//	uint32_t t_now = 0;
	UTCTimeStruct tm_s;

	utc_get_time(&tm_s);

//	memset(&t_struct, 0, sizeof(tuya_ble_time_struct_data_t));
		
//	utc_update();
//	t_now = utc_get_clock();
//	tuya_ble_utc_sec_2_mytime(t_now, &t_struct, 0);

	
//	UART_PRINTF("get_key_state:%d \r\n",get_key_state());
	if(old_sec != tm_s.seconds)
	{
		old_sec = tm_s.seconds;
		if(get_feed_info_flag == 0 && set_time_flag == 0 && set_val_flag == 0)
		{
	
//			if(reverse) 
//			{
			
//				reverse = 0;
//			} 
//			else 
//			{

//				reverse = 1;
//			}			
			get_time();
			
//			test_feed_info();
		}
	}

		if(unlock_flag)
		{			
			lock_led();
			
			if(led_flag == 0)
			{
				SEG9[12]|=0X80;
			}
		}
		else
		{
			SEG9[12]&=~0X80;
		}
				
//		disp_voltage();
		
		display();


		
		if(old_min != tm_s.minutes)
		{
			old_min = tm_s.minutes;
			check_feed_flag = 1;
			save_time++;
			
			UART_PRINTF("%04d-%02d-%02d %02d:%02d:%02d \r\n",
				tm_s.year, tm_s.month, tm_s.day,
				tm_s.hour, tm_s.minutes, tm_s.seconds);

		}
		
		if(save_time == 1)
		{
//			rtc_timestamp = t_now;
			UART_PRINTF("save save_time 11\n");
			save_info.rtc_hour = tm_s.hour;
			save_info.rtc_minute = tm_s.minutes;
			
			UART_PRINTF("save time:%ld-%ld \n",save_info.rtc_hour, save_info.rtc_minute);

//			flash_erase(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, FLASH_SIZE_ONE);
			flash_erase_sector(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR);
			flash_write(FLASH_SPACE_TYPE_NVR, BLE_SAVE_ADDR, sizeof(SAVE_INFO_t), (uint8_t *)&save_info);

			save_time = 0;
		}
		
		if(lock_timeout > 0)
			lock_timeout--;					//??????????
//	}

		
//	int aaa = 0;
//	aaa = gpio_get_input(CHARGE_DET);
//	UART_PRINTF("DC_DET:%d\r\n", aaa);
//	aaa = gpio_get_input(SET_KEY);
//	UART_PRINTF("SET_KEY:%d\r\n", aaa);
//	feed_error_led();					//????????
//	seg_flash_task();
	
//	UART_PRINTF("@@@@@@@@@@@@@@@@@@@@@@@ UTC_TASK exit\r\n");
	ke_timer_set(UTC_TASK, TASK_APP, 10);
}

static int app_utc_handler(ke_msg_id_t const msgid,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	utc_callback();	
	return (KE_MSG_CONSUMED);
}

static int app_get_rec_handler(ke_msg_id_t const msgid,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	rec_key_callback();	
	return (KE_MSG_CONSUMED);
}

static int key_scan_handler(ke_msg_id_t const msgid,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	key_scan_callback();	
	return (KE_MSG_CONSUMED);
}

/* Default State handlers definition. */
const struct ke_msg_handler appm_default_state[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,    	(ke_msg_func_t)appm_msg_handler},
    {GAPM_DEVICE_READY_IND,     	(ke_msg_func_t)gapm_device_ready_ind_handler},
    {GAPM_CMP_EVT,             		(ke_msg_func_t)gapm_cmp_evt_handler},
    {GAPC_GET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_get_dev_info_req_ind_handler},
    {GAPC_SET_DEV_INFO_REQ_IND, 	(ke_msg_func_t)gapc_set_dev_info_req_ind_handler},
    {GAPC_CONNECTION_REQ_IND,   	(ke_msg_func_t)gapc_connection_req_ind_handler},
    {GAPC_CMP_EVT,             		(ke_msg_func_t)gapc_cmp_evt_handler},
    {GAPC_DISCONNECT_IND,       	(ke_msg_func_t)gapc_disconnect_ind_handler},
    {GAPM_PROFILE_ADDED_IND,    	(ke_msg_func_t)gapm_profile_added_ind_handler},
    {GAPC_LE_PKT_SIZE_IND,			(ke_msg_func_t)gapc_le_pkt_size_ind_handler},
    {GAPC_PARAM_UPDATED_IND,		(ke_msg_func_t)gapc_param_updated_ind_handler},
    {GATTC_MTU_CHANGED_IND,			(ke_msg_func_t)gattc_mtu_changed_ind_handler},	
    {GAPC_PARAM_UPDATE_REQ_IND, 	(ke_msg_func_t)gapc_param_update_req_ind_handler},
    {APP_PARAM_UPDATE_REQ_IND, 		(ke_msg_func_t)gapc_update_conn_param_req_ind_handler},
    {APP_PERIOD_TIMER,				(ke_msg_func_t)app_period_timer_handler},
    {APP_GATTC_EXC_MTU_CMD,		    (ke_msg_func_t)gattc_mtu_exchange_req_handler},
	{REC_KEY_TASK,		    		(ke_msg_func_t)app_get_rec_handler},
	{KEY_SCAN_TASK,		    		(ke_msg_func_t)key_scan_handler},
	{UTC_TASK,		    			(ke_msg_func_t)app_utc_handler},
};

/* Specifies the message handlers that are common to all states. */
const struct ke_state_handler appm_default_handler = KE_STATE_HANDLER(appm_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t appm_state[APP_IDX_MAX];

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
