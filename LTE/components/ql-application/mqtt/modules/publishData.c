#include "ilmCommon.h"
#include "ql_api_nw.h"
#include "publishData.h"
#include "rpcHandler.h"
#include "getMsg.pb.h"
#include "pb1.h"
#include "telemetry.pb.h"
#include "rtc.h"
#include "powerFailHandler.h"
#include "pb_common.h"
#include "pb_encode.h"
#include "memoryHandler.h"
#include "ql_fs.h"
#include "ql_api_sim.h"
//DEFINES


//GLOBALS AND STATICS
ql_nw_signal_strength_info_s ss_info;
int32_t ql_temp;
attributes_getMsg msg = attributes_getMsg_init_zero;
const char ifirmver[17] = "02.01.00.03";
pb_ostream_t stream;
unsigned char buffer[350];
telemetry_Telemetry mymessage = telemetry_Telemetry_init_zero;
gnssElements_t gnssElements;
static U2 dataStoreCount=0;
static U4 previousrtcTime=0;
char siminfo[64] = {0};
char siminfoIMSI[64] = {0};

//EXTERNS
extern ilm_t ilm;
extern acParam_t  acParam;
extern uint8_t coControllerStatus;
extern faultPkt_t faultPkt;
extern ql_mutex_t dataFileMutex;
extern U2 currentFileIndex;
extern QFILE dataFile;
extern char dataLogFile[];
extern uint8_t crc8_cal(uint8_t * data, uint16_t length);
extern U1 instaGetPowerFailSense(void);
extern int signal;
extern ilmAstro_t ilmAstro;
extern uint8_t astroTime(double lat, double longn);

mqttQueue_t publish_data(U1 pktType,powerFRTs_t powerFRTs) {
  rtc_t rtc;
  rtc= getRtc();
  if(previousrtcTime==rtc.timeInSec)
  {
    ql_rtos_task_sleep_s(2);
    rtc= getRtc();
  }
  previousrtcTime=rtc.timeInSec;
  
 ql_nw_get_signal_strength(0, &ss_info);
  if(ss_info.rssi==0)
  ss_info.rssi=signal;
  QL_MQTT_LOG("-1-PUBLISHLOG:RSSI: %d\r\n", ss_info.rssi);

  ql_dev_get_temp_value(&ql_temp);
  QL_MQTT_LOG("-1-PUBLISHLOG:TEMPERATURE: %d\r\n", ql_temp);
  
  mqttQueue_t mqttSendData;
  QL_MQTT_LOG("-1-PUBLISHLOG:=========my_log_RTC TIME IN SEC IN CHECK RTC SET : %ld",rtc.timeInSec);
  QL_MQTT_LOG("-1-PUBLISHLOG:=========my_log_RTC TIME IN mqtt rtc.daySecs: %d",rtc.daySecs);
   QL_MQTT_LOG("-1-PUBLISHLOG:=========my_log_type_packet %d",pktType);
  //uint8_t msgPtr=(uint8_t)&mymessage;
  uint8_t *msgPtr=(uint8_t*)&mymessage;
  memset(msgPtr, 0, sizeof(mymessage));
  mymessage.ts=(uint64_t)rtc.timeInSec*1000;// mymessage.ts=(uint64_t) (1698922293)* 1000//(uint64_t) payload.timestamp * 1000;
  mymessage.values.pkt= pktType;
  mymessage.values.mode= ilm.ilmConfig.mode;//ilm.ilmConfig.mode;
  mymessage.values.rly= ilm.lamp.relay;//ilm.lamp.relay;
  mymessage.values.lmp= ilm.lamp.state;//ilm.lamp.state;
  mymessage.values.bri= ilm.lamp.brightness;//ilm.lamp.brightness;
  mymessage.values.a0=00;
  mymessage.values.ri=  acParam.current;//15;//acParam.current;
  mymessage.values.rv=  acParam.voltage*1000;//22;//acParam.voltage;
  mymessage.values.rw=  acParam.watt*1000;// 33;//acParam.watt;
  mymessage.values.rpf=acParam.calculatedPF*100;
  mymessage.values.presentGridStatus=acParam.gridStatus;//(getPowerFailSense()==TRUE)?BATTERY_MODE:AC_SUPPLY;//2-battey_mode,1-AC supply
  mymessage.values.coControllerStatus=coControllerStatus;//coControllerStatus;
  mymessage.values.rbt=ilm.lamp.burnTime; //ilm.lamp.burnTime;
  if(pktType==event_power_retain){
    mymessage.values.pwrr=powerFRTs.powerRetainTs;
   // mymessage.values.pwrf=ilm.powerFRTs.powerFailTs;
    mymessage.values.pwrf=powerFRTs.powerFailTs;
    
  }
  if(pktType==event_battery_power_retain){
    mymessage.values.pwrf=powerFRTs.powerFailTs;
    
  }
  if(pktType==event_lamp_fault_clear)
    mymessage.values.fc=faultPkt.fcClear;
  if(pktType==event_lamp_fault_set)
    mymessage.values.fc=faultPkt.fcSet;
  mymessage.values.fd=faultPkt.fd;//payload.fault;
    //mymessage.values.ctp= 1;//payload.ctemp;
  mymessage.values.sig= ss_info.rssi;//payload.rssi;
    //mymessage.values.rnk=1;//curr_instance.dag.rank;

  if(pktType== event_gnss_packet){
    gnssElements=gnss_data();
    mymessage.values.latitude=gnssElements.latitude;
    mymessage.values.longitude=gnssElements.longitude;
    mymessage.values.altitude=gnssElements.altitude;
    mymessage.values.gnssFix=gnssElements.quality;
    mymessage.values.satellite=gnssElements.satellites_num;
    mymessage.values.hdop=gnssElements.hdop;
    mymessage.values.coControllerStatus=coControllerStatus;
    strcpy(mymessage.values.version,ifirmver);
  }

  mymessage.has_values=true;

  

	QL_MQTT_LOG("-1-PUBLISHLOG:======INSIDE PUBLISH");
  stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (pb_encode(&stream, telemetry_Telemetry_fields, &mymessage)){
          QL_MQTT_LOG("-1-PUBLISHLOG:Publish Data Encode Success %d,ts=%ld,pkttype=%d\n",stream.bytes_written,rtc.timeInSec, mymessage.values.pkt);
          //  QL_MQTT_LOG("=========my_log_type_ mymessage.values.pkt %d", mymessage.values.pkt);
          memset(mqttSendData.data,0,sizeof(mqttSendData.data));
          memcpy(mqttSendData.data,buffer,stream.bytes_written);
          mqttSendData.dataSize = stream.bytes_written;
          mqttSendData.topic = 1;
          mqttSendData.mqttFsInfo.fsUpdateRequired =true;
          mqttSendData.mqttFsInfo.index =currentFileIndex;
          mqttSendData.maxPayloadCheck=1;
          mqttSendData.rtc=rtc;
           //ilmStore ,for burn time storing
           if(!(instaGetPowerFailSense()==BATTERY_MODE))
             nvmWriteIlmConfig();
             
            
      
          if((!(instaGetPowerFailSense()==BATTERY_MODE))||(mymessage.values.pkt==8)){
             ql_rtos_mutex_lock((ql_mutex_t)dataFileMutex, QL_WAIT_FOREVER);
             fsDataWrite(&dataFile,(U1 *)&mqttSendData.data,mqttSendData.dataSize,&currentFileIndex,rtc.timeInSec);
             QL_MQTT_LOG("-1-PUBLISHLOG:dataStoreCount=%d\n",dataStoreCount++);
             ql_rtos_mutex_unlock((ql_mutex_t)dataFileMutex);
          }
          //fsDataUpdateUploadState(mqttSendData.mqttFsInfo.index);
          //fsPayloadUpload(1701185119,1701185119);
          return mqttSendData;
   }
  else
      {
          QL_MQTT_LOG("-1-PUBLISHLOG:Publish Data Encoding Failed\n");
          mqttSendData.dataSize =0;
      }
   return mqttSendData;
}


 mqttQueue_t publish_gps_data(void) {
    rtc_t rtc;
    rtc= getRtc();
    if(previousrtcTime==rtc.timeInSec)
  {
    ql_rtos_task_sleep_s(2);
    rtc= getRtc();
  }
  previousrtcTime=rtc.timeInSec;
    mqttQueue_t mqttSendData;
    gnssElements=gnss_data();
    uint8_t *msgPtr=(uint8_t*)&mymessage;
    memset(msgPtr, 0, sizeof(mymessage));
    mymessage.ts=(uint64_t)rtc.timeInSec*1000;
    QL_MQTT_LOG("-1-PUBLISHLOG:Payload Time Stamp value: %lu \n", rtc.timeInSec);
    mymessage.values.pkt= event_gnss_packet;
    mymessage.values.latitude=gnssElements.latitude;
    mymessage.values.longitude=gnssElements.longitude;
    mymessage.values.altitude=gnssElements.altitude;
    mymessage.values.gnssFix=gnssElements.quality;
    mymessage.values.satellite=gnssElements.satellites_num;
    mymessage.values.hdop=gnssElements.hdop;
    mymessage.values.coControllerStatus=coControllerStatus;
    strcpy(mymessage.values.version,ifirmver);
    mymessage.has_values=true;
    QL_MQTT_LOG("-1-PUBLISHLOG:======INSIDE PUBLISH");
    stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (pb_encode(&stream, telemetry_Telemetry_fields, &mymessage)){
          QL_MQTT_LOG("-1-PUBLISHLOG:Publish Data Encode Success %d\n",stream.bytes_written);
          memset(mqttSendData.data,0,sizeof(mqttSendData.data));
          memcpy(mqttSendData.data,buffer,stream.bytes_written);
          mqttSendData.dataSize = stream.bytes_written;
          mqttSendData.topic = 1;
          mqttSendData.mqttFsInfo.fsUpdateRequired =true;
          mqttSendData.mqttFsInfo.index =currentFileIndex;
          mqttSendData.maxPayloadCheck=1;
          mqttSendData.rtc=rtc;
          ql_rtos_mutex_lock((ql_mutex_t)dataFileMutex, QL_WAIT_FOREVER);
          fsDataWrite(&dataFile,(U1 *)&mqttSendData.data,mqttSendData.dataSize,&currentFileIndex,rtc.timeInSec);
          //fsDataUpdateUploadState(mqttSendData.mqttFsInfo.index);
          //fsPayloadUpload(1701185119,1701185119);
          ql_rtos_mutex_unlock((ql_mutex_t)dataFileMutex);
    }
    else
    {
        QL_MQTT_LOG("-1-PUBLISHLOG:Publish Data Encoding Failed\n");
        mqttSendData.dataSize =0;
          
    }
   return mqttSendData;
}





mqttQueue_t publish_config(sendConfig type) {
   mqttQueue_t mqttSendData;
   uint8_t *msgPtr=(uint8_t*)&msg;
   memset(msgPtr, 0, sizeof(msg));
   strcpy(ilm.ilmConfig.ilmProjAttributes.firmwareVersion,ifirmver);
      
   if(type==config_gpsSend){
      strcpy(msg.verStr,ilm.ilmConfig.ilmProjAttributes.firmwareVersion);
        //msg.type=ilm.soft_reboot;
    }

   if((type==config_versionSend)||(type==config_allSend)){
    strcpy(msg.verStr,ilm.ilmConfig.ilmProjAttributes.firmwareVersion);
        //msg.type=ilm.soft_reboot;
    }

   if((type==config_otherSend)||(type==config_allSend)){
    //mode - slot/manual/testing
    msg.mode=ilm.ilmConfig.mode;
    //publish interval
    msg.pint=ilm.ilmConfig.periodicInterval;
    //ping interval
   // msg.ping=10;//dummy value
    //maximum payload per day
    msg.mpc=ilm.ilmConfig.maxPayload;
    //  printf("packet_per_day: %u\r\n",mqtt_client_config.max_payload_per_day);
    //  printf("packet count=%lu\r\n",msg.mpc);
    msg.timer=ilm.ilmConfig.batKillDelay;
    }
    if((type==config_thresholdSend)||(type==config_allSend)){
    //threshold
    msg.mload=ilm.ilmConfig.acThresholds.minLoad;
    msg.nload=ilm.ilmConfig.acThresholds.noLoad;
    msg.vmax= ilm.ilmConfig.acThresholds.maxVolt;
    msg.vmin=ilm.ilmConfig.acThresholds.minVolt;
    msg.imax= ilm.ilmConfig.acThresholds.maxCurrent;
    msg.imin=ilm.ilmConfig.acThresholds.minCurrent;
    msg.wmax= ilm.ilmConfig.acThresholds.maxWatt;
    msg.wmin= ilm.ilmConfig.acThresholds.minWatt;
    msg.pmin=ilm.ilmConfig.acThresholds.minPowerFactor; 
    }
    if((type==config_servSend)||(type==config_allSend)){
      // server settings
        for(uint8_t i=0;ilm.ilmConfig.server.host[i]!='\0';i++){
          msg.baddr[i]=ilm.ilmConfig.server.host[i];
        }
        msg.bport=ilm.ilmConfig.server.port;
    }
    if((type==config_boardSend)||(type==config_allSend)){
        //board number
       for(uint8_t i=0;ilm.ilmConfig.ilmProjAttributes.boardNumber[i]!='\0';i++){
            if(i<30)
                msg.board[i]=ilm.ilmConfig.ilmProjAttributes.boardNumber[i];
            else{
                msg.board[i]=0;
                break;
            }
       }
    }

  if ((type == config_slotSend) || (type == config_allSend)) {
   nvmReadAstroFileWithBackup();
   msg.mode = ilm.ilmConfig.mode;
  //Astro Configs
  if ((ilm.ilmConfig.mode == astro_Manual) || (ilm.ilmConfig.mode == astro_Auto)) {
   
  if (ilm.ilmConfig.mode == astro_Manual) {
    msg.astroLatitude = ilmAstro.astroLatManual;
    msg.astroLongitude = ilmAstro.astroLongManual;
  } else {
    msg.astroLatitude = ilmAstro.astroLatAuto;
    msg.astroLongitude = ilmAstro.astroLongAuto;
  }
  astroTime(msg.astroLatitude,msg.astroLongitude);
  // msg.astroOnOffSet=ilmAstro.astroOnOffset;
  // msg.astroOffOffSet=ilmAstro.astroOffOffset;
    msg.astroOnOffSet = (ilmAstro.astroOnOffset == 0) ? 1 : ilmAstro.astroOnOffset ;
	  msg.astroOffOffSet = (ilmAstro.astroOffOffset == 0) ? 1 : ilmAstro.astroOffOffset ;

  msg.astroSlot_on = ilmAstro.astroSlot_on_time;
  msg.astroSlot_off = ilmAstro.astroSlot_off_time;
}
    //slot settings
    msg.s0 = ilm.ilmConfig.slot.timeInSecs[0];
    msg.s1 = ilm.ilmConfig.slot.timeInSecs[1];
    msg.s2 = ilm.ilmConfig.slot.timeInSecs[2];
    msg.s3 = ilm.ilmConfig.slot.timeInSecs[3];
    msg.s4 = ilm.ilmConfig.slot.timeInSecs[4];
    msg.s5 = ilm.ilmConfig.slot.timeInSecs[5];

    msg.b0 = (ilm.ilmConfig.slot.brightness[0] == 0) ? 1 : (ilm.ilmConfig.slot.brightness[0]);
    msg.b1 = (ilm.ilmConfig.slot.brightness[1] == 0) ? 1 : (ilm.ilmConfig.slot.brightness[1]);
    msg.b2 = (ilm.ilmConfig.slot.brightness[2] == 0) ? 1 : (ilm.ilmConfig.slot.brightness[2]);
    msg.b3 = (ilm.ilmConfig.slot.brightness[3] == 0) ? 1 : (ilm.ilmConfig.slot.brightness[3]);
    msg.b4 = (ilm.ilmConfig.slot.brightness[4] == 0) ? 1 : (ilm.ilmConfig.slot.brightness[4]);
    msg.b5 = (ilm.ilmConfig.slot.brightness[5] == 0) ? 1 : (ilm.ilmConfig.slot.brightness[5]);

    }
    if(type==config_apnSend){
      memcpy(msg.apn,ilm.ilmConfig.apn,strlen(ilm.ilmConfig.apn));
    }
    if((type==config_allSend)||(type==config_sendiccidPacket)){
       ql_sim_get_iccid(0, siminfo, 64);
       QL_MQTT_LOG("-1-ICCID: %s", siminfo);
        ql_sim_get_imsi(0, siminfoIMSI, 64);
        QL_MQTT_LOG("-1-IMSI %s", siminfoIMSI);
        if(strlen(siminfo)<30){
       memcpy(msg.iccid,siminfo,strlen(siminfo));
        }
        if(strlen(siminfoIMSI)<30){
       memcpy(msg.imsiNumber,siminfoIMSI,strlen(siminfoIMSI));
      }
    }
    stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (pb_encode(&stream, attributes_getMsg_fields, &msg)){
		QL_MQTT_LOG("-1-PUBLISHLOG:Pub Config Encode Pass\n");
    	memset(mqttSendData.data,0,sizeof(mqttSendData.data));
        memcpy(mqttSendData.data,buffer,stream.bytes_written);
        mqttSendData.dataSize = stream.bytes_written;
        mqttSendData.topic = 2;
        return mqttSendData;
	}
    else
        QL_MQTT_LOG("-1-PUBLISHLOG:Pub Config Encode Failed\n");
	mqttSendData.dataSize =0;
    return mqttSendData;
}


void fsPayloadUpload(U4 fromTime , U4 toTime){
    extern  ql_queue_t ql_mqttSend_queueHandle;
    mqttQueue_t mqttSendData;
    fsDataBackUp_t fileData;
	uint8_t crc_temp;
	U2 index=0;
  int dataFile;
	for(index=0;index<1440;index++){
		
    ql_rtos_mutex_lock((ql_mutex_t)dataFileMutex, QL_WAIT_FOREVER);
    dataFile = ql_fopen(dataLogFile, "ab+");
    ql_fclose(dataFile);
    memset(&fileData,0,sizeof(fileData));
    fsDataRead(&fileData,index);
    ql_rtos_mutex_unlock((ql_mutex_t)dataFileMutex);

		crc_temp = fileData.crc;
		fileData.crc = 0;
		fileData.crc = crc8_cal((uint8_t*)&fileData, sizeof(fileData));
   	QL_MQTT_LOG("-1-PUBLISHLOG:my_log_fspayLoad Befor index %d crc_temp =%d , fileData.crc =%d  ,ts =%ld",index,crc_temp,fileData.crc,fileData.ts);
     QL_MQTT_LOG("-1-PUBLISHLOG:fsPayloadUpload state=%d,dataLength=%d,uploadStatus=%d,ts=%ld fsDataUpdateUploadState crc=%d ",fileData.state,fileData.dataLength,fileData.uploadStatus,fileData.ts,fileData.crc);
		if ((crc_temp !=0)&&(fileData.crc!=0)){
    if (crc_temp == fileData.crc)
    {
       for(int i=0;i<fileData.dataLength;i++)
           QL_MQTT_LOG("-1-PUBLISHLOG:fsmqtt fs crc data %x  ,index = %d",fileData.data[i],i);
 
        QL_MQTT_LOG("-1-PUBLISHLOG:my_log_fspayLoad pkt_ts=%ld ,from_ts=%ld,to_ts=%ld index %d crc_temp =%d , fileData.crc =%d",fileData.ts,fromTime,toTime,index,crc_temp,fileData.crc);
			  if((fileData.ts>=fromTime)&&(fileData.ts<=toTime)){
				memset(mqttSendData.data,0,sizeof(mqttSendData.data));
				memcpy(mqttSendData.data,fileData.data,fileData.dataLength);
     		mqttSendData.dataSize=fileData.dataLength;
				mqttSendData.topic=1;
				mqttSendData.mqttFsInfo.fsUpdateRequired=2;
        QL_MQTT_LOG("-1-PUBLISHLOG:my_log_fspayLoad pkt_ts Inside");
         for(int i=0;i<fileData.dataLength;i++)
           QL_MQTT_LOG("-1-PUBLISHLOG:mfsmqtt fs crc data %x  ,index = %d",mqttSendData.data[i],i);
        ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 *)&mqttSendData,0);
        ql_rtos_task_sleep_s(5);
			  
			}
		}	
   }
	}
  
}
