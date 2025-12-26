#include "ilmCommon.h"
#include "fsDriver.h"

//DEFINES
#define DEFAULT_BROKER_PORT 9114//1883///9114//1883// 9114///1883 ///1883
#define DEFAULT_BROKER_HOST_NAME  "iotpro.io"//"schnelliot.in"///"schnelliot.in"////"schnelliot.in"//"iotpro.io"// // "mqtt://schnelliot.in:1883"

#define DEFAULT_LAMP_CONTROL slot_time // manual//slot_time
#define DEFAULT_PACKET_COUNTER_PER_DAY 12000
#define DEFAULT_PUBLISH_INTERVAL 7200
#define BATTERY_SHUT_DELAY 600//600

// #define DEFAULT_SLOT_1_TIME 21600 // Local :- 6:00 AM            
// #define DEFAULT_SLOT_1_BRIGHTNESS 65
// #define DEFAULT_SLOT_2_TIME 64800 // Local :- 6:00 PM           
// #define DEFAULT_SLOT_2_BRIGHTNESS 100
// #define DEFAULT_SLOT_3_TIME 75600 // Local :- 9:00 PM           
// #define DEFAULT_SLOT_3_BRIGHTNESS 65
// #define DEFAULT_SLOT_4_TIME 82800 // Local :- 11:00 PM           
// #define DEFAULT_SLOT_4_BRIGHTNESS 40
// #define DEFAULT_SLOT_5_TIME 3600 // Local :- 1:00 AM            
// #define DEFAULT_SLOT_5_BRIGHTNESS 40
// #define DEFAULT_SLOT_6_TIME 14400 // Local :- 4:00 AM           
// #define DEFAULT_SLOT_6_BRIGHTNESS 65
#define DEFAULT_SLOT_1_TIME                        1800      // Local :- 6:00 AM            
#define DEFAULT_SLOT_1_BRIGHTNESS                  65
#define DEFAULT_SLOT_2_TIME                        45000      // Local :- 6:00 PM           
#define DEFAULT_SLOT_2_BRIGHTNESS                  100
#define DEFAULT_SLOT_3_TIME                        55800      // Local :- 9:00 PM           
#define DEFAULT_SLOT_3_BRIGHTNESS                  65
#define DEFAULT_SLOT_4_TIME                        63000      // Local :- 11:00 PM           
#define DEFAULT_SLOT_4_BRIGHTNESS                  40
#define DEFAULT_SLOT_5_TIME                        70200      // Local :- 1:00 AM            
#define DEFAULT_SLOT_5_BRIGHTNESS                  40
#define DEFAULT_SLOT_6_TIME                        81000      // Local :- 4:00 AM           
#define DEFAULT_SLOT_6_BRIGHTNESS                  65


#define DEFAULT_FEEDBACK_NO_LOAD_WATT 9000 // milliWatt 4000ul = 4 Watt
#define DEFAULT_FEEDBACK_MIN_LOAD_WATT 14000
#define NO_LOAD_CURRENT 20//#define NO_LOAD_CURRENT 50 // DEFAULT_FEEDBACK_NO_LOAD_WATT / 230           // This is to set 0 on relay off.
#define MIN_LOAD_CURRENT 20//#define MIN_LOAD_CURRENT 50 // DEFAULT_FEEDBACK_MIN_LOAD_WATT / 230          // Checking the lamp fault

#define DEFAULT_THRESHOLD_VOLT_MAX 300000 // 270.00
#define DEFAULT_THRESHOLD_VOLT_MIN 110000 // 180.00
#define DEFAULT_THRESHOLD_CURRENT_MAX 1500 // 1500// 2.000
#define DEFAULT_THRESHOLD_CURRENT_MIN 40 //  25//0.010
//#define DEFAULT_THRESHOLD_PF_MAX            // 2
#define DEFAULT_THRESHOLD_PF_MIN 80 //  0
#define DEFAULT_THRESHOLD_WATT_MAX 400000 // 500
#define DEFAULT_THRESHOLD_WATT_MIN 120000//40000//120000 // 8
#define DEFAULT_APN "www"

// #define DEFAULT_LDR_MILLI_VOLT_MAX                 2000
// #define DEFAULT_LDR_MILLI_VOLT_MIN                 1000

//GLOBALS AND STATICS
QFILE dataFile;
char ilmConfigFs[] = "UFS:cred.txt";
char ilmConfigFsBackUp[] = "UFS:credBaukUp.txt";
char dataLogFile[] = "UFS:dataLogds.txt";
char ilmResetLog[] = "UFS:Reset.txt";
char ilmAstroLog[] = "UFS:Astro.txt";
char ilmAstroLogBackUp[]="UFS:AstroBackUp.txt";
ilmAstro_t ilmAstro;
ilmReset_t ilmReset;
fsDataBackUp_t fileDataUploadStateBu;
//EXTERNS
extern ql_mutex_t dataFileMutex;
extern ilm_t ilm;
extern const char ifirmver[17];

U1 fsDataRead(fsDataBackUp_t * fileData, U2 fileIndex);
U1 fsDataWrite(QFILE * fd, U1 * data, U2 dataLength, U2 * currentFileIndex, U4 ts);
void fsDataUpdateUploadState(U2 currentFileIndex);
U2 fileReadDu(QFILE * fd, fsDataBackUp_t * fileData);



uint8_t crc8_cal(uint8_t * data, uint16_t length) {
  uint8_t crc = 0x00;

  uint8_t extract;
  uint8_t sum;

  for (uint16_t i = 0; i < length; i++) {
    extract = * data;
    for (uint8_t tempI = 8; tempI; tempI--) {
      sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum)
        crc ^= 0x8C;
      extract >>= 1;
    }
    data++;
  }

  return crc;
}
//===================resetFile===========================================

void nvmWriteResetFile(void) {
  int ilmResetFile, err;
  ilmReset.crc = 0;
  ilmReset.crc = crc8_cal((uint8_t * ) & ilmReset, sizeof(ilmReset));
  QL_MQTT_LOG("-1-MEMLOG: WriteIlmConfig  CRC VALUE =%u", ilmReset.crc);
  ilmResetFile = ql_fopen(ilmResetLog, "wb+");
  err = ql_fwrite( & ilmReset, sizeof(ilmReset), 1, ilmResetFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:nvmWriteResetFile write file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:nvmWriteResetFile write file pass");
  }
  ql_fclose(ilmResetFile);
  QL_MQTT_LOG("-1-MEMLOG:nvmWriteResetFile CRC CALCULATED=%u", ilmReset.crc);
}



void nvmReadResetFile(void) {
  //uint8_t crc_temp;
  int ilmResetFile, err;
  memset( & ilmReset, 0, sizeof(ilmReset_t));
  ilmResetFile = ql_fopen(ilmResetLog, "rb");
  err = ql_fread( & ilmReset, sizeof(ilmReset), 1, ilmResetFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:nvmReadResetFile read file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:nvmReadResetFile read file pass");
  }
    ql_fclose(ilmResetFile);
}
//=========================asrofile=================================================

void nvmWriteAstroFile(void) {
  int ilmAstroFile, err;
  ilmAstro.crc = 0;
  ilmAstro.crc = crc8_cal((uint8_t * ) & ilmAstro, sizeof(ilmAstro));
  QL_MQTT_LOG("-1-MEMLOG:asrofile WriteAstroConfig  CRC VALUE =%u", ilmAstro.crc);
  ilmAstroFile = ql_fopen(ilmAstroLog, "wb+");
  err = ql_fwrite( & ilmAstro, sizeof(ilmAstro), 1, ilmAstroFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:asrofilenvmWriteAstroFile write file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:asrofilenvmWriteAstroFile write file pass");
  }
  ql_fclose(ilmAstroFile);
  QL_MQTT_LOG("-1-MEMLOG:asrofilenvmWriteAstroFile CRC CALCULATED=%u", ilmAstro.crc);
}

void nvmWriteAstroFileBackup(void) {
  int ilmAstroFile, err;
  ilmAstro.crc = 0;
  ilmAstro.crc = crc8_cal((uint8_t * ) & ilmAstro, sizeof(ilmAstro));
  QL_MQTT_LOG("-1-MEMLOG:asrofile WriteAstroConfig  CRC VALUE =%u", ilmAstro.crc);
  ilmAstroFile = ql_fopen(ilmAstroLogBackUp, "wb+");
  err = ql_fwrite( & ilmAstro, sizeof(ilmAstro), 1, ilmAstroFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:asrofilenvmWriteAstroFileBackup write file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:asrofilenvmWriteAstroFileBackup write file pass");
  }
  ql_fclose(ilmAstroFile);
  QL_MQTT_LOG("-1-MEMLOG:asrofilenvmWriteAstroFileBackup CRC CALCULATED=%u", ilmAstro.crc);
}

void nvmWrireAstroFileWithBackup(){
 nvmWriteAstroFileBackup();
 nvmWriteAstroFile();

}



int nvmReadAstroBackUpFile(void) {
  uint8_t crc_temp;
  int ilmAstroFile, err;
  memset( & ilmAstro, 0, sizeof(ilmAstro_t));
  ilmAstroFile = ql_fopen(ilmAstroLogBackUp, "rb");
  err = ql_fread( & ilmAstro, sizeof(ilmAstro), 1, ilmAstroFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:asrofile nvmReadAstroBackUpFile read file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:asrofile nvmReadAstroBackUpFile read file pass");
  }
    ql_fclose(ilmAstroFile);
   crc_temp = ilmAstro.crc;
   ilmAstro.crc = 0;
   ilmAstro.crc = crc8_cal((uint8_t * ) & ilmAstro, sizeof(ilmAstro_t));
    if(crc_temp != ilmAstro.crc){
    return 1;
    }
  
  return 0;
 
}

int nvmReadAstroFile(void) {
  uint8_t crc_temp;
  int ilmAstroFile, err;
  memset( & ilmAstro, 0, sizeof(ilmAstro_t));
  ilmAstroFile = ql_fopen(ilmAstroLog, "rb");
  err = ql_fread( & ilmAstro, sizeof(ilmAstro), 1, ilmAstroFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:asrofile nvmReadAstroFile read file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:asrofile nvmReadAstroFile read file pass");
  }
    ql_fclose(ilmAstroFile);
  crc_temp = ilmAstro.crc;
  ilmAstro.crc = 0;
  ilmAstro.crc = crc8_cal((uint8_t * ) & ilmAstro, sizeof(ilmAstro_t));
   if(crc_temp != ilmAstro.crc){
    return 1;
   }
  
  return 0;
}
void loadDefualtAstroFile(){
  ilmAstro.astroOnOffset=1;
 ilmAstro.astroOffOffset=1;
 ilmAstro.astroLatManual=22.5744;
 ilmAstro.astroLongManual=88.3629;
 ilmAstro.astroLatAuto=22.5744;
 ilmAstro.astroLongAuto=88.3629;
 ilmAstro.astroSlot_on_time=0;
 ilmAstro.astroSlot_off_time=0;
  ilmAstro.storedDay=50;
 ilmAstro.firstBoot=0x77;

 nvmWrireAstroFileWithBackup();
QL_MQTT_LOG("-1-MEMLOG:asrofile loadDefualtAstroFile");

}

void nvmReadAstroFileWithBackup(){
    if(nvmReadAstroFile()){
      QL_MQTT_LOG("-1-MEMLOG:asrofile nvmReadAstroFile crc fail");
	   if(nvmReadAstroBackUpFile()){
       QL_MQTT_LOG("-1-MEMLOG:asrofile nvmReadAstroBackUpFile crc fail");
	     loadDefualtAstroFile();
     }
	}
}

/*
void nvmReadAstroFile(void) {
  //uint8_t crc_temp;
  int ilmAstroFile, err;
  memset( & ilmAstro, 0, sizeof(ilmAstro_t));
  ilmAstroFile = ql_fopen(ilmAstroLog, "rb");
  err = ql_fread( & ilmAstro, sizeof(ilmAstro), 1, ilmAstroFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:nvmReadAstroFile read file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:nvmReadAstroFile read file pass");
  }
    ql_fclose(ilmAstroFile);
}*/
//=======================================================================
void loadIlmDefaultConfig(void) {
  QL_MQTT_LOG("-1-MEMLOG:LOAD ILM DEFAULT CONFIG \n");

  //DEFAULTS
  static
  const char * broker_host_name = DEFAULT_BROKER_HOST_NAME;
  memcpy(ilm.ilmConfig.server.host, broker_host_name, strlen(DEFAULT_BROKER_HOST_NAME));
  ilm.ilmConfig.server.port = DEFAULT_BROKER_PORT;
  ilm.ilmConfig.mode = DEFAULT_LAMP_CONTROL;
  ilm.ilmConfig.maxPayload = DEFAULT_PACKET_COUNTER_PER_DAY;
  ilm.ilmConfig.periodicInterval = DEFAULT_PUBLISH_INTERVAL;
  ilm.ilmConfig.batKillDelay = BATTERY_SHUT_DELAY;

  //SLOT
  ilm.ilmConfig.slot.brightness[0] = DEFAULT_SLOT_1_BRIGHTNESS;
  ilm.ilmConfig.slot.timeInSecs[0] = DEFAULT_SLOT_1_TIME;

  ilm.ilmConfig.slot.brightness[1] = DEFAULT_SLOT_2_BRIGHTNESS;
  ilm.ilmConfig.slot.timeInSecs[1] = DEFAULT_SLOT_2_TIME;

  ilm.ilmConfig.slot.brightness[2] = DEFAULT_SLOT_3_BRIGHTNESS;
  ilm.ilmConfig.slot.timeInSecs[2] = DEFAULT_SLOT_3_TIME;

  ilm.ilmConfig.slot.brightness[3] = DEFAULT_SLOT_4_BRIGHTNESS;
  ilm.ilmConfig.slot.timeInSecs[3] = DEFAULT_SLOT_4_TIME;

  ilm.ilmConfig.slot.brightness[4] = DEFAULT_SLOT_5_BRIGHTNESS;
  ilm.ilmConfig.slot.timeInSecs[4] = DEFAULT_SLOT_5_TIME;

  ilm.ilmConfig.slot.brightness[5] = DEFAULT_SLOT_6_BRIGHTNESS;
  ilm.ilmConfig.slot.timeInSecs[5] = DEFAULT_SLOT_6_TIME;

  //THRESHOLD
  ilm.ilmConfig.acThresholds.minVolt = DEFAULT_THRESHOLD_VOLT_MIN;
  ilm.ilmConfig.acThresholds.maxVolt = DEFAULT_THRESHOLD_VOLT_MAX;
  ilm.ilmConfig.acThresholds.minCurrent = DEFAULT_THRESHOLD_CURRENT_MIN;
  ilm.ilmConfig.acThresholds.maxCurrent = DEFAULT_THRESHOLD_CURRENT_MAX;
  ilm.ilmConfig.acThresholds.minWatt = DEFAULT_THRESHOLD_WATT_MIN;
  ilm.ilmConfig.acThresholds.maxWatt = DEFAULT_THRESHOLD_WATT_MAX;
  ilm.ilmConfig.acThresholds.noLoad = DEFAULT_FEEDBACK_NO_LOAD_WATT;
  ilm.ilmConfig.acThresholds.minLoad = DEFAULT_FEEDBACK_MIN_LOAD_WATT;
  ilm.ilmConfig.acThresholds.minPowerFactor = DEFAULT_THRESHOLD_PF_MIN;
  strcpy(ilm.ilmConfig.ilmProjAttributes.firmwareVersion, ifirmver);
  ilm.lamp.burnTime = 0;
  //ilm.lamp.wh = 0;
  ilm.ilmConfig.firstBoot = 0x55;
  ilm.ilmConfig.ilmProjAttributes.boardNumber[0] = 0;
  memcpy(ilm.ilmConfig.apn,DEFAULT_APN,strlen(DEFAULT_APN));
  //powerFailTsOnPacket= ilm.powerFRTs.powerFailTs;
  // //LDR THRSHOLDS
  // ilm.ilmConfig.ldrThresholds.ldrMin=DEFAULT_LDR_MILLI_VOLT_MIN;
  // ilm.ilmConfig.ldrThresholds.ldrMax=DEFAULT_LDR_MILLI_VOLT_MAX;
}


void nvmWriteIlmConfigBackUp(void) {
  int ilmConfigFile, err;
  ilm.crc = 0;
  ilm.crc = crc8_cal((uint8_t * ) & ilm, sizeof(ilm));
  QL_MQTT_LOG("-1-MEMLOG: WriteIlmConfig  CRC VALUE =%u", ilm.crc);
  ilmConfigFile = ql_fopen(ilmConfigFsBackUp, "wb+");
  err = ql_fwrite( & ilm, sizeof(ilm), 1, ilmConfigFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:nvmWriteIlmConfigBackUp write file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:nvmWriteIlmConfigBackUp write file pass");
  }
  ql_fclose(ilmConfigFile);
  QL_MQTT_LOG("-1-MEMLOG:nvmWriteIlmConfigBackUp CRC CALCULATED=%u", ilm.crc);
}


void nvmWriteIlmConfig(void) {
  int ilmConfigFile;
  ilm.crc = 0;
  ilm.crc = crc8_cal((uint8_t * ) & ilm, sizeof(ilm));
  QL_MQTT_LOG("-1-MEMLOG:WriteIlmConfig  CRC VALUE =%u", ilm.crc);
  ilmConfigFile = ql_fopen(ilmConfigFs, "wb+");
  ql_fwrite( & ilm, sizeof(ilm), 1, ilmConfigFile);
  ql_fclose(ilmConfigFile);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain maximum payload %lu", ilm.ilmConfig.maxPayload);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain periodicInterval %lu", ilm.ilmConfig.periodicInterval);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain batKillDelay %lu", ilm.ilmConfig.batKillDelay);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain volt min %lu", ilm.ilmConfig.acThresholds.minVolt);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain volt max %lu", ilm.ilmConfig.acThresholds.maxVolt);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain min current %lu", ilm.ilmConfig.acThresholds.minCurrent);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain max current %lu", ilm.ilmConfig.acThresholds.maxCurrent);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain min watt %lu", ilm.ilmConfig.acThresholds.minWatt);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain max watt %lu", ilm.ilmConfig.acThresholds.maxWatt);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain no load %lu", ilm.ilmConfig.acThresholds.noLoad);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain min load %lu", ilm.ilmConfig.acThresholds.minLoad);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain ldr min %ld", ilm.ilmConfig.ldrThresholds.ldrMin);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain ldr max %ld", ilm.ilmConfig.ldrThresholds.ldrMax);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 6 brightness is %ld", ilm.ilmConfig.slot.brightness[0]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 6 time is %ld", ilm.ilmConfig.slot.timeInSecs[0]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 1 brightness is %ld", ilm.ilmConfig.slot.brightness[1]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 1 time is %ld", ilm.ilmConfig.slot.timeInSecs[1]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 2 brightness is %ld", ilm.ilmConfig.slot.brightness[2]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 2 time is %ld", ilm.ilmConfig.slot.timeInSecs[2]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 3 brightness is %ld", ilm.ilmConfig.slot.brightness[3]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 3 time is %ld", ilm.ilmConfig.slot.timeInSecs[3]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 4 brightness is %ld", ilm.ilmConfig.slot.brightness[4]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 4 time is %ld", ilm.ilmConfig.slot.timeInSecs[4]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 5 brightness is %ld", ilm.ilmConfig.slot.brightness[5]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain Slot 5 time is %ld", ilm.ilmConfig.slot.timeInSecs[5]);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain modeis %x", ilm.ilmConfig.mode);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain currentPerDayPayload is %ld", ilm.currentPerDayPayload);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain lamp.state is %x", ilm.lamp.state);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain powerFailTs is %ld", ilm.powerFRTs.powerFailTs);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain powerRetainTs is %ld", ilm.powerFRTs.powerRetainTs);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain server host %s", ilm.ilmConfig.server.host);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain server port %ld", ilm.ilmConfig.server.port);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain board number %s", ilm.ilmConfig.ilmProjAttributes.boardNumber);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain firmwareversion %s", ilm.ilmConfig.ilmProjAttributes.firmwareVersion);
  QL_MQTT_LOG("-1-MEMLOG: write pub snl_conf_write.domain burntime %ld", ilm.lamp.burnTime);
}

void nvmReadIlmConfigBackUp(void) {
  uint8_t crc_temp;
  int ilmConfigFile, err;
  memset( & ilm, 0, sizeof(ilm_t));
  ilmConfigFile = ql_fopen(ilmConfigFsBackUp, "rb");
  err = ql_fread( & ilm, sizeof(ilm), 1, ilmConfigFile);
  if (err < 0) {
    QL_MQTT_LOG("-1-MEMLOG:nvmReadIlmConfigBackUp read file failed ");

  } else {
    QL_MQTT_LOG("-1-MEMLOG:nvmReadIlmConfigBackUp read file pass");
  }

  ql_fclose(ilmConfigFile);
  crc_temp = ilm.crc;
  ilm.crc = 0;
  ilm.crc = crc8_cal((uint8_t * ) & ilm, sizeof(ilm_t));
  QL_MQTT_LOG("-1-MEMLOG: nvmReadIlmConfigBackUp CRC CALCULATED=%u  ,ILM CRC READ= %u", ilm.crc, crc_temp);
  if ((crc_temp != ilm.crc)) {
    loadIlmDefaultConfig();
    nvmWriteIlmConfig();
  }
}

void nvmReadIlmConfig(void) {
  uint8_t crc_temp;
  int ilmConfigFile;
  ilmConfigFile = ql_fopen(ilmConfigFs, "rb");
  ql_fread( & ilm, sizeof(ilm), 1, ilmConfigFile);
  ql_fclose(ilmConfigFile);
  crc_temp = ilm.crc;
  ilm.crc = 0;
  ilm.crc = crc8_cal((uint8_t * ) & ilm, sizeof(ilm_t));
  QL_MQTT_LOG("-1-MEMLOG: readIlmConfig CRC CALCULATED=%u  ,ILM CRC READ= %u", ilm.crc, crc_temp);
  if ((crc_temp != ilm.crc) || ((ilm.ilmConfig.firstBoot != 0x55))) {
    if ((crc_temp != ilm.crc)||((crc_temp == 0)&&(0 == ilm.crc)))
      nvmReadIlmConfigBackUp();
    if ((ilm.ilmConfig.firstBoot != 0x55)) {
      loadIlmDefaultConfig();
      nvmWriteIlmConfig();
    }
  }
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain maximum payload %lu", ilm.ilmConfig.maxPayload);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain periodicInterval %lu", ilm.ilmConfig.periodicInterval);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain batKillDelay %lu", ilm.ilmConfig.batKillDelay);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain volt min %lu", ilm.ilmConfig.acThresholds.minVolt);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain volt max %lu", ilm.ilmConfig.acThresholds.maxVolt);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain min current %lu", ilm.ilmConfig.acThresholds.minCurrent);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain max current %lu", ilm.ilmConfig.acThresholds.maxCurrent);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain min watt %lu", ilm.ilmConfig.acThresholds.minWatt);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain max watt %lu", ilm.ilmConfig.acThresholds.maxWatt);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain no load %lu", ilm.ilmConfig.acThresholds.noLoad);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain min load %lu", ilm.ilmConfig.acThresholds.minLoad);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain ldr min %ld", ilm.ilmConfig.ldrThresholds.ldrMin);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain ldr max %ld", ilm.ilmConfig.ldrThresholds.ldrMax);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 6 brightness is %ld", ilm.ilmConfig.slot.brightness[0]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 6 time is %ld", ilm.ilmConfig.slot.timeInSecs[0]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 1 brightness is %ld", ilm.ilmConfig.slot.brightness[1]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 1 time is %ld", ilm.ilmConfig.slot.timeInSecs[1]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 2 brightness is %ld", ilm.ilmConfig.slot.brightness[2]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 2 time is %ld", ilm.ilmConfig.slot.timeInSecs[2]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 3 brightness is %ld", ilm.ilmConfig.slot.brightness[3]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 3 time is %ld", ilm.ilmConfig.slot.timeInSecs[3]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 4 brightness is %ld", ilm.ilmConfig.slot.brightness[4]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 4 time is %ld", ilm.ilmConfig.slot.timeInSecs[4]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 5 brightness is %ld", ilm.ilmConfig.slot.brightness[5]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain Slot 5 time is %ld", ilm.ilmConfig.slot.timeInSecs[5]);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain modeis %x", ilm.ilmConfig.mode);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain currentPerDayPayload is %ld", ilm.currentPerDayPayload);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain lamp.state is %x", ilm.lamp.state);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain powerFailTs is %ld", ilm.powerFRTs.powerFailTs);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain powerRetainTs is %ld", ilm.powerFRTs.powerRetainTs);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain server host %s", ilm.ilmConfig.server.host);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain server port %ld", ilm.ilmConfig.server.port);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain board number %s", ilm.ilmConfig.ilmProjAttributes.boardNumber);
  QL_MQTT_LOG("-1-MEMLOG:my file read pub snl_conf_read.domain firmwareversion %s", ilm.ilmConfig.ilmProjAttributes.firmwareVersion);
}



U2 fileReadDu(QFILE * fd, fsDataBackUp_t * fileData) {
  U2 readState;
  U2 fileIndex = 0;
  QFILE dataFile;
  dataFile = ql_fopen(dataLogFile, "ab+");
  ql_fclose(dataFile);
  dataFile = ql_fopen(dataLogFile, "rb+");
  QL_MQTT_LOG("-1-MEMLOG:initial FilePosition- Index %d", ql_ftell(dataFile));
  for (fileIndex = 0; fileIndex < D_MAX_FILEINDEX; fileIndex++) {
    memset(fileData -> data, 0, 256);
    readState = fileRead( & dataFile, fileIndex, fileData);
    QL_MQTT_LOG("-1-MEMLOG:FilePosition- Index %d  In %d", ql_ftell(dataFile), fileIndex);
    if (!readState) {
      QL_MQTT_LOG("read");
      break;
    }
    if (fileData -> state != D_FILE_INDEXUSED) {
      QL_MQTT_LOG("-1-MEMLOG:inside fileData->state");
      ql_fclose(dataFile);
      return fileIndex;
    }
  }
  ql_fclose(dataFile);
  return fileIndex;
}

void fsDataUpdateUploadState(U2 currentFileIndex) {
  QFILE dataFile;
  fsDataBackUp_t fileData;
  uint8_t crc_temp;
  fsDataRead( & fileData, currentFileIndex);
  crc_temp = fileData.crc;
  fileData.crc = 0;
  fileData.crc = crc8_cal((uint8_t * ) & fileData, sizeof(fileData));
  QL_MQTT_LOG("-1-MEMLOG:my_log_fspayLoad crc=%d ,%d", crc_temp, fileData.crc);
  if (crc_temp == fileData.crc) {
    QL_MQTT_LOG("-1-MEMLOG:MQTT STATE UPDATED %d", fileData.uploadStatus);
    fileData.uploadStatus = D_UPLOADED;
    fileData.crc = 0;
    fileData.crc = crc8_cal((uint8_t * ) & fileData, sizeof(fileData));
    QL_MQTT_LOG("-1-MEMLOG:fsDataUpdateUploadState state=%d,dataLength=%d,uploadStatus=%d,ts=%ld fsDataUpdateUploadState crc=%d ", fileData.state, fileData.dataLength, fileData.uploadStatus, fileData.ts, fileData.crc);
    fileDataUploadStateBu = fileData;
    dataFile = ql_fopen(dataLogFile, "ab+");
    fileWrite( & dataFile, currentFileIndex, & fileData);
    ql_fclose(dataFile);
  }
}

U1 fsDataWrite(QFILE * fd, U1 * data, U2 dataLength, U2 * currentFileIndex, U4 ts) {
  U1 status;
  fsDataBackUp_t fileData;
  QFILE dataFile;
  memset(fileData.data, 0, 256);
  memcpy(fileData.data, data, dataLength);
  fileData.dataLength = dataLength;
  fileData.state = D_FILE_INDEXUSED;
  fileData.uploadStatus = D_NOTUPLOADED;
  fileData.ts = ts;
  fileData.crc = 0;
  for (int i = 0; i < fileData.dataLength; i++)
    QL_MQTT_LOG("-1-MEMLOG:mqtt fs crc data %x  ,index = %d", fileData.data[i], i);

  fileData.crc = crc8_cal((uint8_t * ) & fileData, sizeof(fileData));
  //dataFile = ql_fopen(dataLogFile, "ab+");
  dataFile = ql_fopen(dataLogFile, "ab+");
  if (dataFile < 0) {
    QL_MQTT_LOG("-1-MEMLOG:fsDataWrite open file failed %d", * currentFileIndex);
  }
  for (int i = 0; i < fileData.dataLength; i++)
    QL_MQTT_LOG("-1-MEMLOG:mqtt fs crc data %x  ,index = %d", fileData.data[i], i);

  if (fileWrite( & dataFile, * currentFileIndex, & fileData)) {
    QL_MQTT_LOG("-1-MEMLOG:fsDataWrite- FilePos %d  In %d a", ql_ftell(dataFile), * currentFileIndex);
    * currentFileIndex = ( * currentFileIndex >= D_MAX_FILEINDEX) ? (0) : ( * currentFileIndex + 1);
    QL_MQTT_LOG("-1-MEMLOG:fsDataWrite- FilePos %d  In %d b", ql_ftell(dataFile), * currentFileIndex);
    fileData.state = D_FILE_INDEXNOTUSED;
    status = fileWrite( & dataFile, * currentFileIndex, & fileData);
    QL_MQTT_LOG("-1-MEMLOG:fsDataWrite- FilePos %d  In %d c", ql_ftell(dataFile), * currentFileIndex);
    ql_fclose(dataFile);
    return status;
  }
  ql_fclose(dataFile);
  return 0;
}

U1 fsDataRead(fsDataBackUp_t * fileData, U2 fileIndex) {
  QFILE dataFile;
  U2 status;
  dataFile = ql_fopen(dataLogFile, "rb");
  status = fileRead( & dataFile, fileIndex, fileData);
  ql_fclose(dataFile);
  return status;
}