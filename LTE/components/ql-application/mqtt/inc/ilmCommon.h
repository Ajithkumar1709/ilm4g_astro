/**  
  @file
 mqtt_demo.h

  @brief
  This file provides the definitions for datacall demo, and declares the 
  API functions.

*/
/*============================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


WHEN        WHO            WHAT, WHERE, WHY
----------  ------------   ----------------------------------------------------

=============================================================================*/

#ifndef ILM_COMMON_H
#define ILM_COMMON_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "ql_api_osi.h"
#include "ql_api_nw.h"
#include "ql_api_dev.h"
#include "ql_log.h"

#define QL_MQTT_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)		     	QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	   QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)

#define D_NOTUPLOADED 1
#define D_UPLOADED    2

#define POWER_ON_REBOOT 1
#define SOFTWARE_REBOOT 4

#define D_slot     1
#define D_serv     2
#define D_ping     3
#define D_pint     4
#define D_board    5
#define D_mpc      6
#define D_ts       7
#define D_ldr      8
#define D_vipt     9
#define D_load            10
#define D_BATKILLTIMER    11
#define D_APN    12
#define D_AstroManual    13
#define D_AstroAuto    14


#define D_RELAY_ON 1
#define D_RELAY_OFF 0
#define D_RELAY_DIM 2


#define RTC_SET     1
#define RTC_NOTSET  2

#define NO_RESET 0
#define DO_RESET 1

#define NOT_DONE 0
#define DONE 1

#define MAX_SLOT_SIZE 6
#define D_NONSLOTINDEX 7

typedef uint8_t U1 ;
typedef uint32_t U4 ;
typedef uint16_t U2 ;
typedef uint64_t U8 ;
typedef char S1 ;

// ILM PROCESS EVENTS
#define D_SEND_NO_DATA        -1
#define D_POWER_RETAIN         1
#define D_PERIODIC_INTERVAL    2
#define D_RPC                  3
#define D_GNSS_DPROCESS        4
#define D_POWER_FAIL           5
#define D_LAMP_BURN            6
#define D_ICCID                7
#define D_ASTRO                8

#define true 1
#define false 0
#define D_FAULT 1
#define D_NOFAULT 0

#define BATTERY_MODE 2
#define AC_SUPPLY 1
#define FLICKER_MODE 3


#define D_COCONTROLLER_OK  1
#define D_COCONTROLLER_NOTOK 2

typedef struct
{
  float voltage;
  float current;
  float watt;
  float calculatedPF;
  U1 gridStatus;
}acParam_t;


typedef struct{
U4 powerFailTs;
U4 powerRetainTs;
}powerFRTs_t;

typedef struct{
  U2 noLoadFault  :1;
  U2 relayFault :1;
  U2 voltMinFault :1;
  U2 voltMaxFault :1;
  U2 currentMinFault :1;
  U2 currentMaxFault :1;
  U2 wattMinFault :1;
  U2 wattMaxFault :1;
  U2 lowPowerFactorFault :1;
}faults_t;


typedef struct{
   U2 fcSet;
   U2 fcClear;
   U2 fd;
   faults_t faults;
}faultPkt_t;

typedef struct __attribute__((packed)){
 U1     state;
 U1     data[256];
 U2     dataLength;
 U1     uploadStatus;
 U4     ts;
 U1     crc;
}fsDataBackUp_t;

typedef struct{
  double longitude;
  double latitude;
  float  hdop;
  float altitude;
  unsigned int satellites_num;
  unsigned char valid;
  unsigned char quality;
}gnssElements_t;


typedef enum
{
    event_live = 0,
    event_lamp_on,
    event_lamp_off,
    event_lamp_dim,
    event_lamp_fault_set,
    event_lamp_fault_clear,
    event_config,
    event_power_retain,
    event_battery_power_retain,
    event_gnss_packet,
} event_type;


typedef struct{
   U4 daySecs;
   U4 timeInSec;
   U4 status;
   U2 day;
   
}rtc_t;


typedef struct{
U1 fsUpdateRequired;
U2 index;
}mqttFsStateUpdate_t;



typedef struct {
  U1 data[350];
  U2 dataSize; //2
  U1 topic;
  U1 maxPayloadCheck;
  rtc_t rtc;
  mqttFsStateUpdate_t mqttFsInfo;
  powerFRTs_t powerFRTs;
}
mqttQueue_t;
typedef struct{
   mqttQueue_t mqttSendData;
   U1 pktResponse;
   U1 sendPkt;
}pubHandler_t;

typedef struct {
  U1 nwkStatus;
  U1 mqttStatus;
}
nwk_t;
typedef struct {
  U1 data[350];
  U1 dataSize;
  U1 pktType;
  U1 maxPayloadCheck;
  rtc_t rtc;
  mqttFsStateUpdate_t mqttFsInfo;
  powerFRTs_t powerFRTs;
}
ilmQueue_t;


typedef struct {
  U1 process;
  U4 fromTime;
  U4 toTime;
}payLoadQueue_t;


typedef enum {LAMP_OFF,LAMP_ON,LAMP_FAULT}lampState_e;


typedef struct{
 lampState_e state;  
 U1 brightness;
 U1 relay;
 U4 burnTime;
}lampStatus_t;

typedef enum {
 slot_time=1, manual, testing ,photocell,astro_Manual,astro_Auto
}ilm_mode_t;


typedef struct{
S1 firmwareVersion[20];
S1 boardNumber[20];
}ilmProjAttributes_t;
typedef struct{
  S1 host[40];
  U2 port;
}server_t;


typedef struct {
U1 brightness[MAX_SLOT_SIZE];
U4 timeInSecs[MAX_SLOT_SIZE];
}slot_t;

typedef struct{
U4 minVolt;
U4 maxVolt;
U4 minCurrent;
U4 maxCurrent;
U4 minWatt;
U4 maxWatt;
U4 minLoad;
U4 noLoad;
U4 minPowerFactor;
}acThresholds_t;

typedef struct{
U4 ldrMin;
U4 ldrMax;
}ldrThreshold_t;


typedef struct{
ilm_mode_t mode;  
server_t server;
 U2 maxPayload;
U2 periodicInterval;
U2 batKillDelay;
S1 apn[33];
ilmProjAttributes_t ilmProjAttributes;
slot_t slot;
acThresholds_t acThresholds;
ldrThreshold_t ldrThresholds;
U1 firstBoot;
}ilmConfig_t;

typedef struct{
  U1 crc;
lampStatus_t lamp;
powerFRTs_t powerFRTs;  //fail retain time series
U2 currentPerDayPayload;
U1 currentDay;
ilmConfig_t ilmConfig;
}ilm_t;

 typedef struct{
  U1 firstBoot;
  U1 ResetFlage;
  U1 nextDayFlage;
  U1 crc;
}ilmReset_t;

typedef struct{
   U1 firstBoot;
   int32_t astroOnOffset;
   int32_t astroOffOffset;
   double astroLatManual;
   double astroLongManual;
   double astroLatAuto;
   double astroLongAuto;
   uint32_t astroSlot_on_time;
   uint32_t astroSlot_off_time;
   U2 storedDay;
   U1 crc;
}ilmAstro_t;


		
#endif   /*DATACALL_DEMO_H*/

