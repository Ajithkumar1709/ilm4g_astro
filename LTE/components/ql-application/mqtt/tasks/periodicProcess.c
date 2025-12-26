#include "ilmCommon.h"
#include"rpcHandler.h"
#include "fsDriver.h"
#include"periodicProcess.h"
#include"publishData.h"
#include"memoryHandler.h"
#include"powerFailHandler.h"
#include "ql_api_sim.h"

//DEFINES
#define D_BURNTIME_3MIN 180
#define D_FAULT_BURN_TIME_DURATION 5

//GLOBALS AND STATICS
U2 burnTimeCounter=0;

//EXTERNS
extern ilm_t ilm;
extern ql_queue_t ql_ilmProcess_qHandle;
extern void batKillAction(void);

extern ql_queue_t ql_payloadProcess_qHandle;
extern    U1 onBootPowerRetainProcessed ;

void burnTimeStore(void) {
  //BURN TIME
  if (D_BURNTIME_3MIN <= burnTimeCounter) {
    burnTimeCounter = 0;
    //ilmStore
     QL_MQTT_LOG("-1-PSTART:burnTimeStore");
    if(!getPowerFailSense())
      nvmWriteIlmConfig();
  } else {
    burnTimeCounter++;
  }
}
 U1  faultBurnTimeCounter=0;
extern U1 powerFailFlag;

void periodicProcess_thread(void *param){
  ilmQueue_t ilmData;
  payLoadQueue_t payLoadQueue;
  U4 periodicCounter=0;
 
  ql_rtos_task_sleep_s(3);
  QL_MQTT_LOG("-1-PSTART:periodicProcess_thread");
  ql_rtos_queue_create( & ql_payloadProcess_qHandle, sizeof(payLoadQueue_t), 5);
  while(1){
    payLoadQueue.process=false;
    ql_rtos_queue_wait(ql_payloadProcess_qHandle, (uint8 *)&payLoadQueue, sizeof(payLoadQueue_t),1000);
    if(payLoadQueue.process==true){
         fsPayloadUpload(payLoadQueue.fromTime,payLoadQueue.toTime);
       
    }

    burnTimeStore();

   if(powerFailFlag==false)
      faultBurnTimeCounter++;
   if(faultBurnTimeCounter>=D_FAULT_BURN_TIME_DURATION)
    {
      faultBurnTimeCounter=0;
      QL_MQTT_LOG("-1-PERIODICLOG:my_log_inside_periodic");
      ilmData.pktType=D_LAMP_BURN;
      ilmData.dataSize=1;
      ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 *)&ilmData, 0);
    }

   if(onBootPowerRetainProcessed ==1)
   {
    
    periodicCounter++;
    QL_MQTT_LOG("-1-PERIODICLOG:my_log_outside_periodic");
    QL_MQTT_LOG("-1-PERIODICLOG:my_log_outside_periodic=%lu",periodicCounter);
    // QL_MQTT_LOG("my_log_outside_periodic ilm.ilmConfig.periodicInterval=%d",ilm.ilmConfig.periodicInterval);
   // if(periodicCounter>=30)
    if(periodicCounter>=ilm.ilmConfig.periodicInterval)
    {
    QL_MQTT_LOG("-1-PERIODICLOG:my_log_inside_periodic");
      periodicCounter=0;
      ilmData.pktType=D_PERIODIC_INTERVAL;
      ilmData.dataSize=1;
      ql_rtos_queue_release(ql_ilmProcess_qHandle, sizeof(ilmData), (uint8 *)&ilmData, 0);
    }
   }

  }
}
