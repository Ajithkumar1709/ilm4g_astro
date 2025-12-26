#include "ilmCommon.h"
#include "powerFailHandler.h"
#include "memoryHandler.h"


#include"rpcHandler.h"
#include "ql_api_rtc.h"
#include"rtc.h"
#include"ql_gpio.h"
#include "fsDriver.h"
#include"faultChecker.h"
#include"rpcHandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ql_power.h"
#include "astronomy.h"

//DEFINES
#define LAMPACTION_NOTOK 0
#define LAMPACTION_LAMPON 1
#define LAMPACTION_LAMPOFF 2
#define LAMPACTION_LAMPDIM 3
#define SECONDS_PER_DAY 86400

//GLOBALS AND STATICS
U1 presentSlot = MAX_SLOT_SIZE;
U2 currentFileIndex = 0;
U1 lampBurnAndFaultCheck=false;
faults_t ilmFaults = {};
faultPkt_t faultPkt;
ilm_t ilm;
faults_t faults;

//EXTERNS
extern mqttQueue_t publish_data(U1 pktType,powerFRTs_t powerFRTs);
extern  mqttQueue_t publish_gps_data(void) ;
extern ql_mutex_t dataFileMutex ;
extern void uart_data_req(unsigned char * data);
extern unsigned char nuAcMeasCommand[];
extern unsigned char nuRlyOnCommand[] ;
extern unsigned char nuRlyOffCommand[];
extern unsigned char nuBatKill[];
extern void uart_init();
extern ql_queue_t ql_ilmProcess_qHandle;
extern ql_queue_t ql_mqttSend_queueHandle;
extern void acMeasure(void);
extern acParam_t  acParam;
extern uint8_t coControllerStatus;
extern U1 powerFailFlag;
extern U1 batKillEnable;
extern U1 acMeasureCheck();
extern U1 instaGetPowerFailSense(void);
extern U1 nuAckCheck();
extern U1 uartPacketRcd;
extern mqttQueue_t publish_config(sendConfig type);
extern U1 inBatteryModeCheck(void);
extern ilmAstro_t ilmAstro;
extern ilmReset_t ilmReset;
extern ql_rtc_time_t rtcTm;
//extern U1 batKillTimeCounter;
//extern U1 powerFailFlagCounter;
extern  U1 faultCounter[];
uint32_t randomTimeAstroSend;
uint8_t astroSendFlage=false;
uint32_t addingOffsetAndSlotTime(int32_t tmp){
  QL_MQTT_LOG("-1-ILMLOG:ilmAstro.astroSlotaddingOffsetAndSlotTimeStart %ld",tmp);
  
   if(tmp<0){
    tmp=86400+tmp;
   }
   else if(tmp>=86400){
    tmp=tmp-86400;
   }
  QL_MQTT_LOG("-1-ILMLOG:ilmAstro.astroSlotaddingOffsetAndSlotTimeEnd% ld",tmp);
   return (U4)tmp;
   }
void astroLoadOnOffTime(slot_t * slot){
int i;
for (i = 0; i < MAX_SLOT_SIZE ; i++){
//Off Time
if (slot -> timeInSecs[i] ==1800){
    slot -> timeInSecs[i]=addingOffsetAndSlotTime((int32_t)(ilmAstro.astroSlot_off_time+ilmAstro.astroOffOffset));
    //slot -> timeInSecs[i]=ilmAstro.astroSlot_off_time+ilmAstro.astroOffOffset;
      QL_MQTT_LOG("-1-ILMLOG:ilmAstro.astroSlot_off_time+ilmAstro.astroOffOffsetastroslot -> timeInSecs[%d] %u,",i,slot -> timeInSecs[i]);
  
}
//On Time
if (slot -> timeInSecs[i] ==45000){
    slot -> timeInSecs[i]= addingOffsetAndSlotTime((int32_t)(ilmAstro.astroSlot_on_time+ilmAstro.astroOnOffset));
   //slot -> timeInSecs[i]=ilmAstro.astroSlot_on_time+ilmAstro.astroOnOffset;
      QL_MQTT_LOG("-1-ILMLOG:ilmAstro.astroSlot_on_time+ilmAstro.astroOnOffsetastroslot -> timeInSecs[%d] %u,",i,slot -> timeInSecs[i]);
}

}
}
void lampStateUpdater(){
if((faultPkt.fd!=2)&&(faultPkt.fd!=128)){
  if( (ilm.lamp.relay==D_RELAY_ON)||(ilm.lamp.relay==D_RELAY_DIM))
      ilm.lamp.state=LAMP_ON;

    if(ilm.lamp.relay==D_RELAY_OFF)
      ilm.lamp.state=LAMP_OFF;
}

//if((faults.noLoadFault==true)||(faults.relayFault==true)){
  if((faultPkt.fd==2)||(faultPkt.fd==128)){
    ilm.lamp.state=LAMP_FAULT;
}
}
void faultCounterReset(void){
	faultCounter[voltMinFault]=(faultPkt.faults.voltMinFault==D_FAULT)?(3):(0);
	faultCounter[voltMaxFault]=(faultPkt.faults.voltMaxFault==D_FAULT)?(3):(0);
	faultCounter[currentMinFault]=(faultPkt.faults.currentMinFault==D_FAULT)?(3):(0);
	faultCounter[currentMaxFault]=(faultPkt.faults.currentMaxFault==D_FAULT)?(3):(0);
	faultCounter[wattMinFault]=(faultPkt.faults.wattMinFault==D_FAULT)?(3):(0);
	faultCounter[wattMaxFault]=(faultPkt.faults.wattMaxFault==D_FAULT)?(3):(0);
	faultCounter[noLoadFault]=(faultPkt.faults.noLoadFault==D_FAULT)?(3):(0);
	faultCounter[relayFault]=(faultPkt.faults.relayFault==D_FAULT)?(3):(0);
	faultCounter[lowPowerFactorFault]=(faultPkt.faults.lowPowerFactorFault==D_FAULT)?(3):(0);
}
uint32_t daysInSec(astro_utc_t utc){
 int day_hour, day_min, day_sec = 0;
 uint32_t day_Totsec=0;
 day_hour = utc.hour * 3600;
  day_min = utc.minute * 60;
  day_sec = utc.second;
  day_Totsec = day_hour + day_min + day_sec;
  return day_Totsec;
}
void astroLatLongAutoSet(void){
  gnssElements_t gnssElementsForAstro;
nvmReadAstroFileWithBackup();
gnssElementsForAstro=gnss_data();
if(gnssElementsForAstro.quality==1){
ilmAstro.astroLatAuto=gnssElementsForAstro.latitude;
ilmAstro.astroLongAuto=gnssElementsForAstro.longitude;
}
nvmWrireAstroFileWithBackup();
}
uint8_t astroTime(double lat, double longn) {
  rtc_t rtc;
  uint8_t writeFlag = false;
  uint32_t astroSlot_on_time = 0, astroSlot_off_time = 0;
  astro_observer_t observer = {
    .latitude = lat,
    .longitude = longn,
    .height = 0
  };
  rtc = getRtc();
  if (rtc.status == RTC_SET) {
     QL_MQTT_LOG("-1-ILMLOG:astroTIME YEAR %d,MON %d,DAY %d,HOUR %d,MIN %d,SEC %d,",rtcTm.tm_year, rtcTm.tm_mon, rtcTm.tm_mday, rtcTm.tm_hour, rtcTm.tm_min, rtcTm.tm_sec);
   rtcTm.tm_hour=0;
   rtcTm.tm_min=0;
   rtcTm.tm_sec=0;
    astro_time_t time = Astronomy_MakeTime(rtcTm.tm_year, rtcTm.tm_mon, rtcTm.tm_mday, rtcTm.tm_hour, rtcTm.tm_min,rtcTm.tm_sec);
    astro_search_result_t sunrise = Astronomy_SearchRiseSet(BODY_SUN, observer, +1, time, 300.0);
    astro_search_result_t sunset = Astronomy_SearchRiseSet(BODY_SUN, observer, -1, time, 300.0);
    if (sunrise.status == ASTRO_SUCCESS && sunset.status == ASTRO_SUCCESS) {
      nvmReadAstroFileWithBackup();
      astro_utc_t utc;
      utc=Astronomy_UtcFromTime(sunset.time);
      QL_MQTT_LOG("-1-ILMLOG:astro sunset YEAR %d,MON %d,DAY %d,HOUR %d,MIN %d,SEC %f,",utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second);
      utc=Astronomy_UtcFromTime(sunrise.time);
      QL_MQTT_LOG("-1-ILMLOG:astro sunrise YEAR %d,MON %d,DAY %d,HOUR %d,MIN %d,SEC %f,",utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second);
     
      astroSlot_on_time = daysInSec(Astronomy_UtcFromTime(sunset.time));
      astroSlot_off_time = daysInSec(Astronomy_UtcFromTime(sunrise.time));
      if (ilmAstro.astroSlot_on_time != astroSlot_on_time) {
        ilmAstro.astroSlot_on_time = astroSlot_on_time;
        writeFlag = true;
      }
      if (ilmAstro.astroSlot_off_time != astroSlot_off_time) {
        ilmAstro.astroSlot_off_time = astroSlot_off_time;
        writeFlag = true;
      }
      if (writeFlag == true) {
        nvmWrireAstroFileWithBackup();
      }
      return 1;
    }
  }
  return 0;
}
U1 maxPayLoadChecker(rtc_t rtc) {
  QL_MQTT_LOG("-1-ILMLOG: ilm.maxpayload=%lu,ilm.currentPerDayPayload=%d,rtc.day=%d,ilm.currentDay=%d", ilm.ilmConfig.maxPayload, ilm.currentPerDayPayload, rtc.day, ilm.currentDay);
  if (rtc.status == RTC_SET) {
    U1 current_day = rtc.day;
    if (current_day != ilm.currentDay) {
      QL_MQTT_LOG("-1-ILMLOG:Payload Counter Reset\r\n");
      ilm.currentPerDayPayload = 0;
      ilm.currentDay = current_day;
      ilmReset.nextDayFlage = DONE;
       nvmWriteResetFile();
      if (!getPowerFailSense())
        nvmWriteIlmConfig();
    }
    if (ilm.currentPerDayPayload >= ilm.ilmConfig.maxPayload) {
      return 0;
    } else {
      ilm.currentPerDayPayload++;
      if (!getPowerFailSense())
        nvmWriteIlmConfig();
    }
  }
  return 1;
}

void lampBurnTimeRoutine(acParam_t acParam, rtc_t rtc) {
  static U1 doOnce = 0;
  static U4 currentTimeInSec = 0, previousTimeInSec;
  U4 timeDifference = 0;
  if (doOnce == 0) {
    previousTimeInSec = rtc.timeInSec;
    //cumulativeBurnTime=0;
    doOnce = 1;
  }
  currentTimeInSec = rtc.timeInSec;
  timeDifference = currentTimeInSec - previousTimeInSec;
  QL_MQTT_LOG("-1-ILMLOG:my_log_currentTimeInSec:%lu", currentTimeInSec);
  QL_MQTT_LOG("-1-ILMLOG:my_log_previousTimeInSec:%lu", previousTimeInSec);
  QL_MQTT_LOG("-1-ILMLOG:my_log_cumulativeBurnTime:%lu", ilm.lamp.burnTime);
  //Calculate the cumulative burn time
  //cumulativeBurnTime += (acParam.current>ilm.ilmConfig.acThresholds.minLoad)*timeDifference;
  ilm.lamp.burnTime += ((acParam.watt*1000) > ilm.ilmConfig.acThresholds.minLoad) * timeDifference;//537>9000*1

  //ilm.lamp.burnTime+=cumulativeBurnTime;

  //Update the previous time for the next iteration
  previousTimeInSec = currentTimeInSec;
}

U1 nulampOnCom(void) {
  U1 nuStatus,rtyCount=0;
  U1 timeout=0;
  do{
  acParam.gridStatus=instaGetPowerFailSense();
  uartPacketRcd=0;
  uart_data_req(nuRlyOnCommand);
  timeout=0;
   do{
   ql_rtos_task_sleep_ms(200);
   timeout++;
   }while((uartPacketRcd==false)&&(timeout<15));
 
  //ql_rtos_task_sleep_s(3);
  nuStatus=nuAckCheck();
  rtyCount++;
  if(nuStatus==D_COCONTROLLER_NOTOK){
    ql_rtos_task_sleep_s(1);
   }
  }while((nuStatus==D_COCONTROLLER_NOTOK)&&(rtyCount<3));

  return 1;
}

U1 nulampOffCom(void) {
  U1 nuStatus,rtyCount=0;
  U1 timeout=0;
  do{
  acParam.gridStatus=instaGetPowerFailSense();
  uartPacketRcd=0;
  uart_data_req(nuRlyOffCommand);
  timeout=0;
  do{
   ql_rtos_task_sleep_ms(200);
   timeout++;
  }while((uartPacketRcd==false)&&(timeout<15));
  //ql_rtos_task_sleep_s(3);
  nuStatus=nuAckCheck();
  rtyCount++;
if(nuStatus==D_COCONTROLLER_NOTOK){
    ql_rtos_task_sleep_s(1);
   }
  }while((nuStatus==D_COCONTROLLER_NOTOK)&&(rtyCount<3));

  return 1;
}

U1 nulampDimCom(U1 bri) {
  S1 dimCommand[50];
  U1 timeout=0;
  U1 nuStatus,rtyCount=0;
  do{
  sprintf(dimCommand, "$DIM%03d#", bri);
  acParam.gridStatus=instaGetPowerFailSense();
  uartPacketRcd=0;
  uart_data_req((unsigned char * ) dimCommand);
  timeout=0;
  do{
   ql_rtos_task_sleep_ms(200);
   timeout++;
  }while((uartPacketRcd==false)&&(timeout<15));
  //ql_rtos_task_sleep_s(3);
  nuStatus=nuAckCheck();
  rtyCount++;
  if(nuStatus==D_COCONTROLLER_NOTOK){
    ql_rtos_task_sleep_s(1);
  }
  }while((nuStatus==D_COCONTROLLER_NOTOK)&&(rtyCount<3));
  return 1;
}

U1 lampAction(U1 lamp, U1 bri) {
  //on
  QL_MQTT_LOG("-1-ILMLOG:my_log_nuvoton_lamp=%d,bri=%d", lamp, bri);
  //U1 status=0;
  if (lamp == 1) {
    nulampOnCom();
    ilm.lamp.relay = D_RELAY_ON;
    ilm.lamp.brightness = 100;
  //  ilm.lamp.state = LAMP_ON;
    QL_MQTT_LOG("-1-ILMLOG:my_log_nuvoton_on\n");
    return event_lamp_on;
  }
  //off
  if (lamp == 0) {
    nulampOffCom();
    ilm.lamp.relay = D_RELAY_OFF;
    ilm.lamp.brightness = 0;
   // ilm.lamp.state = LAMP_OFF;
    QL_MQTT_LOG("-1-ILMLOG:my_log_nuvoton_off\n");
    return event_lamp_off;
  }
  //dim
  if (lamp == 2) {
    if(bri>=100)
    bri=99;
    nulampDimCom(bri);
    ilm.lamp.relay = D_RELAY_DIM;
    ilm.lamp.brightness = bri;
  //  ilm.lamp.state = LAMP_ON;
    QL_MQTT_LOG("-1-ILMLOG:my_log_nuvoton_dim\n");
    return event_lamp_dim;
  }
  //return 1; //Ok Notok 
  return LAMPACTION_NOTOK;
}
//===========================================================================================
   uint8_t FindHigherSlot(slot_t slot){
   uint8_t higherBrightnessIndex=0;
    
    uint8_t i;
      for (i = 1; i < MAX_SLOT_SIZE; i++) {
        if (slot.brightness[i] > slot.brightness[higherBrightnessIndex]) {
            higherBrightnessIndex = i;
        }
      }
	  return higherBrightnessIndex;
   }
//==========================================================================================
slot_t  gmtDaySecToLocalDaySec(slot_t slot){
unsigned char i;
for(i=0;i<6;i++){
slot.timeInSecs[i]=slot.timeInSecs[i]+19800;
if(slot.timeInSecs[i]>=SECONDS_PER_DAY)
slot.timeInSecs[i]=slot.timeInSecs[i]-SECONDS_PER_DAY;
QL_MQTT_LOG("-1-ILMLOG:slot.timeInSecs=%ld\n",slot.timeInSecs[i]);
}
return slot;

}

U4 isInRange(U4 value, U4 min, U4 max) {
  return (value >= min) && (value < max);
}

U1 slotFinder(U4 rtcTime, slot_t * slot) {
  U1 slotIndex = 0;
  for (slotIndex = 0; slotIndex < MAX_SLOT_SIZE; slotIndex++) {
    QL_MQTT_LOG("-1-ILMLOG:rts :%ld,%ld,%ld", rtcTime, slot -> timeInSecs[slotIndex], slot -> timeInSecs[slotIndex + 1]);

    if (slotIndex < 5) {
      if (isInRange(rtcTime, slot -> timeInSecs[slotIndex], slot -> timeInSecs[slotIndex + 1])) {
        break;
      }
    } else if ((rtcTime >= slot -> timeInSecs[slotIndex]) || (rtcTime <= slot -> timeInSecs[0])) {
      break;
    }
  }
  return slotIndex;
}

//sort_slot_in_ascending_order
void slotSorter(slot_t * slot) {
  int i, j;
  uint32_t temp_time;
  uint8_t temp_brightness;
  for (i = 0; i < MAX_SLOT_SIZE - 1; i++) {
    for (j = 0; j < MAX_SLOT_SIZE - i - 1; j++) {
      if (slot -> timeInSecs[j] > slot -> timeInSecs[j + 1]) {
        // swap the timeInSecs values
        temp_time = slot -> timeInSecs[j];
        slot -> timeInSecs[j] = slot -> timeInSecs[j + 1];
        slot -> timeInSecs[j + 1] = temp_time;

        // swap the brightness values
        temp_brightness = slot -> brightness[j];
        slot -> brightness[j] = slot -> brightness[j + 1];
        slot -> brightness[j + 1] = temp_brightness;
      }
    }
  }
}

int slotAction(rtc_t rtc, slot_t ilmSlot) {
  slot_t slot = gmtDaySecToLocalDaySec(ilmSlot);
  uint32_t randomResetTime=0;
  
  U1 slotIndex = MAX_SLOT_SIZE;
  static U1 findedSlot = MAX_SLOT_SIZE;
  slotSorter( & slot);
  slotIndex = slotFinder(rtc.daySecs, & slot);
  QL_MQTT_LOG("-1-ILMLOG:my_log_rtc.daySesc:%ld", rtc.daySecs);
  QL_MQTT_LOG("-1-ILMLOG:my_log_slotIndexc:%d", slotIndex);
  QL_MQTT_LOG("-1-ILMLOG:my_log_presentSlot%d,findedSlot%d", presentSlot, findedSlot);
       QL_MQTT_LOG("-1-ILMLOG:1.FindHigherSlot(ilmSlot)=%d", FindHigherSlot(slot));
       for(int i=0;i<6;i++)
       QL_MQTT_LOG("-1-ILMLOG:astroslot.timeInSecs[%d]=%ld,brightness=%ld",i, slot.timeInSecs[i],slot.brightness[i]);
  QL_MQTT_LOG("-1-ILMLOG:astroTimeON %ld,astroTimeOFF %ld",ilmAstro.astroSlot_on_time,ilmAstro.astroSlot_off_time);
    QL_MQTT_LOG("-1-ILMLOG:astroTimeONOffset %ld,astroTimeOFFOffset %ld",ilmAstro.astroOnOffset,ilmAstro.astroOffOffset);
  if (slotIndex != MAX_SLOT_SIZE) {
    findedSlot = slotIndex;
    if (presentSlot != findedSlot) {
      presentSlot = findedSlot;
      nvmReadResetFile(); 
    if(FindHigherSlot(slot) ==presentSlot &&  ilmReset.nextDayFlage == DONE){
	      ilmReset.ResetFlage = DO_RESET;
		    nvmWriteResetFile();
		  
	  }
      //return lampAction((slot.brightness[slotIndex]==0)?(0):(2),slot.brightness[slotIndex]);
      return lampAction((slot.brightness[slotIndex] == 0) ? (0) : (slot.brightness[slotIndex] == 100) ? (1) : (2), slot.brightness[slotIndex]);
    }
    else {
       if (!getPowerFailSense()) {
          nvmReadResetFile();
       if (ilmReset.ResetFlage == DO_RESET) {
        srand(time(NULL));
        randomResetTime = rand() % 31;
       //  QL_MQTT_LOG("-1-ILMLOG:1.randomResetTime(ilmSlot)=%ld", randomResetTime);
       ql_rtos_task_sleep_s(randomResetTime);
       if (!getPowerFailSense()) {
         ilmReset.ResetFlage = NO_RESET;
         ilmReset.nextDayFlage = NOT_DONE;
         nvmWriteResetFile();
         nvmWriteIlmConfig();
         ql_delay_us(10000);
         ql_dev_set_modem_fun(1, 1, 0);
         }
        }
      }
   }
  }
  return -1;
}

void ilmProcess_thread(void * param) {
  powerFRTs_t powerFRTs;
  fsDataBackUp_t fileDataVar;
  QFILE dataFile;
  pubHandler_t pubHandler;
  mqttQueue_t mqttSendData,mqttDataPower;
  ilmQueue_t ilmData,ilmDataLoc;// ilmPowerFRData;
  int sendPkt;
  U1 ledInit;
  rtc_t rtc;
  faultPkt.fd = 0;
  ql_pwrup_reason pwup; 
 
  QL_MQTT_LOG("-1-PSTART:start_ilmProcess_Task");
  //STATUS LED CONFIGURABLE
  ql_pin_set_func(27, 0);
  ql_gpio_init(0, GPIO_OUTPUT, PULL_NONE, LVL_LOW);

  //power fail sense polling
  ql_pin_set_func(24, 0);
  ql_gpio_init(2, GPIO_INPUT, PULL_NONE, LVL_HIGH);

  //nuvoton reset pin
  ql_pin_set_func(26, 0);
  ql_gpio_init(1, GPIO_OUTPUT, PULL_NONE, LVL_HIGH); //nuvoton reset pin
  ql_delay_us(50000);
  ql_gpio_set_level(1, LVL_LOW);

  //power fail sense interrupt
  /*ql_pin_set_func(24, 0);
  ql_int_register(GPIO_2,EDGE_TRIGGER,DEBOUNCE_EN,EDGE_BOTH,PULL_NONE,_gpioint_callback01,NULL);
  ql_int_enable(GPIO_2);*/
  for (ledInit = 0; ledInit < 7; ledInit++) {
    ql_gpio_set_level(0, LVL_LOW);
    ql_delay_us(50000);
    ql_gpio_set_level(0, LVL_HIGH);
    ql_delay_us(50000);
  }

  nvmReadIlmConfig();
  nvmWriteIlmConfigBackUp();
  ql_rtos_mutex_lock((ql_mutex_t) dataFileMutex, QL_WAIT_FOREVER);
  currentFileIndex = fileReadDu( & dataFile, & fileDataVar);
  ql_rtos_mutex_unlock((ql_mutex_t) dataFileMutex);
//========================Reset file===============================================
nvmReadResetFile();
if(ilmReset.firstBoot!=0x77){
 ilmReset.ResetFlage=NO_RESET;
 ilmReset.nextDayFlage=DONE;
 ilmReset.firstBoot=0x77;
 nvmWriteResetFile();
 }
//=================================================================================
 //======================astro file==========================================
 //========================Astro file===============================================
nvmReadAstroFileWithBackup();
if(ilmAstro.firstBoot!=0x77){
loadDefualtAstroFile();
QL_MQTT_LOG("-1-ILMLOG:ilmAstro Default enter");
 }
//=================================================================================
  uart_init();
  ql_rtos_task_sleep_s(2);
  if ((ilm.ilmConfig.mode == slot_time)||(ilm.ilmConfig.mode == astro_Manual)||(ilm.ilmConfig.mode == astro_Auto)) {
    lampAction(D_RELAY_ON, 100);
  } else{
    lampAction(ilm.lamp.relay, ilm.lamp.brightness);
  }
 ql_get_powerup_reason(&pwup);
/*if(pwup==QL_PWRUP_CHARGE){//4 means software reboot if 1 means power on boot

} */
  memset(&faults,0,sizeof(faults_t));
  ql_rtos_queue_create( & ql_ilmProcess_qHandle, sizeof(ilmQueue_t), 5);
  while (1) {
     
    ilmData.dataSize = 0;
    ql_rtos_queue_wait(ql_ilmProcess_qHandle, (uint8 * ) & ilmData, sizeof(ilmQueue_t), 1000);
    QL_MQTT_LOG("-1-ILMLOG:my_log_outside_periodic ilm.ilmConfig.periodicInterval=%d", ilm.ilmConfig.periodicInterval);
    QL_MQTT_LOG("-1-ILMLOG:pwup=%d", pwup);
     QL_MQTT_LOG("-1-ILMLOG:1.ilmReset.ResetFlage=%d", ilmReset.ResetFlage);
      QL_MQTT_LOG("-1-ILMLOG:1.ilmReset.nextDayFlage=%d", ilmReset.nextDayFlage);

    QL_MQTT_LOG("-1-ILMLOG:my_log_outside_periodic ilm.ilmConfig.relay=%d", ilm.lamp.relay);
    QL_MQTT_LOG("-1-ILMLOG:ilm process currentFileIndex=%d", currentFileIndex);
    QL_MQTT_LOG("-1-ILMLOG:my file write pub snl_conf_write.domain burn time %lu", ilm.lamp.burnTime);
    QL_MQTT_LOG("-1-ILMLOG:my file write pub snl_conf_write.domain burn time %s", ilm.ilmConfig.apn);
      QL_MQTT_LOG("-1-ILMLOG:ilmAstro astroTimeON %ld,astroTimeOFF %ld",ilmAstro.astroSlot_on_time,ilmAstro.astroSlot_off_time);
    QL_MQTT_LOG("-1-ILMLOG:ilmAstro astroTimeONOffset %ld,astroTimeOFFOffset %ld",ilmAstro.astroOnOffset,ilmAstro.astroOffOffset);

     QL_MQTT_LOG("-1-ILMLOG:ilmAstro manual lat long %f,astroTimeOFF %f",ilmAstro.astroLatManual,ilmAstro.astroLongManual);
      QL_MQTT_LOG("-1-ILMLOG:ilmAstro auto lat long %f,astroTimeOFF %f",ilmAstro.astroLatAuto,ilmAstro.astroLongAuto);
      QL_MQTT_LOG("-1-ILMLOG:ilmAstro stored day %d",ilmAstro.storedDay);
      QL_MQTT_LOG("-1-ILMLOG:ilmAstro randomTimeAstroSend %u,astroSendFlage=%d",randomTimeAstroSend,astroSendFlage);
    sendPkt = D_SEND_NO_DATA;
    if (ilmData.dataSize) {
      switch (ilmData.pktType) {
      case D_POWER_RETAIN:
        QL_MQTT_LOG("-1-ILMLOG:my_log_init_power_retain_init1");
        sendPkt = event_power_retain;
        powerFRTs=ilmData.powerFRTs;
        break;
      case D_POWER_FAIL:
        QL_MQTT_LOG("-1-ILMLOG:my_log_init_power_fail_init1");
        sendPkt = event_battery_power_retain;
        break;
      case D_LAMP_BURN:
        lampBurnAndFaultCheck = true;
        QL_MQTT_LOG("-1-ILMLOG:my_log_D_LAMP_BURN");
        break;
      case D_GNSS_DPROCESS:
        sendPkt = event_gnss_packet;
        if(pwup==POWER_ON_REBOOT){
        astroLatLongAutoSet();
        ilmDataLoc.pktType=D_ASTRO;
        ilmDataLoc.dataSize=1;
        ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmDataLoc), (uint8 *)&ilmDataLoc, 0);
        }
        QL_MQTT_LOG("-1-ILMLOG:D_GNSS_PROCESS");
   
      break;
      case D_PERIODIC_INTERVAL:
        QL_MQTT_LOG("-1-ILMLOG:my_log_send_live");
        sendPkt = event_live;
        break;
      case D_RPC:
        //sendPkt=D_RPC;
        QL_MQTT_LOG("-1-ILMLOG:my_log_RPC_ok");
        pubHandler = pub_handler(ilmData.data, ilmData.dataSize);
        if (pubHandler.pktResponse) {
          sendPkt = pubHandler.sendPkt;
          if((sendPkt==event_lamp_on)||(sendPkt==event_lamp_off))
            ql_rtos_task_sleep_ms(500);
          if(sendPkt==event_lamp_dim)
            ql_rtos_task_sleep_s(1);
        }
        else{
            if (pubHandler.mqttSendData.dataSize)
            ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(pubHandler.mqttSendData), (uint8 * ) & pubHandler.mqttSendData, 0);
         }
         break;
      case D_ICCID:
      mqttSendData=publish_config(config_sendiccidPacket);
      if (mqttSendData.dataSize) {
              ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 * ) & mqttSendData, 0);
              QL_MQTT_LOG("-1-ILMLOG:D_ICCID");
            }
      break;
       case D_ASTRO:
      mqttSendData=publish_config(config_slotSend);
      if (mqttSendData.dataSize) {
              ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 * ) & mqttSendData, 0);
              QL_MQTT_LOG("-1-ILMLOG:D_ASTRO");
            }
      break;

      }
    } else {
      /*rtc = getRtc();
      ilmPowerFRData = powerFailSenseRoutine(rtc);
      if (ilmPowerFRData.dataSize) {
        sendPkt = ilmPowerFRData.pktType;
        powerFRTs=ilmPowerFRData.powerFRTs;

      }*/
      rtc = getRtc();
      mqttDataPower=powerFailSenseRoutine(rtc);
      if (mqttDataPower.dataSize){
          ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttDataPower), (uint8 * ) & mqttDataPower, 0);
      }
       
       else  {
        if (rtc.status == RTC_SET) {
            QL_MQTT_LOG("-1-ILMLOG:my_log_slot_mode %d",ilm.ilmConfig.mode);
        if (ilm.ilmConfig.mode == slot_time) {
          sendPkt = slotAction(rtc, ilm.ilmConfig.slot);
          if (sendPkt != D_SEND_NO_DATA) {
          
            ql_rtos_task_sleep_s(3);
          }
        }

  if ((ilm.ilmConfig.mode == astro_Manual) || (ilm.ilmConfig.mode == astro_Auto)) {
       nvmReadAstroFileWithBackup();
        if (ilm.ilmConfig.mode == astro_Manual)
         astroTime(ilmAstro.astroLatManual, ilmAstro.astroLongManual);
       else
         astroTime(ilmAstro.astroLatAuto, ilmAstro.astroLongAuto);

     if(rtcTm.tm_mday!= ilmAstro.storedDay){
        srand(time(NULL));
       uint16_t lower = 0;
       uint16_t upper = 120;
       randomTimeAstroSend = (rand() % (upper - lower + 1)) + lower; 
       randomTimeAstroSend+=rtc.daySecs;
        astroSendFlage=true;
		    astroLatLongAutoSet();

		   ilmAstro.storedDay=rtcTm.tm_mday;
       QL_MQTT_LOG("-1-ILMLOG:ilmAstro.storedDay");
		   nvmWrireAstroFileWithBackup();
       }
       slot_t slot = ilm.ilmConfig.slot;
       astroLoadOnOffTime(&slot);
       sendPkt = slotAction(rtc, slot);
      if (sendPkt != D_SEND_NO_DATA) {
         QL_MQTT_LOG("-1-ILMLOG:my_log_slot_mode");
         ql_rtos_task_sleep_s(3);
       }
     }
        }
      }
    }
   QL_MQTT_LOG("-1-ILMLOG:my_log before faultCheckRoutine");
    if ((lampBurnAndFaultCheck) || (sendPkt != D_SEND_NO_DATA)) {
    acMeasure();
    rtc = getRtc();
    if (rtc.status == RTC_SET) {
      lampBurnTimeRoutine(acParam, rtc);
      //FAULT CHECK
      if ((coControllerStatus == D_COCONTROLLER_OK)) {
        if((lampBurnAndFaultCheck)||(sendPkt != D_SEND_NO_DATA)){
         lampBurnAndFaultCheck = false;   
        //if (!((batKillEnable == true) && (getPowerFailSense() == true))) {
         if((powerFailFlag == false)&&(inBatteryModeCheck()==false)){
          faultPkt = faultCheckRoutine(acParam, ilmFaults, faultPkt);
           QL_MQTT_LOG("-1-ILMLOG:my_log faultCheckRoutine");
          ilmFaults = faultPkt.faults;
          if (faultPkt.fcSet) {
            mqttSendData = publish_data(event_lamp_fault_set,powerFRTs);
            if (mqttSendData.dataSize) {
              ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 * ) & mqttSendData, 0);
              QL_MQTT_LOG("-1-ILMLOG:my_log_1faultPkt.fcSet ConcontrollerStatus=%d", coControllerStatus);
            }

          }
          if (faultPkt.fcClear) {
            mqttSendData = publish_data(event_lamp_fault_clear,powerFRTs);
            if (mqttSendData.dataSize)
              ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 * ) & mqttSendData, 0);

          }
        }
        else
          lampStateUpdater();
        }
      }

      if (sendPkt != D_SEND_NO_DATA) {
        mqttSendData = publish_data(sendPkt,powerFRTs);
        if (mqttSendData.dataSize)
          ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 * ) & mqttSendData, 0);
        QL_MQTT_LOG("-1-ILMLOG:my_log_1senddata");
        sendPkt = D_SEND_NO_DATA;
      }

    }
  
  }
 }
}