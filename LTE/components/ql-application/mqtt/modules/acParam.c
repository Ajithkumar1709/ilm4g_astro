#include "ilmCommon.h"
#include "acParam.h"
#include "coControllerProcess.h"
#include "powerFailHandler.h"


//DEFINES
//GLOBALS AND STATICS
uint8_t coControllerStatus=false;
acParamNu_t acParamNu;
acParam_t  acParam;

//EXTERNS
extern volatile uint8_t uC_pktRcd;
extern unsigned char nuAcMeasCommand[];
extern uint8_t ptr[39];
extern U1 batKillEnable;
extern void uart_data_req(unsigned char * data);
extern U1 instaGetPowerFailSense(void);
extern U1 uartPacketRcd;

unsigned char uC_Checksum(unsigned char * data, unsigned char length) {
  unsigned char count;
  unsigned int Sum = 0;
  for (count = 0; count < length; count++)
    Sum += data[count];
  Sum = -Sum;
  return (Sum & 0xFF);
}


U1 acMeasureCheck() {
  uC_pktRcd = 0;
  coControllerStatus = D_COCONTROLLER_NOTOK;
  int i;
  for (i = 0; i < 39; i++) {
    uC_input_handler(ptr[i]);
  }
  QL_MQTT_LOG("-1-ACLOG:MEASURE ENTER");
  //check flag 
  if (uC_pktRcd) {
    memcpy(acParamNu.ucData, ptr, 39);
    //  check sum 
    if (uC_Checksum((uint8_t * ) & acParamNu.uCProtoPkt.ucAcGnss, sizeof(ucAcGnss_t)) == acParamNu.uCProtoPkt.checksum) {
      QL_MQTT_LOG("-1-ACLOG:PKT RCD OK");

      acParam.voltage = acParamNu.uCProtoPkt.ucAcGnss.pvc.voltage;
      acParam.current = acParamNu.uCProtoPkt.ucAcGnss.pvc.current;
      acParam.watt = acParamNu.uCProtoPkt.ucAcGnss.pvc.power;
      if (acParam.watt > 0)
        acParam.calculatedPF = (acParam.watt) / (acParam.voltage * ((acParam.current) / 1000.00));
      else
        acParam.calculatedPF = 0;
      QL_MQTT_LOG("-1-ACLOG:voltage=%f", acParamNu.uCProtoPkt.ucAcGnss.pvc.voltage);
      QL_MQTT_LOG("-1-ACLOG:current=%f", acParamNu.uCProtoPkt.ucAcGnss.pvc.current);
      QL_MQTT_LOG("-1-ACLOG:power=%f", acParamNu.uCProtoPkt.ucAcGnss.pvc.power);
      QL_MQTT_LOG("-1-ACLOG:ack=%d", acParamNu.uCProtoPkt.ackNo);
      if (acParam.current <= 55) //mA
        acParam.current = 0;
      if (acParam.voltage <= 15) //volts
        acParam.voltage = 0;
      if (acParam.watt <= 5) //volts
        acParam.watt = 0;
      if ((acParam.voltage == 0) && (acParam.current == 0)) {
        acParam.watt = 0;
        acParam.calculatedPF = 0;
      }
      if (((batKillEnable == true) && (getPowerFailSense() == true))) {
        acParam.voltage = 0;
        acParam.current = 0;
        acParam.watt = 0;
        acParam.calculatedPF = 0;
      }
      coControllerStatus = D_COCONTROLLER_OK;
    }
    uC_pktRcd = 0;
  } else {
    acParam.voltage = 0;
    acParam.current = 0;
    acParam.watt = 0;
    acParam.calculatedPF = 0;
    coControllerStatus = D_COCONTROLLER_NOTOK;
    
  }
  return coControllerStatus;
}

U1 nuAckCheck() {
  uC_pktRcd = 0;
  coControllerStatus = D_COCONTROLLER_NOTOK;
  int i;
  for (i = 0; i < 39; i++) {
    uC_input_handler(ptr[i]);
  }
  QL_MQTT_LOG("-1-ACLOG:NUVOTON ACK CHECk");
  //check flag 
  if (uC_pktRcd) {
    memcpy(acParamNu.ucData, ptr, 39);
    //  check sum 
    if (uC_Checksum((uint8_t * ) & acParamNu.uCProtoPkt.ucAcGnss, sizeof(ucAcGnss_t)) == acParamNu.uCProtoPkt.checksum) {
      QL_MQTT_LOG("-1-ACLOG:NUVOTON ACK OK");
      coControllerStatus = D_COCONTROLLER_OK;
    }
    uC_pktRcd = 0;
  }
  else {
    coControllerStatus = D_COCONTROLLER_NOTOK;
  }
  return coControllerStatus;
}

void acMeasure(void) {
  U1 nuStatus,rtyCount=0;
  U1 timeout=0;
  do{
   acParam.gridStatus=instaGetPowerFailSense();
   uartPacketRcd=false;
   uart_data_req(nuAcMeasCommand);
   timeout=0;
   do{
   ql_rtos_task_sleep_ms(200);
   timeout++;
   }while((uartPacketRcd==false)&&(timeout<15));
   //ql_rtos_task_sleep_s(3);
   nuStatus=acMeasureCheck();
   rtyCount++;
   if(nuStatus==D_COCONTROLLER_NOTOK){
    ql_rtos_task_sleep_s(1);
    QL_MQTT_LOG("-1-ACLOG:rtyCount=%d",rtyCount);
   }
   }while((nuStatus==D_COCONTROLLER_NOTOK)&&(rtyCount<3));

}