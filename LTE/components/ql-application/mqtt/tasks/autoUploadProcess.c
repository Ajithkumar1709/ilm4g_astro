#include "ilmCommon.h"
#include "fsDriver.h"
#include "memoryHandler.h"

//DEFINES

//GLOBALS AND STATICS

//EXTERNS
extern int  mqtt_connected;
extern ql_queue_t ql_mqttSend_queueHandle;
extern ql_mutex_t dataFileMutex;
extern char dataLogFile[];
extern uint8_t crc8_cal(uint8_t * data, uint16_t length) ;

void payLoadAutoUpload(U2 currentFileIndex) {
  mqttQueue_t mqttSendData;
  fsDataBackUp_t fileData;
  uint8_t crc_temp;
  U2 index = 0;
  U1 status=0;
  int dataFile;
  index = currentFileIndex;
  for (int i = 0; i < 100; i++) {
      ql_rtos_mutex_lock((ql_mutex_t)dataFileMutex, QL_WAIT_FOREVER);
      dataFile = ql_fopen(dataLogFile, "ab+");
      ql_fclose(dataFile);
      memset( & fileData, 0, sizeof(fileData));
      status=fsDataRead( & fileData, index);
      ql_rtos_mutex_unlock((ql_mutex_t)dataFileMutex);
    if (status){
      crc_temp = fileData.crc;
      fileData.crc = 0;
      fileData.crc = crc8_cal((uint8_t * ) & fileData, sizeof(fileData));
      if ((crc_temp != 0) && (fileData.crc != 0)) {
        if (crc_temp == fileData.crc) {
         if (fileData.uploadStatus == D_NOTUPLOADED) 
          {
            QL_MQTT_LOG("-1-AUTOUPLOADLOG:fsAutoUpload index=%d",index);
            memset(mqttSendData.data, 0, sizeof(mqttSendData.data));
            memcpy(mqttSendData.data, fileData.data, fileData.dataLength);
            mqttSendData.dataSize = fileData.dataLength;
            mqttSendData.topic = 1;
            mqttSendData.mqttFsInfo.index=index;
            mqttSendData.mqttFsInfo.fsUpdateRequired = 1;
            ql_rtos_queue_release(ql_mqttSend_queueHandle, sizeof(mqttSendData), (uint8 * ) & mqttSendData, 0);
            ql_rtos_task_sleep_s(5);
          }
        }
      }
    }else{
       ql_rtos_task_sleep_s(1);
    }
    if (index)
      index--;
    else
      index = D_MAX_FILEINDEX;
  }

}

void autoUploadProcess_thread(void * param) {
  extern U2 currentFileIndex;
  ql_rtos_task_sleep_s(3);
  QL_MQTT_LOG("-1-PSTART:autoUploadProcess_task started");
  while(1){
    if(mqtt_connected==1)
    {
     QL_MQTT_LOG("-1-AUTOUPLOADLOG:autoUploadProcess_task current_index=%d",currentFileIndex);
     payLoadAutoUpload(currentFileIndex);
     ql_rtos_task_sleep_s(60);
    }
    ql_rtos_task_sleep_s(60);
   
  }
}