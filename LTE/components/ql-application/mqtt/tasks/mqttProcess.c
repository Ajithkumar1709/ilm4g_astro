#include "ilmCommon.h"
#include "ql_mqttclient.h"
#include "memoryHandler.h"
#include "ql_gpio.h"
#include "ql_api_nw.h"


//DEFINES
#define IMEI 20
typedef enum {
  nwkRegister,
  dataCall,
  mqttClientInit,
  mqttConnect,
  mqttExit
}mqttState_e;

//GLOBALS AND STATICS
int  mqtt_connected=0;
static int  mqtt_subscribe=0;
S1 operator_name[15]={'\0'};
U1 apn_set;
int signal;

char imei_no[IMEI]={'\0'};
static ql_sem_t  mqtt_semp;
struct mqtt_connect_client_info_t client_info = {
  0
};

S1 URL[60];
mqtt_client_t  mqtt_cli;
ql_nw_signal_strength_info_s ss_info;

ilmQueue_t ilmData;
//EXTERNS
extern ql_queue_t ql_ilmProcess_qHandle;
extern ql_mutex_t dataFileMutex ;
extern ilm_t ilm;
extern ql_queue_t ql_mqttSend_queueHandle;
extern U1 maxPayLoadChecker(rtc_t rtc);
extern ql_task_t mqtt_task;


static void mqtt_state_exception_cb(mqtt_client_t *client)
{
	QL_MQTT_LOG("-1-NWKLOG:mqtt session abnormal disconnect");
	mqtt_connected = 0;
}

static void mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
{
	QL_MQTT_LOG("-1-NWKLOG:mqtt_connect_result_cb status: %d", status);
	if(status == 0){
		mqtt_connected = 1;
	}
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_requst_subUnsub_result_cb(mqtt_client_t *client, void *arg,int err)
{
 // mqttFsStateUpdate_t *mqttInfo=arg;
	QL_MQTT_LOG("-1-NWKLOG:Mqttt Subscribe  Status err: %d   ", err);
	if(err==0){
    mqtt_subscribe=1;
  }
  ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_requst_result_cb(mqtt_client_t *client, void *arg,int err)
{
  mqttFsStateUpdate_t *mqttInfo=arg;
	QL_MQTT_LOG("-1-NWKLOG:Mqttt Publish Status err: %d , Fs Required %d , Fs Index= %d  ", err,mqttInfo->fsUpdateRequired,mqttInfo->index);
	if(err==0){
    if(mqttInfo->fsUpdateRequired==1){
      //fsDataUpdateUploadState(mqttInfo->index);
      ql_rtos_mutex_lock((ql_mutex_t)dataFileMutex, QL_WAIT_FOREVER);
      fsDataUpdateUploadState(mqttInfo->index);    	
      ql_rtos_mutex_unlock((ql_mutex_t)dataFileMutex);
    }
  }
  ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_inpub_data_cb(mqtt_client_t * client, void * arg, int pkt_id,
  const char * topic,
    const unsigned char * payload, unsigned short payload_len) {
  QL_MQTT_LOG("-1-NWKLOG:mqtt_inpub_data_cb topic: %s ,payload: %s", topic, payload);
  memset(&ilmData, 0, sizeof(ilmData));
  memcpy(ilmData.data, payload, payload_len);
  ilmData.pktType = D_RPC;
  ilmData.dataSize = payload_len;
  ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
}
int check_operator(uint8_t nSim,int profile_idx){
  int ret=0;
  U1 i=0;
	ql_nw_operator_info_s *oper_info = (ql_nw_operator_info_s *)calloc(1, sizeof(ql_nw_operator_info_s));
	ql_nw_get_operator_name(nSim, oper_info);
	QL_MQTT_LOG("long_oper_name:%s, short_oper_name:%s\r\n",oper_info->long_oper_name, oper_info->short_oper_name);
	stpcpy(operator_name,oper_info->long_oper_name);
  for (i=0; i < strlen(operator_name);i++) 
    operator_name[i] = toupper((int)operator_name[i]);
  QL_MQTT_LOG("THE OPERATOR NAME IS=%s\r\n",operator_name);
	if(strstr(operator_name,"AIRTEL")){
	 QL_MQTT_LOG("apn set as airtel\r\n");
     ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "airtelgprs.com", NULL, NULL, 0);
	}
	else if(strstr(operator_name,"CELLONE")){
	 QL_MQTT_LOG("apn set as bsnl\r\n");
	 ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "bsnlnet", NULL, NULL, 0); 
	}
	else if(strstr(operator_name,"VI")){
	  QL_MQTT_LOG("apn set as vodafone\r\n");
	  ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "www", NULL, NULL, 0);
	}
	else if(strstr(operator_name,"JIO")){
	  QL_MQTT_LOG("apn set as jio\r\n");
	  ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "jionet", NULL, NULL, 0);
	}
  else{
     QL_MQTT_LOG("apn set as RPC\r\n");
	  ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, ilm.ilmConfig.apn, NULL, NULL, 0);
	}

  
  QL_MQTT_LOG("-1-NWKLOG:DATA CALL RESULT:%d", ret);
  if (ret != 0) {
     QL_MQTT_LOG("-1-NWKLOG:DATA CALL FAILURE!");
  }
 return ret;
	
}
void mqtt_init() {

  client_info.keep_alive = 60;
  client_info.clean_session = 1;
  client_info.will_qos = 0;
  client_info.will_retain = 0;
  client_info.will_topic = NULL;
  client_info.will_msg = NULL;
  client_info.client_id = imei_no;
  client_info.client_user = imei_no;
  client_info.client_pass = imei_no;
  client_info.pkt_timeout = 5;
  client_info.retry_times = 3;
}

U1 mqttConnectProcess(void) {
  U1 state = nwkRegister;
  int nwkRegRetryCount = 0;
  uint16_t sim_cid;
  int ret = 0;
  uint8_t nSim = 0;
  int profile_idx = 1, status;

  while (state != mqttExit) {
    switch (state) {
    case nwkRegister:
      while ((ret = ql_network_register_wait(nSim, 10)) != 0 && nwkRegRetryCount < 5) {
        nwkRegRetryCount++;
        ql_rtos_task_sleep_s(1);
      }
      ql_nw_get_signal_strength(0, &ss_info);
      signal=ss_info.rssi;
      if (ret == 0) {
        nwkRegRetryCount = 0;
        QL_MQTT_LOG("-1-NWKLOG:NETWORK REGISTERED!");
        state = dataCall;
      } else {
        QL_MQTT_LOG("-1-NWKLOG:NETWORK NOT REGISTERED!");
        state = mqttExit;
      }
      break;
    case dataCall:
      ql_set_data_call_asyn_mode(nSim, profile_idx, 0);
      QL_MQTT_LOG("-1-NWKLOG:START DATA CALL");
    //  ret = ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "www", NULL, NULL, 0);
      ret=check_operator(nSim,profile_idx);
      QL_MQTT_LOG("-1-NWKLOG:DATA CALL RESULT:%d", ret);
      if (ret != 0) {
        QL_MQTT_LOG("-1-NWKLOG:DATA CALL FAILURE!");
        state = mqttExit;
      } else {
        if (QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(nSim, profile_idx, & sim_cid)) {
          QL_MQTT_LOG("-1-NWKLOG:nSim or profile_idx is invalid!!!!");
          state = mqttExit;
        } else {
          state = mqttClientInit;
          QL_MQTT_LOG("-1-NWKLOG:nSim or profile_idx is valid");
        }
      }
      break;
    case mqttClientInit:
      ql_mqtt_client_deinit( & mqtt_cli);
      status = ql_mqtt_client_init( & mqtt_cli, sim_cid);
      if (status != MQTTCLIENT_SUCCESS) {
        QL_MQTT_LOG("-1-NWKLOG:mqtt client init failed %d!!!!", status);

        state = mqttExit;
      } else {
        QL_MQTT_LOG("-1-NWKLOG:mqtt client init success!!!!");
        state = mqttConnect;
      }
      QL_MQTT_LOG("-1-NWKLOG:mqtt_cli:%d", mqtt_cli);
      break;
    case mqttConnect:
      mqtt_connected = 0;
      sprintf(URL, "mqtt://%s:%d", ilm.ilmConfig.server.host, ilm.ilmConfig.server.port);
      ret = ql_mqtt_connect( & mqtt_cli, URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t * ) & client_info, mqtt_state_exception_cb);
      if (ret == MQTTCLIENT_WOUNDBLOCK) {
        QL_MQTT_LOG("-1-NWKLOG:mqtt wait connect result");
        status=ql_rtos_semaphore_wait(mqtt_semp, 120000);
        if(status!=QL_OSI_SUCCESS){
            QL_MQTT_LOG("-1-NWKLOG:mqtt connect timeout");
          mqtt_connected =0;
          }
        QL_MQTT_LOG("-1-NWKLOG:Mqtt connect wait exit ok ");

        if (mqtt_connected == 1) {
          QL_MQTT_LOG("-1-NWKLOG:MQTT CONNECT OK");
          mqtt_subscribe = 0;
          ql_mqtt_set_inpub_callback( & mqtt_cli, mqtt_inpub_data_cb, NULL);
          if (ql_mqtt_sub_unsub( & mqtt_cli, "v2/r/req/+", 1, mqtt_requst_subUnsub_result_cb, NULL, 1) == MQTTCLIENT_WOUNDBLOCK) {
            QL_MQTT_LOG("-1-NWKLOG:mqtt wait subscrible result");
            status=ql_rtos_semaphore_wait(mqtt_semp, 120000);
            if(status!=QL_OSI_SUCCESS){
              QL_MQTT_LOG("-1-NWKLOG:mqtt subscribe timeout");
              mqtt_connected =0;
            }
            QL_MQTT_LOG("-1-NWKLOG:Mqtt subscribe wait exit ok ");
            if (mqtt_subscribe == 1) {
              QL_MQTT_LOG("-1-NWKLOG:MQTT SUBSCRIBE SUCCESS");
              ql_gpio_set_level(0, LVL_LOW);
              return 1;
            } else {
              QL_MQTT_LOG("-1-NWKLOG:MQTT SUBSCRIBE FAIL");
              state = mqttExit;
            }
          }
        } else {
          QL_MQTT_LOG("-1-NWKLOG:MQTT CONNECT FAIL");
          state = mqttExit;
        }
      }
      break;
    }
  }
  return 0;
}

void airplane_mode() {
  int err;
  err = ql_dev_set_modem_fun(0, 0, 0);
  QL_MQTT_LOG("-1-NWKLOG:airplan mode Enter=%d", err);
  ql_rtos_task_sleep_s(2);
  err = ql_dev_set_modem_fun(1, 0, 0);
  QL_MQTT_LOG("-1-NWKLOG:airplan mode Exit=%d", err);

}

nwk_t networkCheckRecovery(void) {
  nwk_t nwk;
  int rty_count;
  if (mqtt_connected == 0) {
     ql_gpio_set_level(0, LVL_HIGH);
    for (rty_count = 0; rty_count < 1; rty_count++) {
      airplane_mode();
      mqtt_init();
      if (mqttConnectProcess()) {
        nwk.nwkStatus = 1;
        nwk.mqttStatus = mqtt_connected;
        return nwk;
      } else {
        nwk.nwkStatus = 0;
        nwk.mqttStatus = mqtt_connected;
        return nwk;
      }
    }
  } else {
    nwk.nwkStatus = 1;
    nwk.mqttStatus = mqtt_connected;
  }
  return nwk;
}

void mqtt_publish(U1 * data, unsigned short dataSize, char topic, void * arg) {
  QlOSStatus status;
  if (topic == 1) {
    ql_gpio_set_level(0, LVL_HIGH);
    QL_MQTT_LOG("-1-NWKLOG:Mqtt  publish data length %d", dataSize);
    if ((ql_mqtt_publish( & mqtt_cli, "v1/devices/me/telemetry", data, dataSize, 1, 0, mqtt_requst_result_cb, arg) == MQTTCLIENT_WOUNDBLOCK)) {
      QL_MQTT_LOG("-1-NWKLOG:Mqtt wait enter publish topic 1 result");
      status=ql_rtos_semaphore_wait(mqtt_semp,120000);
      if(status!=QL_OSI_SUCCESS){
        QL_MQTT_LOG("-1-NWKLOG:mqtt publish telemetry timeout");
      mqtt_connected =0;
      }
	  QL_MQTT_LOG("-1-NWKLOG:Mqtt wait exit publish topic 1 result");
    }
    ql_gpio_set_level(0, LVL_LOW);
  }
  if (topic == 2) {
    ql_gpio_set_level(0, LVL_HIGH);
    if ((ql_mqtt_publish( & mqtt_cli, "v1/devices/me/attributes", data, dataSize, 1, 0, mqtt_requst_result_cb, arg) == MQTTCLIENT_WOUNDBLOCK)) {
      QL_MQTT_LOG("-1-NWKLOG:Mqtt wait enter publish topic 2 result");
      status=ql_rtos_semaphore_wait(mqtt_semp,120000);
      if(status!=QL_OSI_SUCCESS){
        QL_MQTT_LOG("-1-NWKLOG:mqtt publish attributes timeout");
      mqtt_connected =0;
      }
	  QL_MQTT_LOG("-1-NWKLOG:Mqtt wait enter publish topic 2result");
    }
    ql_gpio_set_level(0, LVL_LOW);
  }
}


void mqttProcess_thread(void * arg) {
  mqttQueue_t mqttData;
  nwk_t nwk;
  ql_rtos_task_sleep_s(3);
  QL_MQTT_LOG("-1-PSTART:mqttProcess_thread");
  ql_rtos_semaphore_create( & mqtt_semp, 0);
  ql_dev_get_imei(imei_no, sizeof(imei_no), 0);
  QL_MQTT_LOG("-1-NWKLOG:IMEI NO: %s\r\n", imei_no);
  mqtt_init();
  mqttConnectProcess();
  while (1) {
    mqttData.dataSize = 0;
    nwk = networkCheckRecovery();
    ql_rtos_queue_wait(ql_mqttSend_queueHandle, (uint8 * ) & mqttData, sizeof(mqttQueue_t), 1000);
    if (mqttData.dataSize) {
      if (nwk.nwkStatus) {
        if (mqttData.maxPayloadCheck == 1) {
          if (maxPayLoadChecker(mqttData.rtc))
            mqtt_publish(mqttData.data, mqttData.dataSize, mqttData.topic, & mqttData.mqttFsInfo);
        } else
          mqtt_publish(mqttData.data, mqttData.dataSize, mqttData.topic, & mqttData.mqttFsInfo);
      }
    }
  }
  ql_rtos_semaphore_delete(mqtt_semp);
  ql_rtos_task_delete(mqtt_task);
  return;
}