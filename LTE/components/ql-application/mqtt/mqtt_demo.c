#include "ilmCommon.h"

//DEFINES

//GLOBALS AND STATICS

//TASKS
ql_task_t mqtt_task = NULL;
static ql_task_t ilm_task = NULL;
static ql_task_t periodic_task = NULL;
ql_task_t onBoot_task = NULL;
static ql_task_t autoUpload_task = NULL;
ql_task_t gnss_task = NULL;

//MUTEX
ql_mutex_t mutex = NULL;
ql_mutex_t dataFileMutex = NULL;
//QUEUE
ql_queue_t ql_mqttSend_queueHandle = NULL;
ql_queue_t ql_ilmProcess_qHandle = NULL;
ql_queue_t ql_payloadProcess_qHandle = NULL;
U4 pwrr;

//EXTERNS
extern void ilmProcess_thread(void * param);
extern void periodicProcess_thread(void * param);
extern void mqttProcess_thread(void * arg);
extern void ql_gnss_thread(void * param);
extern void onBootProcess_thread(void * param);
extern void autoUploadProcess_thread(void * param);
extern void setUnknownRtcTime(void) ;


int ql_mqtt_app_init(void) {
  QlOSStatus err = QL_OSI_SUCCESS;
  setUnknownRtcTime();

  //mutex Creation
  ql_rtos_mutex_create( & mutex);
  ql_rtos_mutex_create( & dataFileMutex);

  //queue Creation
  ql_rtos_queue_create( & ql_mqttSend_queueHandle, sizeof(mqttQueue_t), 5);

  err = ql_rtos_task_create( & ilm_task, 24 * 1024, APP_PRIORITY_NORMAL, "ilm_app", ilmProcess_thread, NULL, 5);
  if (err != QL_OSI_SUCCESS) {
    QL_MQTT_LOG("mqtt_app init failed");
  }
  err = ql_rtos_task_create( & periodic_task, 24 * 1024, APP_PRIORITY_NORMAL, "periodic_app", periodicProcess_thread, NULL, 5);
  if (err != QL_OSI_SUCCESS) {
    QL_MQTT_LOG("mqtt_app init failed");
  }
  err = ql_rtos_task_create( & mqtt_task, 24 * 1024, APP_PRIORITY_NORMAL, "mqtt_app", mqttProcess_thread, NULL, 5);
  if (err != QL_OSI_SUCCESS) {
    QL_MQTT_LOG("mqtt_app init failed");
  }
  err = ql_rtos_task_create( & gnss_task, 4096, APP_PRIORITY_NORMAL, "ql_gnssdemo", ql_gnss_thread, NULL, 5);
  if (err != QL_OSI_SUCCESS) {
    QL_MQTT_LOG("mqtt_app init failed");
  }

  err = ql_rtos_task_create( & onBoot_task, 4096, APP_PRIORITY_NORMAL, "ql_onBoot_Task", onBootProcess_thread, NULL, 5);
  if (err != QL_OSI_SUCCESS) {
    QL_MQTT_LOG("mqtt_app init failed");
  }

  err = ql_rtos_task_create( & autoUpload_task, 4096, APP_PRIORITY_NORMAL, "autoUpload_task", autoUploadProcess_thread, NULL, 5);
  if (err != QL_OSI_SUCCESS) {
    QL_MQTT_LOG("mqtt_app init failed");
  }
  return err;
}