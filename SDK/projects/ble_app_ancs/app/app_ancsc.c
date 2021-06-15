/**
 ****************************************************************************************
 *
 * @file app_ancs.c
 *
 * @brief Ancsc Application Module entry point
 *
 * Copyright (C) BeKen 2016-2017
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_ancsc.h"              // Battery Application Module Definitions
#include "co_utils.h"        		// common utility definition
#include "app.h"                    // Application Definitions
#include "app_task.h"               // application task definitions
#include "ancsc_task.h"             // health thermometer functions
#include "prf_types.h"              // Profile common types definition
#include "uart.h"
/*
 * DEFINES
 ****************************************************************************************
 */


static void app_ancsc_notification_ntf_unpack(struct notificationCharInd  *param);

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// ANCS Application Module Environment Structure
struct app_ancsc_env_tag app_ancsc_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_ancsc_init(void)
{
	UART_PRINTF("%s \r\n",__func__);
    // Reset the environment
    memset(&app_ancsc_env, 0, sizeof(struct app_ancsc_env_tag)); 
}

void app_ancs_add_ancsc(void)
{
	UART_PRINTF("%s \r\n",__func__);
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, 0);
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH, ENABLE);
    req->prf_task_id = TASK_ID_ANCSC;
    req->app_task = TASK_APP;
    req->start_hdl = 0;
    // Send the message
    ke_msg_send(req); 
}

void app_ancsc_enable_prf(uint16_t conhdl)
{
	
	UART_PRINTF("%s \r\n",__func__);
    app_ancsc_env.conhdl = conhdl;

    // Allocate the message
    struct ancsc_enable_req * req = KE_MSG_ALLOC(ANCSC_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_ANCSC), TASK_APP,
                                                ancsc_enable_req);

    // Fill in the parameter structure
    req->conhdl      = conhdl;
    req->con_type    = PRF_CON_DISCOVERY;
   

    // Send the message
    ke_msg_send(req);
}



static int app_ancsc_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    // Drop the message
	UART_PRINTF("%s \r\n",__func__);
    return (KE_MSG_CONSUMED);
}


static int app_ancsc_enable_rsp_handler(ke_msg_id_t const msgid,
                                     struct ancsc_enable_rsp  *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("%s \r\n",__func__);

	struct ancsc_ntf_src_ntf_cfg_req * ntf_req = KE_MSG_ALLOC(ANCSC_CFG_NTF_SRC_INDNTF_REQ,
                                            prf_get_task_from_id(TASK_ID_ANCSC), TASK_APP,
                                            ancsc_ntf_src_ntf_cfg_req);
	

	ntf_req->ntf_cfg  = PRF_CLI_START_NTF;

	ke_msg_send(ntf_req);

	return (KE_MSG_CONSUMED);
}

static int app_ancsc_enable_ntf_rsp_handler(ke_msg_id_t const msgid,
                                     struct ancsc_ntf_src_ntf_cfg_rsp  *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
																			 
{
	UART_PRINTF("%s ,status = 0x%x\r\n",__func__,param->status);
	
	struct ancsc_data_src_ntf_cfg_req * data_req = KE_MSG_ALLOC(ANCSC_CFG_DATA_SRC_INDNTF_REQ,
                                            prf_get_task_from_id(TASK_ID_ANCSC), TASK_APP,
                                            ancsc_data_src_ntf_cfg_req);
	
	data_req->ntf_cfg  = PRF_CLI_START_NTF;
	ke_msg_send(data_req);
	
	return (KE_MSG_CONSUMED);
}

static int app_ancsc_enable_data_ntf_rsp_handler(ke_msg_id_t const msgid,
                                     struct ancsc_ntf_src_ntf_cfg_rsp  *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
																			 
{
	UART_PRINTF("%s ,status = 0x%x\r\n",__func__,param->status);
	return (KE_MSG_CONSUMED);
}



static int app_ancsc_notification_nft_handler(ke_msg_id_t const msgid,
                                     struct notificationCharInd  *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
																			 
{
//	UART_PRINTF("%s \r\n\r\n",__func__);
	
//	uint16_t length = param->length;;
//	uint8_t EventID = param->ntf.EventID;
//	uint8_t EventFlags = param->ntf.EventFlags;
//	uint8_t CategoryID = param->ntf.CategoryID;
//	uint8_t CategoryCount = param->ntf.CategoryCount;
//	uint32_t NotificationUID = param->ntf.NotificationUID;	
//	UART_PRINTF("length = 0x%x,EventID = 0x%x,EventFlags = 0x%x\r\n,CategoryID = 0x%x,CategoryCount = 0x%x,NotificationUID = 0x%08x\r\n\r\n",length,EventID,EventFlags,CategoryID,CategoryCount,NotificationUID);
	app_ancsc_notification_ntf_unpack(param);
	
		
	return (KE_MSG_CONSUMED);
}


static int app_ancsc_data_ntf_handler(ke_msg_id_t const msgid,
                                     struct dataCharInd  *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    	
	UART_PRINTF("%s \r\n",__func__);
			
//	static uint8_t continue_flag = 0;
//	static uint8_t data_index = 0;
//	static struct AttributeList list;
    //uint8_t list_buff[256];

    for(uint8_t i=0;i<param->length;i++)
    {
        UART_PRINTF("%c", param->value[i]);
    }
    UART_PRINTF("\r\n");
#if 0
    if ( *(param->value ) == 'b')	//  bili
    {     
        uart_printf("bili\r\n");
    }
    else if ((*(param->value ) == 'm') && (*(param->value+1 ) == 'q'))  // mqq 
    {     
        uart_printf("mqq\r\n");
    }
    else if ((*(param->value ) == 'x')  && (*(param->value+1 ) == 'i'))  // xin 
    {
        uart_printf("xin\r\n");
    }


    list.data = (uint8_t*)list_buff[0];
	if(continue_flag == 0)
	{
		UART_PRINTF("first !!!!!!\r\n");
		list.AttributeID = co_read8(param->value + 5);
		list.Attrlen = co_read16p(param->value + 6);
			
		
		UART_PRINTF("AttributeID = 0x%x,Attrlen = 0x%x\r\n",list.AttributeID,list.Attrlen);
			
		if((list.Attrlen + 8 ) > param->length)
		{
			for(int i = 0;i < 12;i++)
			{
				list.data[data_index++] = *(param->value + 8 + i);
			}
			list.Attrlen-= 12;
			continue_flag = 1;
	 	}
		else
		{
			for(int i = 0;i < list.Attrlen;i++)
			{
				list.data[data_index++] = *(param->value + 8 + i);
			}
			
			UART_PRINTF("Comingcall num0 is ");
			for(int j = 0;j< data_index;j++)
			{
				UART_PRINTF("%c ",list.data[j]);
			}
			UART_PRINTF("\r\n");
			
			list.data = 0;
			data_index = 0;
		}
		
	}
	else if(continue_flag == 1)
	{
		UART_PRINTF("second !!!!!!!\r\n");
			
		if(list.Attrlen > param->length)
		{
			UART_PRINTF("second 1111!!!!!!!\r\n");
			for(int i = 0;i < param->length;i++)
			{
				list.data[data_index++] = *(param->value + i);
				UART_PRINTF("%x ",list.data[data_index-1]);
			}
			list.Attrlen-=param->length;
		}
		else if(list.Attrlen == param->length)
		{
			UART_PRINTF("second 2222!!!!!!!\r\n");
			for(int i = 0;i <list.Attrlen;i++)
			{
			 list.data[data_index++] = *(param->value + i);
			 UART_PRINTF("%x ",list.data[data_index -1]);
			}
			list.Attrlen = 0;
			continue_flag = 0;
			UART_PRINTF("Comingcall num is ");
	 		for(int j = 0;j< data_index;j++)
			{
	 			UART_PRINTF("%c ",list.data[j]);
			}
	 		UART_PRINTF("\r\n");
			
			list.data = 0;
			data_index = 0;
		}else
		{
			for(int i = 0;i <list.Attrlen;i++)
			{
			 	list.data[data_index++] = *(param->value + i);
			}
		}
	}
    #endif	
	return (KE_MSG_CONSUMED);
}


/*
 * LOCAL FUNC DEFINITIONS
 ****************************************************************************************
 */



static void app_ancsc_get_incomingcall_number(uint32_t NotificationUID)
{
	UART_PRINTF("%s \r\n\r\n",__func__);
		
	struct Get_Notification_Attribute_Command * req = KE_MSG_ALLOC_DYN(ANCSC_WR_CNTL_POINT_REQ,
	                                    prf_get_task_from_id(TASK_ID_ANCSC), TASK_APP,
	                                    Get_Notification_Attribute_Command,0x08);

	uint8_t *buf = (uint8_t *)&(req->cmd[0]);


	/*-----------------------------------------------------------------------------------//
	-----------
	Byte	  |	0	|	1	|	2	|	3	|	4	|	5	|	6	|	7	|	
	-----------
	param	|CommandID|   	NotificationUID		|AttributeID|   Maxlen	 |
	------------------------------------------------------------------------------------*/

	co_write8(buf, CommandIDGetNotificationAttributes);
	co_write32p(buf + 1, NotificationUID);
	co_write8(buf + 5, NotificationAttributeIDTitle);
	co_write16p(buf + 6, 50);
	req->length = 0x08;
		
	ke_msg_send(req);

}


static void app_ancsc_IncomingCall_unpack(uint8_t EventID,uint8_t EventFlags,uint8_t CategoryCount,uint32_t NotificationUID)
{
	
	switch (EventID)
	{
		case EventIDNotificationAdded:// 0
			UART_PRINTF("add one new incoming call\r\n");
			app_ancsc_get_incomingcall_number(NotificationUID);
			break;
		
		case EventIDNotificationModified:// 0
			UART_PRINTF("Modified one new incoming call\r\n");
			break;
		
		case EventIDNotificationRemoved:// 0
			UART_PRINTF("Removed one new incoming call\r\n");
			break;
		
		default:
			UART_PRINTF("Reserved_EventID_values\r\n");
			break;
	}
}


static void app_ancsc_get_Socialmsg(uint32_t NotificationUID)
{
    uint8_t qq_get_info[20] = { 0x00,0x04,0x00,0x00,0x00,0x00, 
                                0x01,0x78,0x00,0x02,0x78,0x00, 
                                0x03,0x78,0x00,0x04,0x05,0x06,
                                0x07								
							   };
    
   	
    UART_PRINTF("%s \r\n\r\n",__func__);
    struct Get_Notification_Attribute_Command * req = KE_MSG_ALLOC_DYN(ANCSC_WR_CNTL_POINT_REQ,
                                                                       prf_get_task_from_id(TASK_ID_ANCSC), 
                                                                       TASK_APP,
                                                                       Get_Notification_Attribute_Command,20);
    uint8_t *buf = (uint8_t *)&(req->cmd[0]);//req->cmd[0]
    /*************The format of a Get Notification Attribute command*******************/
    /*-----------------------------------------------------------------------------------//
    Byte      |     0           |       1       |       2       |       3       |       4       |       5               |       6       |       7       |
    param       |CommandID|          NotificationUID                |AttributeID|  Maxlen               |
    ------------------------------------------------------------------------------------*/
    co_write8(buf, CommandIDGetNotificationAttributes);
    co_write32p(buf + 1, NotificationUID);
    // NotificationAttributeIDTitle, NotificationAttributeIDAppIdentifier
    //co_write8(buf + 5, NotificationAttributeIDAppIdentifier);  // 
    //co_write16p(buf + 6, 50);
    //co_write8(buf + 2, 0);
    //co_write8(buf + 3, 0);
    //co_write8(buf + 4, 0);
    co_write8(buf + 5, 0);
    
	for(uint8_t i = 0;i < 13;i++)
    {
    	co_write8(buf + 6+ i,qq_get_info[6+i]);
    }
	
  	req->length = 19;
    ke_msg_send(req);
}


static void app_ancsc_Socialmsg_unpack(uint8_t EventID,uint8_t EventFlags,
        uint8_t CategoryCount,uint32_t NotificationUID)
{
  // uart_printf("\r\n EvID=%u\r\n",EventID);
    switch (EventID)
    {
	    case EventIDNotificationAdded:// 0
	        //uart_printf("add one new incoming call\r\n");
	        app_ancsc_get_Socialmsg(NotificationUID);
	        break;
	    case EventIDNotificationModified:// 0
	        //uart_printf("Modified one new incoming call\r\n");
	        break;
	    case EventIDNotificationRemoved:// 0
	        //  //uart_printf("Removed one new incoming call\r\n");
	        break;
	    default:
	        //uart_printf("Reserved_EventID_values\r\n");
	        break;
    }
}


static void app_ancsc_notification_ntf_unpack(struct notificationCharInd  *param)
{
	switch(param->ntf.CategoryID)
	{
		case CategoryIDOther: // 0
			UART_PRINTF("CategoryIDOther \r\n");
		break;
		
		case CategoryIDIncomingCall: // 1
			UART_PRINTF("CategoryIDIncomingCall \r\n");
			app_ancsc_IncomingCall_unpack(param->ntf.EventID,param->ntf.EventFlags,param->ntf.CategoryCount,param->ntf.NotificationUID);
		break;
		
		case CategoryIDMissedCall: // 2
			UART_PRINTF("CategoryIDMissedCall \r\n");
		break;
		
		case CategoryIDVoicemail: // 3
			UART_PRINTF("CategoryIDVoicemail \r\n");
		break;
		
		case CategoryIDSocial:  // 4
			UART_PRINTF("CategoryIDSocial \r\n");
			app_ancsc_Socialmsg_unpack(param->ntf.EventID,param->ntf.EventFlags,param->ntf.CategoryCount,param->ntf.NotificationUID);
		break;
		
		case CategoryIDSchedule: // 5
			UART_PRINTF("CategoryIDSchedule \r\n");
		break;
		
		case CategoryIDEmail: // 6
			UART_PRINTF("CategoryIDEmail \r\n");
		break;
		
		case CategoryIDNews: // 7
			UART_PRINTF("CategoryIDNews \r\n");
		break;

		case CategoryIDHealthAndFitness: // 8
			UART_PRINTF("CategoryIDHealthAndFitness \r\n");
		break;
		
		case CategoryIDBusinessAndFinance: // 9
			UART_PRINTF("CategoryIDBusinessAndFinance \r\n");
		break;

		case CategoryIDLocation: // 10
			UART_PRINTF("CategoryIDLocation \r\n");
		break;
		
		case CategoryIDEntertainment: // 11
			UART_PRINTF("CategoryIDEntertainment \r\n");
		break;
		
		default:
			UART_PRINTF("Reserved CategoryID\r\n");
			break;
	}
}
		


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_ancsc_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
	{KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_ancsc_msg_dflt_handler},

	{ANCSC_ENABLE_RSP,              (ke_msg_func_t)app_ancsc_enable_rsp_handler},

	{ANCSC_CFG_NTF_SRC_INDNTF_RSP,  (ke_msg_func_t)app_ancsc_enable_ntf_rsp_handler},

	{ANCSC_CFG_DATA_SRC_INDNTF_RSP, (ke_msg_func_t)app_ancsc_enable_data_ntf_rsp_handler},

	{ANCSC_NOTIFICATION_IND,		(ke_msg_func_t)app_ancsc_notification_nft_handler},

	{ANCSC_DATA_SOURCE_IND,			(ke_msg_func_t)app_ancsc_data_ntf_handler},
};

const struct ke_state_handler app_ancsc_table_handler =
    {&app_ancsc_msg_handler_list[0], (sizeof(app_ancsc_msg_handler_list)/sizeof(struct ke_msg_handler))};



/// @} APP
