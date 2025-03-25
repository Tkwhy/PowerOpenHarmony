/*
 * Copyright (c) 2021 GOODIX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "user_app.h"
#include "gr55xx_sys.h"
#include "app_log.h"
#include "app_error.h"
#include "gus.h"
#include "gus_c.h"
#include <string.h>

/*
 * DEFINES
 *****************************************************************************************
 */
/**@brief Gapm config data. */
//#define DEVICE_NAME                        "Goodix_Tem" /**< Device Name which will be set in GAP. */
#define DEVICE_NAME                        "ble_test" /**< Device Name which will be set in GAP. */
/**< The fast advertising min interval (in units of 0.625 ms). */
#define APP_ADV_FAST_MIN_INTERVAL          32
/**< The fast advertising max interval (in units of 0.625 ms). */
#define APP_ADV_FAST_MAX_INTERVAL          48
/**< The slow advertising min interval (in units of 0.625 ms). */
#define APP_ADV_SLOW_MIN_INTERVAL          160
/**< The slow advertising max interval (in units of 0.625 ms). */
#define APP_ADV_SLOW_MAX_INTERVAL          400

#define APP_ADV_TIMEOUT_IN_SECONDS         0            /**< The advertising timeout in units of seconds. */
#define MIN_CONN_INTERVAL                  320          /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                  520          /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                      0            /**< Slave latency. */
#define CONN_SUP_TIMEOUT                   400          /**< Connection supervisory timeout (4 seconds). */



#define APP_FIRST_CONN_INTERVAL_MIN          7                /**< Minimal connection interval(in units of 1.25 ms). */
#define APP_FIRST_CONN_INTERVAL_MAX          7                /**< Maximal connection interval(in units of 1.25 ms). */
#define APP_SECOND_CONN_INTERVAL_MIN         9                /**< Minimal connection interval(in units of 1.25 ms). */
#define APP_SECOND_CONN_INTERVAL_MAX         9                /**< Maximal connection interval(in units of 1.25 ms). */
#define APP_THIRD_CONN_INTERVAL_MIN          11               /**< Minimal connection interval(in units of 1.25 ms). */
#define APP_THIRD_CONN_INTERVAL_MAX          11               /**< Maximal connection interval(in units of 1.25 ms). */
#define RECONN_INTERVAL_MIN                  8                /**< Minimal connection interval(in units of 1.25 ms). */
#define RECONN_INTERVAL_MAX                  8                /**< Minimal connection interval(in units of 1.25 ms). */
#define APP_FIRST_UPDATE_CONN_INTERVAL_MIN   51               /**< Update minimal connection interval(in units of 1.25 ms). */
#define APP_FIRST_UPDATE_CONN_INTERVAL_MAX   51               /**< Update maximal connection interval(in units of 1.25 ms). */
#define APP_SECOND_UPDATE_CONN_INTERVAL_MIN  53               /**< Update minimal connection interval(in units of 1.25 ms). */
#define APP_SECOND_UPDATE_CONN_INTERVAL_MAX  53               /**< Update maximal connection interval(in units of 1.25 ms). */
#define APP_THIRD_UPDATE_CONN_INTERVAL_MIN   55               /**< Update minimal connection interval(in units of 1.25 ms). */
#define APP_THIRD_UPDATE_CONN_INTERVAL_MAX   55               /**< Update maximal connection interval(in units of 1.25 ms). */
#define APP_CONN_SLAVE_LATENCY               0                /**< Slave latency. */
#define APP_CONN_SUP_TIMEOUT                 50              /**< Connection supervisory timeout (in unit of 10 ms). */
#define MAX_NB_LECB_DEFUALT                  10               /**< Defualt length of maximal number of LE Credit based connection. */
#define MAX_TX_OCTET_DEFUALT                 251              /**< Default maximum transmitted number of payload octets. */
#define MAX_TX_TIME_DEFUALT                  2120             /**< Defualt maximum packet transmission time. */


uint8_t slave_connect_handle=0xff;
uint8_t master_connect_handle=0xff;
/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static gap_adv_param_t      s_gap_adv_param;            /**< Advertising parameters for legay advertising. */
static gap_adv_time_param_t s_gap_adv_time_param;       /**< Advertising time parameter. */

static const uint8_t s_adv_data_set[] = {               /**< Advertising data. */
    0x03,
    BLE_GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID,
    0x01,
    0x01,

    // Manufacturer specific adv data type
    0x05,
    BLE_GAP_AD_TYPE_MANU_SPECIFIC_DATA,
    // Goodix SIG Company Identifier: 0x04F7
    0xF7,
    0x04,
    // Goodix specific adv data
    0x02,0x03
};

static const uint8_t s_adv_rsp_data_set[] = {                /**< Scan responce data. */
    0x17,
    BLE_GAP_AD_TYPE_COMPLETE_NAME,
    'P', 'o', 'w', 'e', 'r', 'e', 'd', '_', 'b', 'y', '_','O', 'p', 'e', 'n', 'H', 'a', 'r', 'm', 'o', 'n', 'y'
};

static gap_init_param_t  gap_connect_param = 
{
	.type                = GAP_INIT_TYPE_DIRECT_CONN_EST,
    .interval_min        = APP_FIRST_CONN_INTERVAL_MIN,
    .interval_max        = APP_FIRST_CONN_INTERVAL_MAX,
    .slave_latency       = APP_CONN_SLAVE_LATENCY,
    .sup_timeout         = APP_CONN_SUP_TIMEOUT,
    .conn_timeout        = 0,
};
/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize gap parameters.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile)parameters
 *          of the device including the device name, appearance, and the preferred connection parameters.
 *****************************************************************************************
 */
static void gap_params_init(void)
{
    sdk_err_t   error_code;

    ble_gap_pair_enable(false);

    s_gap_adv_param.adv_intv_max  = APP_ADV_SLOW_MAX_INTERVAL;
    s_gap_adv_param.adv_intv_min  = APP_ADV_FAST_MIN_INTERVAL;
    s_gap_adv_param.adv_mode      = GAP_ADV_TYPE_ADV_IND;
    s_gap_adv_param.chnl_map      = GAP_ADV_CHANNEL_37_38_39;
    s_gap_adv_param.disc_mode     = GAP_DISC_MODE_GEN_DISCOVERABLE;
    s_gap_adv_param.filter_pol    = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;

 
    error_code = ble_gap_device_name_set(BLE_GAP_WRITE_PERM_NOAUTH, DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(error_code);

    error_code = ble_gap_adv_param_set(0, BLE_GAP_OWN_ADDR_STATIC, &s_gap_adv_param);
    APP_ERROR_CHECK(error_code);

    error_code = ble_gap_adv_data_set(0, BLE_GAP_ADV_DATA_TYPE_DATA, s_adv_data_set, sizeof(s_adv_data_set));
    APP_ERROR_CHECK(error_code);

    error_code = ble_gap_adv_data_set(0, BLE_GAP_ADV_DATA_TYPE_SCAN_RSP, \
                                      s_adv_rsp_data_set, sizeof(s_adv_rsp_data_set));
    //APP_LOG_INFO("%s, s_adv_rsp_data_set:%s, s_adv_rsp_data_set_len:%d", __func__, s_adv_rsp_data_set, sizeof(s_adv_rsp_data_set));
    APP_ERROR_CHECK(error_code);

    s_gap_adv_time_param.duration     = 0;
    s_gap_adv_time_param.max_adv_evt  = 0;
}


static void gap_scan_params_init(void)
{
    sdk_err_t        error_code;
    gap_scan_param_t scan_param;

    ble_gap_privacy_params_set(150, true);
    
    scan_param.scan_type     = GAP_SCAN_ACTIVE;
    scan_param.scan_mode     = GAP_SCAN_OBSERVER_MODE;
    scan_param.scan_dup_filt = GAP_SCAN_FILT_DUPLIC_EN;
    scan_param.use_whitelist = false;
	
    scan_param.interval      = 160;
    scan_param.window        = 80;
    scan_param.timeout       = 0;
    
	
    error_code = ble_gap_scan_param_set(BLE_GAP_OWN_ADDR_STATIC, &scan_param);
    APP_ERROR_CHECK(error_code);
}

static void gus_client_process_event(gus_c_evt_t *p_evt)
{
    sdk_err_t   error_code;
	uint8_t ucIndex = 0;

    switch (p_evt->evt_type)
    {
        case GUS_C_EVT_DISCOVERY_COMPLETE:
            APP_LOG_INFO("GUS_C_EVT_DISCOVERY_COMPLETE");        
            error_code = gus_c_tx_notify_set(p_evt->conn_idx, true);
            APP_ERROR_CHECK(error_code);
			ble_gap_scan_stop();
            break;

        case GUS_C_EVT_TX_NTF_SET_SUCCESS:
            APP_LOG_INFO("GUS_C_EVT_TX_NTF_SET_SUCCESS");   
            error_code = ble_gattc_mtu_exchange(p_evt->conn_idx);
            APP_ERROR_CHECK(error_code);
            //uint8_t test_data[]="hello_world/r/n";
            //error_code = gus_c_tx_data_send(slave_connect_handle, test_data, sizeof(test_data));
            //APP_LOG_INFO("gus_c_tx_data_send error_code = %d", error_code);  
            break;

        case GUS_C_EVT_PEER_DATA_RECEIVE:
            //主机接收数据
            APP_LOG_INFO("receive from slave:%s", p_evt->p_data);
            //APP_LOG_HEX_DUMP(p_evt->p_data, p_evt->length);            
            break;

        default:
            break;
    }
}



static void gus_service_process_event(gus_evt_t *p_evt)
{
	uint8_t ucIndex = 0;
    sdk_err_t   error_code;
	
    switch (p_evt->evt_type)
    {
        case GUS_EVT_TX_PORT_OPENED:
            break;

        case GUS_EVT_TX_PORT_CLOSED:
            break;

        case GUS_EVT_RX_DATA_RECEIVED:            //从机接收数据
            APP_LOG_INFO("receive from master:%s", p_evt->p_data);
            //APP_LOG_HEX_DUMP(p_evt->p_data, p_evt->length);
            //error_code = ble_gattc_mtu_exchange(p_evt->conn_idx);
            //APP_ERROR_CHECK(error_code);
            uart_data_send(p_evt->p_data, p_evt->length);
            //uint8_t test_data[]="ack hello_world/r/n";;
            //uint32_t error =  gus_tx_data_send(master_connect_handle, test_data, sizeof(test_data));
            //APP_LOG_INFO("gus_tx_data_send eror = %d", error);
            break;
		
		case GUS_EVT_TX_DATA_SENT:
            break;

        default:
            break;
    }
}
/**
 *****************************************************************************************
 * @brief Initialize services that will be used by the application.
 *****************************************************************************************
 */
static void services_init(gus_evt_handler_t evt_handler)
{
    sdk_err_t   error_code;
    gus_init_t gus_init;

    gus_init.evt_handler = evt_handler;

    error_code = gus_service_init(&gus_init);
    APP_ERROR_CHECK(error_code);
} 

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int BleConnectFlag = -1;
void app_disconnected_handler(uint8_t conn_idx, uint8_t reason)
{
    sdk_err_t error_code;
    if(conn_idx == slave_connect_handle)
    {
        slave_connect_handle = 0xff;
    }else if(conn_idx == master_connect_handle)
    {
        master_connect_handle = 0xff;
    }
    BleConnectFlag = -1;
    error_code = ble_gap_adv_start(0, &s_gap_adv_time_param);
    APP_ERROR_CHECK(error_code);
    APP_LOG_INFO("ble_gap_adv_start error=%d", error_code);     
}

void app_connected_handler(uint8_t conn_idx, const gap_conn_cmp_t *p_param)
{
    sdk_err_t error_code;
    error_code = ble_gattc_mtu_exchange(conn_idx);
    APP_ERROR_CHECK(error_code);
    if(p_param->ll_role == GAP_LL_ROLE_MASTER)
    {
        slave_connect_handle = conn_idx;

        // start discovery of services. The NUS Client waits for a discovery result
        error_code = gus_c_disc_srvc_start(conn_idx);
        APP_ERROR_CHECK(error_code);    
        APP_LOG_INFO("gus_c_disc_srvc_start error=%d", error_code);         
    }else
    {
        master_connect_handle = conn_idx;
        BleConnectFlag = 1;
    }
    
}
uint8_t app_master_connect_get()
{
    return master_connect_handle;
}

uint8_t app_master_connect_flag_get()
{
    return BleConnectFlag;
}
uint8_t adv_rsp_data_set[28] = {0};
void app_get_adv_name(uint8_t *adv_name, uint16_t adv_name_len)
{

    adv_rsp_data_set[0] = adv_name_len;
    adv_rsp_data_set[1] = BLE_GAP_AD_TYPE_COMPLETE_NAME;
    for(int i =2 ;i <= adv_name_len; i++)
    {
        adv_rsp_data_set[i]= adv_name[i-2];
    }

}
uint8_t *GetAdvName()
{
    if(adv_rsp_data_set[0] >= 2 && adv_rsp_data_set[1] == BLE_GAP_AD_TYPE_COMPLETE_NAME)
    {
        return &adv_rsp_data_set;
    }
    else
    {
        return NULL;
    }
}

static uint8_t BLE_IsTargetAddr(uint8_t *pAddr)
{
	//for(uint8_t ucIndex = 0; ucIndex < BLE_LINK_CENTRAL_CONUNT; ucIndex++)
    uint8_t target_addr[6]={0xf6, 0xcc, 0x34, 0x11, 0x5a, 0xe4};//需要连接的从机，MAC地址是手机看到的倒叙
	{
		if(0 == memcmp(target_addr, pAddr, 6))
		{
			return true;
		}
	}

	return false;
}

void app_adv_report_handler(const gap_ext_adv_report_ind_t  *p_adv_report)
{
    sdk_err_t   error_code;
	
	//if(p_adv_report->rssi > -80) //信号强度限制
	{
       
		if(true == BLE_IsTargetAddr((p_adv_report->broadcaster_addr.gap_addr.addr)))
		{
            APP_LOG_INFO("rssi:%d",p_adv_report->rssi);  
			APP_LOG_INFO("find slave mac: %02X:%02X:%02X:%02X:%02X:%02X.",
                        p_adv_report->broadcaster_addr.gap_addr.addr[5],
                        p_adv_report->broadcaster_addr.gap_addr.addr[4],
                        p_adv_report->broadcaster_addr.gap_addr.addr[3],
                        p_adv_report->broadcaster_addr.gap_addr.addr[2],
                        p_adv_report->broadcaster_addr.gap_addr.addr[1],
                        p_adv_report->broadcaster_addr.gap_addr.addr[0]);
			
			 gap_connect_param.peer_addr.addr_type = p_adv_report->broadcaster_addr.addr_type;
			 memcpy(gap_connect_param.peer_addr.gap_addr.addr,p_adv_report->broadcaster_addr.gap_addr.addr,6);
				
			error_code = ble_gap_connect(BLE_GAP_OWN_ADDR_STATIC, &gap_connect_param);
			//ble_gap_scan_stop();
		}
	}
}

void ble_init_cmp_callback(void)
{
    sdk_err_t     error_code;
    gap_bdaddr_t  bd_addr;
    sdk_version_t version;

    sys_sdk_verison_get(&version);
    APP_LOG_INFO("Goodix GR551x SDK V%d.%d.%02d (commit %d)",
                 version.major, version.minor, version.build, version.commit_id);

    error_code = ble_gap_addr_get(&bd_addr);
    APP_ERROR_CHECK(error_code);
    APP_LOG_INFO("Local Board %02X:%02X:%02X:%02X:%02X:%02X.",
                 bd_addr.gap_addr.addr[5],      /* gap_addr byte5 */
                 bd_addr.gap_addr.addr[4],      /* gap_addr byte4 */
                 bd_addr.gap_addr.addr[3],      /* gap_addr byte3 */
                 bd_addr.gap_addr.addr[2],      /* gap_addr byte2 */
                 bd_addr.gap_addr.addr[1],      /* gap_addr byte1 */
                 bd_addr.gap_addr.addr[0]);     /* gap_addr byte0 */
    APP_LOG_INFO("Template application example started.");

    services_init(gus_service_process_event);
  	//gus_client_init(gus_client_process_event);  
    
    gap_params_init();
    //gap_scan_params_init();
    
    error_code = ble_gap_adv_start(0, &s_gap_adv_time_param);
    APP_ERROR_CHECK(error_code);

    //error_code = ble_gap_scan_start();
    // APP_ERROR_CHECK(error_code);   
}
