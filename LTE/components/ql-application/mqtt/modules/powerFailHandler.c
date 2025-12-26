#include "ilmCommon.h"
#include "ql_gpio.h"
#include "memoryHandler.h"
#include"rtc.h"
//DEFINES
#define D_MAXPOWERFAILCOUNTER 0

//GLOBALS AND STATICS
U1 powerFailFlag = false;
U1 batKillEnable = false;
U2 batKillTimeCounter = 0;
U1 powerFailFlagCounter = 0;

//EXTERNS
extern  ql_queue_t ql_ilmProcess_qHandle;
extern ilm_t ilm;
extern void faultCounterReset(void);
extern acParam_t acParam;
extern void acMeasure();
extern void lampStateUpdater();
extern faults_t faultChecker();
U1 instaGetPowerFailSense(void) {

   ql_LvlMode gpio_lvl;
   ql_gpio_get_level(2, & gpio_lvl);
    if (gpio_lvl == LVL_LOW){
       return BATTERY_MODE; //  Power is not available
    }
  return AC_SUPPLY; // Power is available
}

U1 getPowerFailSense(void) {
  U1 count=0;
  ql_LvlMode gpio_lvl;
  ql_gpio_get_level(2, & gpio_lvl);
  QL_MQTT_LOG("-1-PWRFAILLOG: PowerFailSense=%d", gpio_lvl);

  if (gpio_lvl == LVL_LOW){
  do 
		{
      count++;
      if(count>15){
         return true; //  Power is not available
      }
     ql_rtos_task_sleep_ms(200);
     ql_gpio_get_level(2, & gpio_lvl); 
		 }while(gpio_lvl == LVL_LOW);
	}
  return false; // Power is available
}

U1 inBatteryModeCheck(void){
	U1 count=0;
	if(instaGetPowerFailSense()==BATTERY_MODE){
	    do{
	      count++;
	      if(count>15){
	         QL_MQTT_LOG("-1-PWRFAILLOG:X BATTERY_MODE");
		 return true; //  Power is not available
	      }
	     ql_rtos_task_sleep_ms(200);
	     }while(instaGetPowerFailSense()==BATTERY_MODE);
	}
	return false;
}

U1 inAcModeCheck(void){
	U1 count=0;
	if(instaGetPowerFailSense()==AC_SUPPLY){
	    do{
	      count++;
	      if(count>15){
	      	 QL_MQTT_LOG("-1-PWRFAILLOG:X AC_SUPPLY");
		 return true; //  Power is available
	      }
	     ql_rtos_task_sleep_ms(200);
	     }while(instaGetPowerFailSense()==AC_SUPPLY);
	}
	return false;
}

U1 acFlickerDetectionBat2PWR(void){
	U1 flickerCount=0;
	do{
	     if(inBatteryModeCheck()==true){
	     	 return BATTERY_MODE;
	     }
	     if(inAcModeCheck()==true){
		 return AC_SUPPLY;
	     }
	     flickerCount++;
	}while(flickerCount<10);
	QL_MQTT_LOG("-1-PWRFAILLOG:X Bat2PWR FLICKER_MODE");
	return FLICKER_MODE;
}

U1 acFlickerDetectionPWR2BAT(void){
	U1 flickerCount=0;
	do{
	     if(inAcModeCheck()==true)
		 return AC_SUPPLY;
	     if(inBatteryModeCheck()==true){
		 return BATTERY_MODE;
	     }
	     
	     flickerCount++;
	}while(flickerCount<10);
	QL_MQTT_LOG("-1-PWRFAILLOG:X PWR2BAT FLICKER_MODE");
	return FLICKER_MODE;
}

extern mqttQueue_t publish_data(U1 pktType,powerFRTs_t powerFRTs) ;

mqttQueue_t powerFailSenseRoutine(rtc_t rtc){
  extern U1 lampBurnAndFaultCheck;
	ilmQueue_t ilmData;
//POWER FAIL SENSE
	ilmData.dataSize = 0;
	mqttQueue_t mqttSendData;
	if (powerFailFlag == false){
	 if(acFlickerDetectionBat2PWR()==BATTERY_MODE){ 
		 acMeasure();
		 if((acParam.voltage<50)&&(instaGetPowerFailSense()==BATTERY_MODE)){
			rtc=getRtc();
			powerFailFlag = true;
			//config Store
			QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER");
			nvmWriteIlmConfigBackUp();
			nvmWriteIlmConfig();
			QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE EXIT");
			//bat kill
			batKillEnable = true;
			batKillTimeCounter = 0;
			lampBurnAndFaultCheck=false;
			faultCounterReset();
			//Power
			if (rtc.status == RTC_SET){
				ilmData.pktType = event_battery_power_retain;
				ilmData.dataSize = 1;
				ilm.powerFRTs.powerFailTs = rtc.timeInSec;
				ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
				QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER%d",rtc.timeInSec);
				QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER");
				nvmWriteIlmConfigBackUp();
				nvmWriteIlmConfig();
				QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE EXIT");
				//ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
        lampStateUpdater();
				mqttSendData = publish_data(event_battery_power_retain,ilmData.powerFRTs);
				return mqttSendData;
			}
		 }
	 }
	}
	 else{
		if(powerFailFlag == true){
			if(acFlickerDetectionPWR2BAT()==AC_SUPPLY){
				rtc=getRtc();
				acMeasure();
				ql_rtos_task_sleep_s(2);
				acMeasure();
        ql_rtos_task_sleep_s(1);
				if((acParam.voltage>50)&&(instaGetPowerFailSense()==AC_SUPPLY)){
					extern U1 faultBurnTimeCounter;
					faultBurnTimeCounter=0;
					faultCounterReset();
					powerFailFlag = false;
					powerFailFlagCounter = 0;
					batKillEnable = false;
					//Power Retain Packet Send 
					if (rtc.status == RTC_SET) {
						
						lampBurnAndFaultCheck=false; 
						ilm.powerFRTs.powerRetainTs = rtc.timeInSec;
						ilmData.powerFRTs.powerRetainTs= ilm.powerFRTs.powerRetainTs;
						ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
						QL_MQTT_LOG("-1-PWRFAILLOG: POWER_RETAIN OCCURED=%d", rtc.timeInSec);
						QL_MQTT_LOG("-1-PWRFAILLOG: POWER_RETAIN OCCURED");
						ilmData.pktType = event_power_retain;
						ilmData.dataSize = 1;
						//ql_rtos_task_sleep_s(3);
						 // lampStateUpdater();
             faultChecker(acParam);
						//ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
						mqttSendData = publish_data(event_power_retain,ilmData.powerFRTs);
						return mqttSendData;
					}
				}


			}
		}
	}
	mqttSendData.dataSize =0;
	return mqttSendData;
}

/*
ilmQueue_t powerFailSenseRoutine(rtc_t rtc) {
  ilmQueue_t ilmData;
  //POWER FAIL SENSE
  ilmData.dataSize = 0;
  if (powerFailFlag == false){
     if(getPowerFailSense()) {
 // if ((getPowerFailSense()) && (powerFailFlag == false)) {
    if (powerFailFlagCounter >= D_MAXPOWERFAILCOUNTER) {
      powerFailFlag = true;

      //config Store
      QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER");
      nvmWriteIlmConfigBackUp();
      nvmWriteIlmConfig();
      QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE EXIT");

      //bat kill
      batKillEnable = true;
      batKillTimeCounter = 0;
      rtc=getRtc();
      //Power
      if (rtc.status == RTC_SET) {
        ilmData.pktType = event_battery_power_retain;
        ilmData.dataSize = 1;
        ilm.powerFRTs.powerFailTs = rtc.timeInSec;
        ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
        QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER%d",rtc.timeInSec);
        QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER");
        nvmWriteIlmConfigBackUp();
        nvmWriteIlmConfig();
        QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE EXIT");
        //ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
      }
    } else {
      powerFailFlagCounter++;
    }
  } 
  }
  
  else {
         rtc=getRtc();
    if ((0 == getPowerFailSense()) && (powerFailFlag == true)) {
      extern U1 faultBurnTimeCounter;
      faultBurnTimeCounter=0;
      powerFailFlag = false;
      powerFailFlagCounter = 0;
      batKillEnable = false;
      //Power Retain Packet Send 
      if (rtc.status == RTC_SET) {
        
        extern U1 lampBurnAndFaultCheck;
        lampBurnAndFaultCheck=false; 
        ilm.powerFRTs.powerRetainTs = rtc.timeInSec;
        ilmData.powerFRTs.powerRetainTs= ilm.powerFRTs.powerRetainTs;
        ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
        QL_MQTT_LOG("-1-PWRFAILLOG: POWER_RETAIN OCCURED=%d", rtc.timeInSec);
        QL_MQTT_LOG("-1-PWRFAILLOG: POWER_RETAIN OCCURED");
        ilmData.pktType = event_power_retain;
        ilmData.dataSize = 1;
        ql_rtos_task_sleep_s(3);
        //ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
      }
    }
  }
  return ilmData;
}
*/
/*
void powerFailSenseRoutine(rtc_t rtc) {
  ilmQueue_t ilmData;
  //POWER FAIL SENSE

  if (powerFailFlag == false){
     if(getPowerFailSense()) {
 // if ((getPowerFailSense()) && (powerFailFlag == false)) {
    if (powerFailFlagCounter >= D_MAXPOWERFAILCOUNTER) {
      powerFailFlag = true;

      //config Store
      QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER");
      nvmWriteIlmConfigBackUp();
      nvmWriteIlmConfig();
      QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE EXIT");

      //bat kill
      batKillEnable = true;
      batKillTimeCounter = 0;
      //Power
      if (rtc.status == RTC_SET) {
        ilmData.pktType = D_POWER_FAIL;
        ilmData.dataSize = 1;
        ilm.powerFRTs.powerFailTs = rtc.timeInSec;
        ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
        QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER%d",rtc.timeInSec);
        QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE ENTER");
        nvmWriteIlmConfigBackUp();
        nvmWriteIlmConfig();
        QL_MQTT_LOG("-1-PWRFAILLOG:NV STORE EXIT");
        ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
      }
    } else {
      powerFailFlagCounter++;
    }
  } 
  }
  
  else {
         rtc=getRtc();
    if ((0 == getPowerFailSense()) && (powerFailFlag == true)) {
      powerFailFlag = false;
      powerFailFlagCounter = 0;
      batKillEnable = false;
      //Power Retain Packet Send 
      if (rtc.status == RTC_SET) {
        ilm.powerFRTs.powerRetainTs = rtc.timeInSec;
        ilmData.powerFRTs.powerRetainTs= ilm.powerFRTs.powerRetainTs;
        ilmData.powerFRTs.powerFailTs=ilm.powerFRTs.powerFailTs;
        QL_MQTT_LOG("-1-PWRFAILLOG: POWER_RETAIN OCCURED=%d", rtc.timeInSec);
        QL_MQTT_LOG("-1-PWRFAILLOG: POWER_RETAIN OCCURED");
        ilmData.pktType = D_POWER_RETAIN;
        ilmData.dataSize = 1;
        ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 * ) & ilmData, 0);
      }
    }
  }
}*/

void batKillAction(void){
  extern unsigned char nuBatKill[];
  extern void uart_data_req(unsigned char * data);
  //BAT KILL
   QL_MQTT_LOG("-1-PWRFAILLOG:batKillTimeCounter=%d,ilm.ilmConfig.batKillDelay=%d",batKillTimeCounter,ilm.ilmConfig.batKillDelay);
  
  if (ilm.ilmConfig.batKillDelay <= (batKillTimeCounter)) {
    batKillTimeCounter = 0;
    //bat Kill Command
    QL_MQTT_LOG("-1-PWRFAILLOG:batKillACtion done");
    uart_data_req(nuBatKill);

  } else {
    batKillTimeCounter++;//1 count is equal to 7 sec
  }
}