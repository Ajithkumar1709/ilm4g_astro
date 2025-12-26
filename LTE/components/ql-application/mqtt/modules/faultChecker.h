#ifndef fault_checker_H
#define fault_checker_H
#include"stdint.h"
#include <stdio.h>
#include "ilmCommon.h"
//#include"AC_param.h"





typedef  enum{
  //noLoadFault=1,
  relayFault=1,
  voltMinFault,
  voltMaxFault,
  currentMinFault,
  currentMaxFault,
  meterCommunication,
  noLoadFault,
  wattMinFault,
  wattMaxFault,
  lowPowerFactorFault,
  EndFault
}fault_e;










extern faults_t faultChecker(acParam_t acParam);
extern faults_t faultsFilter(faults_t faults);
extern faultPkt_t faultEventGenerator(faults_t presentfaults,faults_t oldFaults,faultPkt_t faultPkt);
extern faultPkt_t faultCheckRoutine(acParam_t acParam,faults_t ilmFaults,faultPkt_t faultPkt);



#endif