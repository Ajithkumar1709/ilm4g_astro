#include "ilmCommon.h"
#include "ql_api_rtc.h"
#include "rtc.h"
#include "ql_gnss.h"
#include "mqtt_demo.h"

//DEFINES
typedef unsigned long utime_t;

#define SECS_PER_MIN ((utime_t)(60))
#define SECS_PER_HOUR ((utime_t)(3600))
#define SECS_PER_DAY ((utime_t)(SECS_PER_HOUR * 24))
#define LEAP_YEAR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))

//GLOBALS AND STATICS
ql_gnss_data_t gpsData;
ql_gnss_data_t * gps_data = & gpsData;
ql_rtc_time_t rtcTm;
static
const uint8_t monthDays[] = {
  31,
  28,
  31,
  30,
  31,
  30,
  31,
  31,
  30,
  31,
  30,
  31
}; // API starts months from 1, this array starts from 0

//EXTERNS
extern ql_gnss_data_t g_gps_data;
extern ql_mutex_t mutex;

utime_t makeTime(tm1Elements_t tm1) {
  // assemble time elements into utime_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  int i;
  uint32_t seconds;
  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds = tm1.Year * (SECS_PER_DAY * 365);
  for (i = 0; i < tm1.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds += SECS_PER_DAY; // add extra days for leap years
    }
  }
  // add days for this year, months start from 1
  for (i = 1; i < tm1.Month; i++) {
    if ((i == 2) && LEAP_YEAR(tm1.Year)) {
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i - 1]; //monthDay array starts from 0
    }
  }
  seconds += (tm1.Day - 1) * SECS_PER_DAY;
  seconds += tm1.Hour * SECS_PER_HOUR;
  seconds += tm1.Minute * SECS_PER_MIN;
  seconds += tm1.Second;
  return (utime_t) seconds;
}

void setUnknownRtcTime(void) {
  QL_MQTT_LOG("-1-RTCLOG:SET UNKNOWN RTC TIME");
  ql_rtc_time_t tm;
  tm.tm_year = 2001; // Year - 1900
  tm.tm_mon = 2; // Month, where 0 = jan
  tm.tm_mday = 2; // Day of the month
  tm.tm_hour = 2; //hour
  tm.tm_min = 2; //min
  tm.tm_sec = 2; //sec
  ql_rtc_set_time( & tm);
}

rtc_t timeConversion(ql_rtc_time_t tm) {
  int day_hour, day_min, day_sec = 0;
  unsigned long long int day_Totsec = 0;
  tm1Elements_t tm1;
  rtc_t rtc;
  tm1.Year = tm.tm_year - 1970; // Year - 1900
  tm1.Month = tm.tm_mon; // Month, where 0 = jan
  tm1.Day = tm.tm_mday; // Day of the month
  tm1.Hour = tm.tm_hour; //hour
  tm1.Minute = tm.tm_min; //min
  tm1.Second = tm.tm_sec; //sec
  rtc.timeInSec = makeTime(tm1);
  rtc.timeInSec = rtc.timeInSec - 19800;
  day_hour = tm1.Hour * 3600;
  day_min = tm1.Minute * 60;
  day_sec = tm1.Second;
  day_Totsec = day_hour + day_min + day_sec;
  rtc.daySecs = day_Totsec;
  rtc.day = tm1.Day;
  return rtc;
}
/*
rtc_t getRtc(void) {
  static rtc_t rtc;
  ql_rtc_time_t tm;
  //get rtc 
  ql_rtc_get_time( & tm);
  ql_rtc_set_timezone(22); //UTC+32
  ql_rtc_get_localtime( & tm);
  rtc = timeConversion(tm);
   QL_MQTT_LOG("-1-RTCLOG: tm.tm_year:%d",  tm.tm_year);
  if (rtc.timeInSec < 1693542216) {
    QL_MQTT_LOG("-1-RTCLOG:TIME NOT VALID:%ld", rtc.timeInSec);
    ql_rtos_mutex_lock((ql_mutex_t) mutex, QL_WAIT_FOREVER);
    gpsData = g_gps_data;
    ql_rtos_mutex_unlock((ql_mutex_t) mutex);

    if ((gps_data -> quality == 1) && (gps_data -> valid == 1)) {
      QL_MQTT_LOG("-1-RTCLOG:LOADING FROM GPS DATA");
      tm.tm_year = (gps_data -> time.tm_year + 2000); // Year - 1900
      tm.tm_mon = gps_data -> time.tm_mon; // Month, where 0 = jan 
      tm.tm_mday = gps_data -> time.tm_mday; // Day of the month                
      tm.tm_hour = gps_data -> time.tm_hour; //hour               
      tm.tm_min = gps_data -> time.tm_min; //min               
      tm.tm_sec = gps_data -> time.tm_sec; //sec
      QL_MQTT_LOG("-1-RTCLOG:GPS TIME YEAR %ld,MON %ld,DAY %ld,HOUR %ld,MIN %ld,SEC %ld, ",
        tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
      ql_rtc_set_time( & tm);
      ql_rtc_set_timezone(22); //UTC+32
      ql_rtc_get_localtime( & tm);
      rtc = timeConversion(tm);
      QL_MQTT_LOG("-1-RTCLOG:LOADED GPS TIME:%ld", rtc.timeInSec);
      QL_MQTT_LOG("-1-RTCLOG: tm.tm_year:%d",  tm.tm_year);
      if (rtc.timeInSec > 1693542216) {
        QL_MQTT_LOG("-1-RTCLOG:RTC_SET BY GPS:%ld", rtc.timeInSec);
        rtc.status = RTC_SET;
      } else {
        QL_MQTT_LOG("-1-RTCLOG:RTC_NOTSET , GPS TIME INVALID:%ld", rtc.timeInSec);
        rtc.status = RTC_NOTSET;
      }
      return rtc;
    } else {
      rtc.status = RTC_NOTSET;
      QL_MQTT_LOG("-1-RTCLOG:NO VALID RTC : %ld", rtc.timeInSec);
      return rtc;
    }
  }
  rtc.status = RTC_SET;
  QL_MQTT_LOG("-1-RTCLOG: RTC_SET RTC TIME IN SEC: %ld ,%ld", rtc.timeInSec, rtc.daySecs);
  return rtc;
}
*/
rtc_t getRtc(void) {
  static rtc_t rtc;
  ql_rtc_time_t tm;
  //get rtc
  ql_rtc_get_time( & tm);
  ql_rtc_set_timezone(22); //UTC+32
  rtcTm=tm;
  ql_rtc_get_localtime( & tm);
  rtc = timeConversion(tm);
  if (rtc.timeInSec < 1693542216) {
    QL_MQTT_LOG("-1-RTCLOG:TIME NOT VALID:%ld", rtc.timeInSec);
    ql_rtos_mutex_lock((ql_mutex_t) mutex, QL_WAIT_FOREVER);
    gpsData = g_gps_data;
    ql_rtos_mutex_unlock((ql_mutex_t) mutex);

    if ((gps_data -> quality == 1) && (gps_data -> valid == 1)) {
      QL_MQTT_LOG("-1-RTCLOG:LOADING FROM GPS DATA");
      tm.tm_year = (gps_data -> time.tm_year + 2000); // Year - 1900
      tm.tm_mon = gps_data -> time.tm_mon; // Month, where 0 = jan
      tm.tm_mday = gps_data -> time.tm_mday; // Day of the month                
      tm.tm_hour = gps_data -> time.tm_hour; //hour              
      tm.tm_min = gps_data -> time.tm_min; //min              
      tm.tm_sec = gps_data -> time.tm_sec; //sec
      QL_MQTT_LOG("-1-RTCLOG:GPS TIME YEAR %ld,MON %ld,DAY %ld,HOUR %ld,MIN %ld,SEC %ld, ",
        tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
      ql_rtc_set_time( & tm);
      ql_rtc_set_timezone(22); //UTC+32
       rtcTm=tm;
      ql_rtc_get_localtime( & tm);
      rtc = timeConversion(tm);
    
      QL_MQTT_LOG("-1-RTCLOG:LOADED GPS TIME:%ld", rtc.timeInSec);
      if (rtc.timeInSec > 1693542216) {
        QL_MQTT_LOG("-1-RTCLOG:RTC_SET BY GPS:%ld", rtc.timeInSec);
        rtc.status = RTC_SET;
      } else {
        QL_MQTT_LOG("-1-RTCLOG:RTC_NOTSET , GPS TIME INVALID:%ld", rtc.timeInSec);
        rtc.status = RTC_NOTSET;
      }
      return rtc;
    } else {
      rtc.status = RTC_NOTSET;
      QL_MQTT_LOG("-1-RTCLOG:NO VALID RTC : %ld", rtc.timeInSec);
      return rtc;
    }
  }
  rtc.status = RTC_SET;
  QL_MQTT_LOG("-1-RTCLOG: RTC_SET RTC TIME IN SEC: %ld ,%ld", rtc.timeInSec, rtc.daySecs);
  return rtc;
}