#ifndef AC_param_H
#define AC_param_H
#include"stdint.h"

extern volatile uint8_t uC_pktRcd;

#pragma pack(1)
typedef struct {
 uint8_t hr;
 uint8_t min;
 uint8_t sec;
}xtime_t;

typedef struct {
 uint8_t day;
 uint8_t month;
 uint16_t year;
}xdate_t;

typedef struct {
xtime_t time;
xdate_t date;
uint8_t valid;
}gnssTimeDateData_t;

typedef struct  {
float latitude;
float longitude;
float altitude;
uint8_t fix;
uint8_t satelite;
float hdop;
uint8_t valid;
}gnssLocData_t;

typedef struct  {
gnssTimeDateData_t gnssTimeData;
gnssLocData_t gnssLocData;
uint16_t batVolt;
uint8_t valid;
}gnssData_t;

typedef struct  {
 float voltage;
 float current;
 float power;
 uint16_t batVolt;
}pvc_t;

typedef union{
 gnssData_t gnss;
 pvc_t   pvc;
}ucAcGnss_t;

typedef struct 
{
uint8_t sync[3]; //SYN & PreAmble
ucAcGnss_t ucAcGnss;
uint8_t ackNo;
uint8_t ackStatus;
uint16_t  checksum;
uint8_t end[2];
}ucProtoPkt_t;

typedef union{
uint8_t ucData[39];
ucProtoPkt_t uCProtoPkt;
}acParamNu_t;


extern int uC_input_handler(unsigned char c);


#endif