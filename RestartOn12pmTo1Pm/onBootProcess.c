#include "ilmCommon.h"
#include "ql_api_rtc.h"
#include "rtc.h"
#include "powerFailHandler.h"
#include "onBootProcess.h"
#include "ql_api_sim.h"
//DEFINES
#define D_DATA_NOTSEND 0
#define D_DATA_SENDED 1
//GLOBALS AND STATICS
U1 onBootPowerRetainProcessed = 0;
//EXTERNS
extern int  mqtt_connected;
extern ilm_t ilm;
extern ql_queue_t ql_ilmProcess_qHandle;
extern ql_task_t onBoot_task;
extern U1 batKillEnable;
extern bool ResetFlage;
extern uint16_t randomResetTime;

void onBootProcess_thread(void * param) {
  gnssElements_t gnssElements;
  ilmQueue_t ilmData;
  U1 gpsPacketDoOnce = 0;
  U4 rtcNotSetSecs = 0;
  U4 pwrrTsSetDoOnce = 0;
  rtc_t rtc;
  ql_rtos_task_sleep_s(7);
  QL_MQTT_LOG("-1-PSTART:ONBOOT PROCESS STARTED", rtcNotSetSecs);
  
  while (1) {
    gnssElements = gnss_data();
    rtc = getRtc();
    
   /* if (rtc.status == RTC_SET && rtc.daySecs >= 54000 && rtc.daySecs <= 54006) {
      ql_dev_set_modem_fun(1, 1, 0);
  }*/
  if (ResetFlage == DO_RESET) {
    if (rtc.status == RTC_SET && rtc.daySecs >= 43200&&rtc.daySecs <= 46800) {
  if(rtc.daySecs >=randomResetTime){
  if (!getPowerFailSense()){
    ql_dev_set_modem_fun(1, 1, 0);
     }
    } 
  }
  }

    ql_rtos_task_sleep_s(1);
    QL_MQTT_LOG("-1-PSTART:ONBOOT PROCESS STARTED %d",rtc.timeInSec );
    //powerFailSenseRoutine(rtc);
     if(batKillEnable==true)
      batKillAction();
    
    //Power Retain Packet Send 
    if (rtc.status == RTC_NOTSET) {
      rtcNotSetSecs++;
      QL_MQTT_LOG("-1-ONBOOTLOG:RTC_NOTSET RTC SECS=%d", rtcNotSetSecs);
    }
    if(rtc.status == RTC_SET && pwrrTsSetDoOnce == 0) {
      ilm.powerFRTs.powerRetainTs = rtc.timeInSec - rtcNotSetSecs;
      QL_MQTT_LOG("-1-ONBOOTLOG:POWER RETAIN TS=%ld ,RTC TIME =%ld,RTC NS TIME =%ld ",ilm.powerFRTs.powerRetainTs,rtc.timeInSec,rtcNotSetSecs);
      pwrrTsSetDoOnce = 1;
      rtcNotSetSecs=0;
    }

    if ((onBootPowerRetainProcessed == D_DATA_NOTSEND) && (rtc.status == RTC_SET)) {
      QL_MQTT_LOG("-1-ONBOOTLOG:POWER REATIN PKT SENT");
      ilmData.pktType = D_POWER_RETAIN;
      ilmData.dataSize = 1;
      ilmData.powerFRTs.powerRetainTs=rtc.timeInSec;
        ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
      ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
      ilmData.pktType=D_ICCID;
      ilmData.dataSize=1;
      ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 *)&ilmData, 0);
	  
      //   ql_rtos_task_sleep_s(10);
      onBootPowerRetainProcessed =D_DATA_SENDED ;
    }

    //Gnss Data Packet Send 
    if ((gnssElements.quality == 1) && (gpsPacketDoOnce == D_DATA_NOTSEND) && (rtc.status == RTC_SET) && (gnssElements.valid == 1)) {
      QL_MQTT_LOG("-1-ONBOOTLOG:GPS PACKET SENT");
      ql_rtos_task_sleep_s(3);
      gnssElements = gnss_data();
      ilmData.pktType = D_GNSS_DPROCESS;
      ilmData.dataSize = 1;
      ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
      gpsPacketDoOnce = D_DATA_SENDED;
    }
  }
  ql_rtos_task_delete(onBoot_task);

}