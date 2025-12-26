
// Online C compiler to run C program online
#include "ilmCommon.h"
#include "faultChecker.h"
#include "ql_mqttclient.h"

//DEFINES
#define true 1
#define false 0
#define D_FAULT 1
#define D_NOFAULT 0


//GLOBALS AND STATICS

//EXTERNS
extern ilm_t ilm;
extern faults_t faults;
float wattMinFaultChecker;


faults_t faultChecker(acParam_t acParam){

  /*ilm.ilmConfig.acThresholds.minVolt=200000;
  ilm.ilmConfig.acThresholds.maxVolt=260000;
	ilm.ilmConfig.acThresholds.minCurrent=80;
	ilm.ilmConfig.acThresholds.maxCurrent=1500;
	ilm.ilmConfig.acThresholds.minWatt=10000;//10;
	ilm.ilmConfig.acThresholds.maxWatt=300000;
	ilm.ilmConfig.acThresholds.minLoad=50;//min 
	ilm.ilmConfig.acThresholds.noLoad=50;*/
  QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker_writeilm.lamp.relay %d", ilm.lamp.relay);
  faults.wattMinFault=false;
  faults.wattMaxFault=false;
  faults.lowPowerFactorFault=false;
  faults.noLoadFault=false;
  faults.relayFault=false;
  faults.currentMinFault=false;
  faults.currentMaxFault=false;

if(faults.voltMinFault==false){
  faults.voltMinFault=((acParam.voltage*1000)<=ilm.ilmConfig.acThresholds.minVolt)?(true):(false);//255<240
  }
  else{
  faults.voltMinFault=(((acParam.voltage) * 1000) <= (ilm.ilmConfig.acThresholds.minVolt+5000))?(true):(false);
  }
  
  if(faults.voltMaxFault==false){
  faults.voltMaxFault=((acParam.voltage*1000)>=ilm.ilmConfig.acThresholds.maxVolt)?(true):(false);//264>250
  }
  else{
  faults.voltMaxFault=(((acParam.voltage) * 1000) >= (ilm.ilmConfig.acThresholds.maxVolt-5000))?(true):(false);//2680>=265)
  }
  
  if(ilm.lamp.relay==D_RELAY_ON){
     faults.currentMinFault=((acParam.current)<=ilm.ilmConfig.acThresholds.minCurrent)?(true):(false);
  }
  faults.currentMaxFault=((acParam.current)>=ilm.ilmConfig.acThresholds.maxCurrent)?(true):(false);
  
 if((ilm.lamp.relay==D_RELAY_ON)||(ilm.lamp.relay==D_RELAY_DIM)){
    wattMinFaultChecker=(ilm.ilmConfig.acThresholds.minWatt/100)*ilm.lamp.brightness;
    QL_MQTT_LOG("-1-FAULTCHECKER:My_log_faults wattMinFaultChecker =%f",wattMinFaultChecker);
    faults.wattMinFault=( (acParam.watt*1000)<=wattMinFaultChecker)?(true):(false);
    QL_MQTT_LOG("-1-FAULTCHECKER:My_log_faults.currentMinFault=%f,ilm.ilmConfig.acThresholds.minCurrent=%ld\n",acParam.watt,ilm.ilmConfig.acThresholds.minWatt);
  }
  faults.wattMaxFault=( (acParam.watt*1000)>=ilm.ilmConfig.acThresholds.maxWatt)?(true):(false);

  if(ilm.ilmConfig.acThresholds.minPowerFactor!=1){
  if(ilm.lamp.relay==D_RELAY_ON){
  faults.lowPowerFactorFault=( (acParam.calculatedPF*100)<=ilm.ilmConfig.acThresholds.minPowerFactor)?(true):(false);//99<100
  }
  }
 // faults.noLoadFault=(((ilm.lamp.relay==D_RELAY_ON)||(ilm.lamp.relay==D_RELAY_DIM))&&(acParam.current<=ilm.ilmConfig.acThresholds.noLoad))?(true):(false);//276<=876
//faults.relayFault=((ilm.lamp.relay==D_RELAY_OFF)&&(acParam.current>=ilm.ilmConfig.acThresholds.minLoad))?(true):(false);//876>=276
  faults.noLoadFault=(((ilm.lamp.relay==D_RELAY_ON)||(ilm.lamp.relay==D_RELAY_DIM))&&((acParam.watt*1000)<=ilm.ilmConfig.acThresholds.noLoad))?(true):(false);//276<=876
  faults.relayFault=((ilm.lamp.relay==D_RELAY_OFF)&&((acParam.watt*1000)>=ilm.ilmConfig.acThresholds.minLoad))?(true):(false);//876>=276
if((faults.noLoadFault==false)&&(faults.relayFault==false)){
  if( (ilm.lamp.relay==D_RELAY_ON)||(ilm.lamp.relay==D_RELAY_DIM))
      ilm.lamp.state=LAMP_ON;

    if(ilm.lamp.relay==D_RELAY_OFF)
      ilm.lamp.state=LAMP_OFF;
}



if((faults.noLoadFault==true)||(faults.relayFault==true)){
    ilm.lamp.state=LAMP_FAULT;
  
}
   // faults.noLoadFault=(acParam.current<=ilm.ilmConfig.acThresholds.noLoad)?(true):(false);//650<20
   // faults.relayFault=(acParam.current>=ilm.ilmConfig.acThresholds.minLoad)?(true):(false);//650>51
  // QL_MQTT_LOG("my_log_faults.voltMaxFault:%d,ilm.ilmConfig.acThresholds.maxVolt=%d\n\n",acParam.voltage,ilm.ilmConfig.acThresholds.maxVolt);
   QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker  111 faults.voltMaxFault=%d", faults.voltMaxFault);
   QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker ");
   return faults;


}








  U1 faultCounter[EndFault];
  faults_t faultsFilter(faults_t faults) {
  static U1 doOnce=false;
 
  static faults_t filteredFaults;
  if(doOnce==false){
    memset((U1*)&filteredFaults,0,sizeof(filteredFaults));
    doOnce=true;
  }
 QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault-%d", filteredFaults.wattMinFault);
  if (faults.voltMinFault == D_FAULT) {
    if (faultCounter[voltMinFault] < 3)
      faultCounter[voltMinFault]++;
    else{
      filteredFaults.voltMinFault = D_FAULT;
     
       
    }
  } else {
    if (faultCounter[voltMinFault] != 0){
      faultCounter[voltMinFault]--;
      //filteredFaults.voltMinFault = D_FAULT;
    }
    else{
      filteredFaults.voltMinFault = D_NOFAULT;
     QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker  111 set ");
     
    }
  }

 
  if (faults.voltMaxFault == D_FAULT) {
    if (faultCounter[voltMaxFault] < 3){
      faultCounter[voltMaxFault]++;
    
    }
    else{
       
      filteredFaults.voltMaxFault = D_FAULT;
         QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker  111 clear ");
    }
   
  }
 
   else {
    if (faultCounter[voltMaxFault] != 0){
      faultCounter[voltMaxFault]--;
    //  filteredFaults.voltMaxFault = D_FAULT;
    }
    else
      filteredFaults.voltMaxFault = D_NOFAULT;
  }
 QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker 111 faultCounter[voltMaxFault]=%d  ",faultCounter[voltMaxFault]);
 QL_MQTT_LOG("-1-FAULTCHECKER:mylog faultChecker 111 faults.voltMaxFault=%d  ",faults.voltMaxFault);
//  printf("Volt max %d \n", filteredFaults.voltMaxFault);
 
  if (faults.currentMinFault == D_FAULT) {
    if (faultCounter[currentMinFault] < 3)
      faultCounter[currentMinFault]++;
    else
      filteredFaults.currentMinFault = D_FAULT;

  } else {
    if (faultCounter[currentMinFault] != 0){
      faultCounter[currentMinFault]--;
   //   filteredFaults.currentMinFault = D_FAULT;
    }
    else
      filteredFaults.currentMinFault = D_NOFAULT;
  }

  if (faults.currentMaxFault == D_FAULT) {
    if (faultCounter[currentMaxFault] < 3)
      faultCounter[currentMaxFault]++;
    else
      filteredFaults.currentMaxFault = D_FAULT;

  } else {
    if (faultCounter[currentMaxFault] != 0){
      faultCounter[currentMaxFault]--;
  //    filteredFaults.currentMaxFault = D_FAULT;
    }
    else
      filteredFaults.currentMaxFault = D_NOFAULT;
  }


//printf("out------%d\n",faults.wattMinFault);
  if (faults.wattMinFault == D_FAULT) {
  //  QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault falut clear_count%d\n",faults.wattMinFault);
    if (faultCounter[wattMinFault] < 3){
   QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault falut set_count=%d\n", faultCounter[wattMinFault]);
      faultCounter[wattMinFault]++;
    }
    else
    {
      filteredFaults.wattMinFault = D_FAULT;
    QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault falut set\n");
     
    }

  } else {
    if (faultCounter[wattMinFault] != 0){
      faultCounter[wattMinFault]--;
    //  filteredFaults.wattMinFault = D_FAULT;
      QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault falut clear_count=%d\n", faultCounter[wattMinFault]);
    }
    else{
      filteredFaults.wattMinFault = D_NOFAULT;
      QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault falut clear");
    }
  }

  if (faults.wattMaxFault == D_FAULT) {
    if (faultCounter[wattMaxFault] < 3)
      faultCounter[wattMaxFault]++;
    else
      filteredFaults.wattMaxFault = D_FAULT;

  } else {
    if (faultCounter[wattMaxFault] != 0){
      faultCounter[wattMaxFault]--;
    //  filteredFaults.wattMaxFault = D_FAULT;
    }
    else
      filteredFaults.wattMaxFault = D_NOFAULT;
  }

  if (faults.noLoadFault == D_FAULT) {
    if (faultCounter[noLoadFault] < 3)
      faultCounter[noLoadFault]++;
    else
      filteredFaults.noLoadFault = D_FAULT;

  } else {
    if (faultCounter[noLoadFault] != 0){
      faultCounter[noLoadFault]--;
   //   filteredFaults.noLoadFault = D_FAULT;
    }
    else
      filteredFaults.noLoadFault = D_NOFAULT;
  }

  if (faults.relayFault == D_FAULT) {
    if (faultCounter[relayFault] < 3)
      faultCounter[relayFault]++;
    else
      filteredFaults.relayFault = D_FAULT;
      
    

  } else {
    if (faultCounter[relayFault] != 0){
      faultCounter[relayFault]--;
    //  filteredFaults.relayFault = D_FAULT;
    }
    else
      filteredFaults.relayFault = D_NOFAULT;
  }

 
 if (faults.lowPowerFactorFault == D_FAULT) {
    if (faultCounter[lowPowerFactorFault] < 3)
      faultCounter[lowPowerFactorFault]++;
    else
      filteredFaults.lowPowerFactorFault = D_FAULT;

  } else {
    if (faultCounter[lowPowerFactorFault] != 0){
      faultCounter[lowPowerFactorFault]--;
    //  filteredFaults.lowPowerFactorFault = D_FAULT;
    }
    else
      filteredFaults.lowPowerFactorFault = D_NOFAULT;
  }
QL_MQTT_LOG("-1-FAULTCHECKER:watt_minFault-%d", filteredFaults.wattMinFault);
  return filteredFaults;

}

faultPkt_t faultEventGenerator(faults_t presentfaults,faults_t oldFaults,faultPkt_t faultPkt){
    faultPkt.fcSet=0;
    faultPkt.fcClear=0;
    faultPkt.faults=presentfaults;
    
    
    //if(presentfaults!=oldFaults)
    {
        if(presentfaults.noLoadFault!=oldFaults.noLoadFault){
        if(presentfaults.noLoadFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<noLoadFault);
          faultPkt.fd|=(1<<noLoadFault);
       //  ilm.lamp.state=LAMP_FAULT;
         QL_MQTT_LOG("-1-FAULTCHECKER:My_log_noload Event falut set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<noLoadFault);
         faultPkt.fd &= ~(1 << noLoadFault);
        // ilm.lamp.state=LAMP_ON;

         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_noload Event falut clear\n");
        }
        }
        if(presentfaults.relayFault!=oldFaults.relayFault){
        if(presentfaults.relayFault==D_FAULT){
         //Fault Set Event
          faultPkt.fcSet|=(1<<relayFault);
          faultPkt.fd|=(1<<relayFault);
        //  ilm.lamp.state=LAMP_FAULT;
          QL_MQTT_LOG("-1-FAULTCHECKER:my_log_relay Event falut set\n");
        }
        else{
         //Fault Clear Event
          faultPkt.fcClear|=(1<<relayFault);
          faultPkt.fd &= ~(1 << relayFault);
        //  ilm.lamp.state=LAMP_OFF;
          QL_MQTT_LOG("-1-FAULTCHECKER:my_log_relay Event falut clear\n");
        }
        }
        if(presentfaults.voltMinFault!=oldFaults.voltMinFault){
        if(presentfaults.voltMinFault==D_FAULT){
         //Fault Set Event
          faultPkt.fcSet|=(1<<voltMinFault);
          faultPkt.fd|=(1<<voltMinFault);
        //  ilm.lamp.state=LAMP_FAULT;
          QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ Volt_min Event Set\n");
        }
        else{
         //Fault Clear Event
            faultPkt.fcClear|=(1<<voltMinFault);
            faultPkt.fd &= ~(1 << voltMinFault);
        //    ilm.lamp.state=LAMP_ON;
            QL_MQTT_LOG("-1-FAULTCHECKER:my_log_Volt_min Event Clear\n");
        }
        }
        if(presentfaults.voltMaxFault!=oldFaults.voltMaxFault){
        if(presentfaults.voltMaxFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<voltMaxFault);
         faultPkt.fd|=(1<<voltMaxFault);
       //  ilm.lamp.state=LAMP_FAULT;
            QL_MQTT_LOG("-1-FAULTCHECKER:my_log_Volt_max Event Set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<voltMaxFault);
         faultPkt.fd &= ~(1 << voltMaxFault);
       //  ilm.lamp.state=LAMP_ON;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_Volt_max Event Clear\n");
        }
        }
        if(presentfaults.currentMinFault!=oldFaults.currentMinFault){
        if(presentfaults.currentMinFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<currentMinFault);
         faultPkt.fd|=(1<<currentMinFault);
      //   ilm.lamp.state=LAMP_FAULT;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ current_min Event set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<currentMinFault);
         faultPkt.fd &= ~(1 << currentMinFault);
       //  ilm.lamp.state=LAMP_ON;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ current_min Event Clear\n");
        }
        }
        if(presentfaults.currentMaxFault!=oldFaults.currentMaxFault){
        if(presentfaults.currentMaxFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<currentMaxFault);
         faultPkt.fd|=(1<<currentMaxFault);
      //   ilm.lamp.state=LAMP_FAULT;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ current_max Event set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<currentMaxFault);
         faultPkt.fd &= ~(1 << currentMaxFault);
       //  ilm.lamp.state=LAMP_ON;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ current_max Event Clear\n");
        }
        }
        //printf("----%d\n",presentfaults.wattMinFault);
        //printf("----%d\n",oldFaults.wattMinFault);
        if(presentfaults.wattMinFault!=oldFaults.wattMinFault){
        if(presentfaults.wattMinFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<wattMinFault);
         faultPkt.fd|=(1<<wattMinFault);
       //  ilm.lamp.state=LAMP_FAULT;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ watt_min Event set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<wattMinFault);
         faultPkt.fd &= ~(1 << wattMinFault);
       //  ilm.lamp.state=LAMP_ON;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ watt_min Event clear\n");
        }
        }
        if(presentfaults.wattMaxFault!=oldFaults.wattMaxFault){
        if(presentfaults.wattMaxFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<wattMaxFault);
         faultPkt.fd|=(1<<wattMaxFault);
       //  ilm.lamp.state=LAMP_FAULT;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_ watt_max Event set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<wattMaxFault);
         faultPkt.fd &= ~(1 << wattMaxFault);
        // ilm.lamp.state=LAMP_ON;
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_watt_max Event clear\n");
         
        }
        }
        if(presentfaults.lowPowerFactorFault!=oldFaults.lowPowerFactorFault){
        if(presentfaults.lowPowerFactorFault==D_FAULT){
         //Fault Set Event
         faultPkt.fcSet|=(1<<lowPowerFactorFault);
         faultPkt.fd|=(1<<lowPowerFactorFault);
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_pf_min Event set\n");
        }
        else{
         //Fault Clear Event
         faultPkt.fcClear|=(1<<lowPowerFactorFault);
         faultPkt.fd &= ~(1 << lowPowerFactorFault);
         QL_MQTT_LOG("-1-FAULTCHECKER:my_log_pf_min Event Clear\n");
        }
        }
    }
    // if(((faultPkt.fd&(1<<relayFault))==0)&&((faultPkt.fd&(1<<noLoadFault))==0)){
    //   if( (ilm.lamp.relay==D_RELAY_ON)||(ilm.lamp.relay==D_RELAY_DIM))
    //         ilm.lamp.state=LAMP_ON;
            
    //   if(ilm.lamp.relay==D_RELAY_OFF)
    //         ilm.lamp.state=LAMP_OFF;
    // }
    return faultPkt;
}

faultPkt_t faultCheckRoutine(acParam_t acParam,faults_t ilmFaults,faultPkt_t faultPkt){
    faults_t faults,filteredFaults;
    faults=faultChecker(acParam);
    filteredFaults=faultsFilter(faults);
    return faultEventGenerator(filteredFaults,ilmFaults,faultPkt); 
}



