#include "ilmCommon.h"
#include "memoryHandler.h"
#include "rpcHandler.h" //protobuf
#include "pb_decode.h"
#include "pb1.h"
#include "pb_common.h"
#include "getMsg.pb.h"
#include "rpcNew.pb.h"
#include "fota_ftp.h"

//DEFINES
//CLEAR PARAMS RPC
#define D_BURNTIME_CLR     5
#define D_REBOOT           13
#define D_DEFAULT          0 
#define D_PAYLOADCOUNTER   6

//SLOT
#define MAX_ALARM_SIZE 6
#define SECONDS_PER_DAY 86400

//GLOBALS AND STATICS
const char methodString[rpc_packet_end][6] = {
  "ctrl",
  "set",
  "get",
  "clr",
  "data",
  "ota",
  "ping"
};

unsigned char buffer[350];
rpc_RpcRequestMsg decodedRpc = {};

//EXTERNS
extern U1 presentSlot;
extern U1 lampAction(U1 lamp, char bri);
extern ilm_t ilm;
extern int fota_ftp_app(void);
extern mqttQueue_t publish_gps_data(void);
extern mqttQueue_t publish_config(sendConfig type);
extern ilmAstro_t ilmAstro;
extern void astroLatLongAutoSet(void);
pubHandler_t pub_handler(uint8_t * chunk, uint16_t chunk_len) {
  pubHandler_t pubHandler;
  QL_MQTT_LOG("-1-RPCLOG:PUB_HANDLER\n");
  sendConfig _configResponse = config_SendNothing;
  memset( & decodedRpc, 0, sizeof(decodedRpc));
  /* Create a stream that reads from the buffer. */
  pb_istream_t dsstream = pb_istream_from_buffer(chunk, chunk_len);
  /* Now we are ready to decode the message. */
  if (pb_decode( & dsstream, rpc_RpcRequestMsg_fields, & decodedRpc)) {
    QL_MQTT_LOG("-1-RPCLOG:ProtoBuff Decode Success %lu\n", decodedRpc.params.rpcType);
    //ilmParamFlashRead();
    ilm_mode_t control = ilm.ilmConfig.mode;
    U1 sendPkt = 0;
    uint8_t slotIndex = 0;
    char
    const * method = decodedRpc.method;
    uint8_t methodCnt = 0;
    for (methodCnt = rpc_packet_control; methodCnt < rpc_packet_end; methodCnt++) {
      if (!strcmp(method, methodString[methodCnt]))
        break;
    }
    if (methodCnt != rpc_packet_end) {
      clock_time_t slotTime[6] = {
        (clock_time_t) decodedRpc.params.s0,
        (clock_time_t) decodedRpc.params.s1,
        (clock_time_t) decodedRpc.params.s2,
        (clock_time_t) decodedRpc.params.s3,
        (clock_time_t) decodedRpc.params.s4,
        (clock_time_t) decodedRpc.params.s5
      };
      uint8_t slotBrightPercentage[6] = {
        (uint8_t) decodedRpc.params.b0,
        (clock_time_t) decodedRpc.params.b1,
        (clock_time_t) decodedRpc.params.b2,
        (clock_time_t) decodedRpc.params.b3,
        (clock_time_t) decodedRpc.params.b4,
        (clock_time_t) decodedRpc.params.b5
      };

      char
      const * rpc_board_number = decodedRpc.params.board;

      /*uint32_t timestamp = (uint32_t) decodedRpc.params.ts;*/
      mqtt_get_type_t mqtt_get_type; // = (mqtt_get_type_t) decodedRpc.params.rpcType;

      switch (methodCnt) {
      case rpc_packet_control:
        if ((decodedRpc.params.rpcType >= 2) && (decodedRpc.params.rpcType <= 4)) {
          control = (ilm_mode_t) decodedRpc.params.mode; //mode
          if (slot_time == control) {
            QL_MQTT_LOG("-1-RPCLOG:Time Slot Mode \r\n");
            presentSlot = D_NONSLOTINDEX;
            ilm.ilmConfig.mode = slot_time;
            //slotModeAction
            _configResponse = config_slotSend;
          }
          if (manual == control) {
            QL_MQTT_LOG("-1-RPCLOG:Manual Mode \r\n");
            ilm.ilmConfig.mode = manual;
            sendPkt = lampAction(decodedRpc.params.lamp, decodedRpc.params.bri);
            if (sendPkt)
              _configResponse = config_sendLivePacket;

            QL_MQTT_LOG("-1-RPCLOG:my_log_decodedRpc.params.lamp=%d,decodedRpc.params.bri=%d", decodedRpc.params.lamp, decodedRpc.params.bri);
          }
          nvmWriteIlmConfig();
        }
        break;

      case rpc_packet_config: //SET COMMAND
        if ((decodedRpc.params.rpcType >= 1) && (decodedRpc.params.rpcType <= 12)) {
          switch (decodedRpc.params.rpcType) {
          case D_slot:
            QL_MQTT_LOG("-1-RPCLOG:SLOT CONFIG RCD\r\n");
            for (slotIndex = 0; slotIndex < MAX_ALARM_SIZE; slotIndex++) {
              if (slotTime[slotIndex] > SECONDS_PER_DAY)
                break;
              if (!((slotBrightPercentage[slotIndex] >= 0) && (slotBrightPercentage[slotIndex] <= 100)))
                break;
            }
            if (slotIndex == MAX_ALARM_SIZE) {
              for (slotIndex = 0; slotIndex < MAX_ALARM_SIZE; slotIndex++) {
                ilm.ilmConfig.slot.timeInSecs[slotIndex] = slotTime[slotIndex];
                ilm.ilmConfig.slot.brightness[slotIndex] = slotBrightPercentage[slotIndex];
              }
              ilm_mode_t control = ilm.ilmConfig.mode;
              //MODE
              control = (ilm_mode_t) decodedRpc.params.mode;
            if (control == slot_time) {
                presentSlot = D_NONSLOTINDEX;
                ilm.ilmConfig.mode = slot_time;
                QL_MQTT_LOG("-1-RPCLOG:Time Slot Mode \r\n");
                //slotModeAction
                nvmWriteIlmConfig();
              }
              if((control == astro_Manual) || (control == astro_Auto)) {
                nvmReadAstroFileWithBackup();
                ilmAstro.astroOnOffset = decodedRpc.params.astroOnOffSet;
                ilmAstro.astroOffOffset = decodedRpc.params.astroOffOffSet;
                if (control == astro_Manual) {
                  ilmAstro.astroLatManual = decodedRpc.params.astroLatitude;
                  ilmAstro.astroLongManual = decodedRpc.params.astroLongitude;
                }
                 nvmWrireAstroFileWithBackup();
                if(control == astro_Auto){
                  astroLatLongAutoSet(); 
                }
                presentSlot = D_NONSLOTINDEX;
                ilm.ilmConfig.mode = control;
                QL_MQTT_LOG("-1-RPCLOG:Time Slot Mode \r\n");
                  //slotModeAction
                  nvmWriteIlmConfig();
                }
            }
        
            _configResponse = config_slotSend;
            break;
          case D_serv:
            /*server settings*/
            QL_MQTT_LOG("-1-RPCLOG:SERV CONFIG RCD\r\n");
            memset(ilm.ilmConfig.server.host, 0, sizeof(ilm.ilmConfig.server.host));
            memcpy(ilm.ilmConfig.server.host, decodedRpc.params.baddr, strlen(decodedRpc.params.baddr));
            ilm.ilmConfig.server.port = (uint16_t) decodedRpc.params.bport;
            nvmWriteIlmConfig();
            _configResponse = config_servSend;
            break;
          case D_ping:
            /*ping interval*/
            QL_MQTT_LOG("-1-RPCLOG:Publish mqtt_keep_alive change\r\n");
            break;
          case D_pint:
            /*publish interval*/
            QL_MQTT_LOG("-1-RPCLOG:Publish interval change\r\n");
            ilm.ilmConfig.periodicInterval = (uint16_t) decodedRpc.params.pint;
            nvmWriteIlmConfig();
            _configResponse = config_otherSend;
            break;
          case D_board:
            /*board Commands*/
            QL_MQTT_LOG("-1-RPCLOG:BOARD NUMBER CONFIG RCD\r\n");
            memset(ilm.ilmConfig.ilmProjAttributes.boardNumber, 0, sizeof(ilm.ilmConfig.ilmProjAttributes.boardNumber));
            memcpy(ilm.ilmConfig.ilmProjAttributes.boardNumber, rpc_board_number, strlen(rpc_board_number));
            nvmWriteIlmConfig();
            _configResponse = config_boardSend;
            break;
          case D_mpc:
            /* maximum packet counter*/
            QL_MQTT_LOG("-1-RPCLOG:Max packet counter per day\r\n");
            ilm.ilmConfig.maxPayload = (uint16_t) decodedRpc.params.mpc;
            nvmWriteIlmConfig();
            _configResponse = config_otherSend;
            break;

          case D_ts:

            /*time Commands*/
            /*
            QL_MQTT_LOG("TIME SET THROUGH RPC\r\n");
            PCF2129_Set_TimeStamp(timestamp);
            ntp_set_timestamp(timestamp);
            */
            break;
          case D_ldr:
            /*ldr Thresholds command*/
            ilm.ilmConfig.ldrThresholds.ldrMin = (uint32_t) decodedRpc.params.pmin; // photocell voltage min
            ilm.ilmConfig.ldrThresholds.ldrMax = (uint32_t) decodedRpc.params.pmax; // photocell voltage max
            nvmWriteIlmConfig();
            _configResponse = config_thresholdSend;

            break;

          case D_vipt:
            QL_MQTT_LOG("-1-RPCLOG:RPC D_vipt\r\n");
            ilm.ilmConfig.acThresholds.maxVolt = (uint32_t) decodedRpc.params.vmax; // photocell voltage min
            ilm.ilmConfig.acThresholds.minVolt = (uint32_t) decodedRpc.params.vmin; // photocell voltage max
            ilm.ilmConfig.acThresholds.maxCurrent = (uint32_t) decodedRpc.params.imax; // photocell voltage min
            ilm.ilmConfig.acThresholds.minCurrent = (uint32_t) decodedRpc.params.imin; // photocell voltage max
            ilm.ilmConfig.acThresholds.maxWatt = (uint32_t) decodedRpc.params.wmax; // photocell voltage min
            ilm.ilmConfig.acThresholds.minWatt = (uint32_t) decodedRpc.params.wmin; // photocell voltage max
            ilm.ilmConfig.acThresholds.minPowerFactor = (uint32_t) decodedRpc.params.pmin; // photocell voltage min
            nvmWriteIlmConfig();
            _configResponse = config_thresholdSend;
            break;

          case D_load:
            /*no load and min load thresholds*/
            QL_MQTT_LOG("-1-RPCLOG:LOAD THRESHOLD RCD\r\n");
            ilm.ilmConfig.acThresholds.minLoad = (uint32_t) decodedRpc.params.mload;
            ilm.ilmConfig.acThresholds.noLoad = (uint32_t) decodedRpc.params.nload;
            nvmWriteIlmConfig();
            _configResponse = config_thresholdSend;
            break;

          case D_BATKILLTIMER:
            ilm.ilmConfig.batKillDelay = (uint16_t) decodedRpc.params.timer; //timer
            nvmWriteIlmConfig();
            _configResponse = config_allSend;
            break;
             
          case D_APN:
            //ilm.ilmConfig.batKillDelay = (uint16_t) decodedRpc.params.timer; //timer
            memset(ilm.ilmConfig.apn, 0, sizeof(ilm.ilmConfig.apn));
            memcpy(ilm.ilmConfig.apn,decodedRpc.params.apn,strlen(decodedRpc.params.apn));
             QL_MQTT_LOG("-1-RPCLOG:apn=%sr\n",decodedRpc.params.apn);
            nvmWriteIlmConfig();
            _configResponse =config_apnSend;
            break;
          }
        }
        break;
      case rpc_packet_ota:
        QL_MQTT_LOG("-1-RPCLOG:OTA CMD RCD\r\n");
        if (decodedRpc.params.rpcType == 1) {
          if (fota_ftp_app() == FAIL) {
            fota_ftp_app();
            //otaFun((uint32_t)decodedRpc.params.version);
          }
        }
        break;
      case rpc_packet_default_settings:
        if ((decodedRpc.params.rpcType >= 1) && (decodedRpc.params.rpcType <= 14)) {
          U4 clear_param = (decodedRpc.params.rpcType - 1);
          if (clear_param == D_BURNTIME_CLR) {
            QL_MQTT_LOG("-1-RPCLOG:D_BURNTIME_CLR\r\n");
            ilm.lamp.burnTime = 0;
            nvmWriteIlmConfig();
          }
          if (clear_param == D_REBOOT) {
            QL_MQTT_LOG("-1-RPCLOG:D_REBOOT\r\n");
           ql_dev_set_modem_fun(1, 1, 0);
          
          }
          if (clear_param == D_DEFAULT) {
            QL_MQTT_LOG("-1-RPCLOG:D_DEFAULT\r\n");
            presentSlot = D_NONSLOTINDEX;
            loadIlmDefaultConfig();
            nvmWriteIlmConfig();
          }
          if (clear_param == D_PAYLOADCOUNTER) {
            QL_MQTT_LOG("-1-RPCLOG:D_PAYLOADCOUNTER\r\n");
            ilm.currentPerDayPayload = 0;
            nvmWriteIlmConfig();

          }
          _configResponse = config_allSend;
        }
        break;
      case rpc_packet_get:
        if ((decodedRpc.params.rpcType >= 1) && (decodedRpc.params.rpcType <= 7)) {
          mqtt_get_type = (mqtt_get_type_t)(decodedRpc.params.rpcType - 1);
          switch ((uint8_t) mqtt_get_type) {
          case mqtt_get_live:
            //GET LIVE ACTION
            _configResponse = config_sendLivePacket;
            sendPkt = 0;

            break;
          case mqtt_get_settings_telemetry:
          case mqtt_threshold_settings_telemetry:
          case mqtt_get_settings_attributes:
          case mqtt_threshold_settings_attributes:
            _configResponse = config_allSend;
            break;
          case mqtt_get_version:
            _configResponse = config_versionSend;
            break;
          case mqtt_gps_send:
            // publish_gps_data();
            _configResponse = config_sendGpsPacket;
            sendPkt=9;
            
            //  post_ct_coil(0);
            break;
          }
        }
        break;
      case rpc_packet_get_payload:
        if (decodedRpc.params.rpcType == 1) {
          QL_MQTT_LOG("-1-RPCLOG:GET PACKET LOG  RCD\r\n");
          extern ql_queue_t ql_payloadProcess_qHandle;
          payLoadQueue_t payLoadQueue;
          payLoadQueue.process = true;
          payLoadQueue.fromTime = (uint32_t) decodedRpc.params.from;
          payLoadQueue.toTime = (uint32_t) decodedRpc.params.to;
          ql_rtos_queue_release(ql_payloadProcess_qHandle, sizeof(payLoadQueue_t), (uint8 * ) & payLoadQueue, 0);
        }
        break;
      }
    }

    if (config_sendLivePacket == _configResponse) {
      pubHandler.pktResponse = 1;
      pubHandler.sendPkt = sendPkt;
      return pubHandler;
    }
    if (config_sendGpsPacket == _configResponse) {
     // pubHandler.mqttSendData = publish_gps_data();
     // pubHandler.pktResponse = 0;
     pubHandler.pktResponse = 1;
      pubHandler.sendPkt = sendPkt;
      return pubHandler;
    }
    if (config_SendNothing != _configResponse) {
      pubHandler.mqttSendData = publish_config(_configResponse);
      pubHandler.pktResponse = 0;
      return pubHandler;
    }
    if (config_SendNothing == _configResponse) {
      pubHandler.mqttSendData.dataSize = 0;
      pubHandler.pktResponse = 0;
      return pubHandler;
    }
  } else
    QL_MQTT_LOG("-1-RPCLOG:=======ProtoBuff Decode Fail======= \n");
  return pubHandler;
}