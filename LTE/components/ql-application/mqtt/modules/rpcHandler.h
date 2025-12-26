
#ifndef RPC_HANDLER_H
#define RPC_HANDLER_H


#include "ilmCommon.h"
typedef uint32_t clock_time_t;

typedef enum
{
    rpc_packet_control,
    rpc_packet_config,
    rpc_packet_get,
    rpc_packet_default_settings,
    rpc_packet_get_payload,
    rpc_packet_ota,
    rpc_packet_ping,
    rpc_packet_end
} rpc_packet_type_t;


typedef enum{
	config_otherSend=1,
	config_boardSend,
	config_slotSend,
	config_servSend,
	config_thresholdSend,
	config_allSend,
	config_versionSend,
	config_gpsSend,
    config_sendLivePacket,
    config_sendGpsPacket,
    config_sendiccidPacket,
    config_apnSend,
	config_SendNothing
}sendConfig;

/*typedef enum {
    photocell, slot_time, manual, testing
} lamp_control_t;*/


typedef enum
{
    mqtt_get_live,
    mqtt_get_settings_telemetry,
    mqtt_threshold_settings_telemetry,
    mqtt_get_settings_attributes,
    mqtt_threshold_settings_attributes,
    mqtt_get_version,
    mqtt_gps_send,
} mqtt_get_type_t;


typedef enum
{
    clear_param_all,
    clear_burn_data,
    clear_payload_counter,
    clear_payload_database,
    clear_fault_data,
    clear_flash,
    reboot,
} clear_param_t;






extern  pubHandler_t pub_handler( uint8_t *chunk,uint16_t chunk_len);



#endif