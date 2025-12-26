#include "ilmCommon.h"
#include "fsDriver.h"

//DEFINES

//GLOBALS AND STATICS

int err;
//EXTERNS


U2 fileWrite(QFILE * fd, U2 fileIndex, fsDataBackUp_t * fileData) {
  err = ql_fseek( * fd, ((sizeof(fsDataBackUp_t)) * (fileIndex)), SEEK_SET); //272//274
  QL_MQTT_LOG("FSDRIVELOG:fsDataWrite- File write seek err %d  size written %d  FileIndex %d ", err, sizeof(fsDataBackUp_t), fileIndex);
  err = ql_fwrite(fileData, sizeof(fsDataBackUp_t), 1, * fd); //==274//273
  if (err < 0) {
    QL_MQTT_LOG("FSDRIVELOG:write file failed %d ",fileIndex);
    return 0;
  }
  return 1;
}

U2 fileRead(QFILE * fd, U2 fileIndex, fsDataBackUp_t * fileData) {

  ql_fseek( * fd, ((sizeof(fsDataBackUp_t)) * fileIndex), SEEK_SET);
  err = ql_fread(fileData, sizeof(fsDataBackUp_t), 1, * fd);

  if (err < 0) {
    QL_MQTT_LOG("FSDRIVELOG:read failed failed %d ",fileIndex);
    return 0;
  } else {
    QL_MQTT_LOG("FSDRIVELOG:read pass failed %d ",fileIndex);
  }
  return 1;
}


