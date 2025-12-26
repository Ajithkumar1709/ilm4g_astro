#ifndef RTC_H
#define RTC_H


#ifdef __cplusplus
extern "C" {
#endif

#include"ilmCommon.h"

#define RTC_SET 1
#define RTC_NOTSET 2

void ql_rtc_app_init(void);
void setUnknownRtcTime(void);
typedef struct {
  int Second;
  int Minute;
  int Hour;
  int Wday; // day of week, sunday is day 1
  int Day;
  int Month;
  int Year; // offset from 1970;
}
tm1Elements_t;


typedef uint32_t clock_time_t;
extern tm1Elements_t tm1;
extern rtc_t getRtc(void);
gnssElements_t gnss_data(void);

#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QL_RTC_DEMO_H */


