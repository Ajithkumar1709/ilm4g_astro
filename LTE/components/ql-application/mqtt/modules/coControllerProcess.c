#include "ilmCommon.h"
#include "ql_uart.h"
#include "acParam.h"


//DEFINES
#define QL_UART_RX_BUFF_SIZE 2048
#define QL_UART_TX_BUFF_SIZE 2048
#define MIN(a, b)((a) < (b) ? (a) : (b))

#define D_DATA_BUFF_LENGTH 39
#define D_EXP_DATA_LENGTH 34
#define SYNC_LEN 2
#define END_LEN 2
#define uint8_t unsigned char



//GLOBALS AND STATICS
const uint8_t startSync[]={0xFF,0xDE};
uint8_t startSyncCnt=0;
const uint8_t endSync[]={0x2F,0x5C};
uint8_t endSyncCnt=0;
uint8_t dataBuffer[D_DATA_BUFF_LENGTH];
uint8_t dataBufferCnt=0;
uint8_t i=0;

volatile uint8_t uC_pktRcd;
uint8_t ptr[39];

unsigned char nuAcMeasCommand[] = "$1#";
unsigned char nuRlyOnCommand[] = "$on#";
unsigned char nuRlyOffCommand[] = "$off#";
unsigned char nuBatKill[] = "$BatKill#";
U1 uartPacketRcd=false;
//EXTERNS


void ql_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size) {
  unsigned char * recv_buff = calloc(1, QL_UART_RX_BUFF_SIZE + 1);
  unsigned int real_size = 0;
  int read_len = 0;

  QL_MQTT_LOG("-1-COCONT:UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
  switch (ind_type) {
  case QUEC_UART_RX_OVERFLOW_IND: //rx buffer overflow
  case QUEC_UART_RX_RECV_DATA_IND: {
    while (size > 0) {
      memset(recv_buff, 0, QL_UART_RX_BUFF_SIZE + 1);
      real_size = MIN(size, QL_UART_RX_BUFF_SIZE);

      read_len = ql_uart_read(port, recv_buff, real_size);
      memcpy(ptr, recv_buff, 39);
      uartPacketRcd=true;
      if ((read_len > 0) && (size >= read_len)) {
        size -= read_len;
      } else {
        break;
      }
    }
    break;
  }
  case QUEC_UART_TX_FIFO_COMPLETE_IND: {
    // QL_UART_DEMO_LOG("tx fifo complete");
    break;
  }
  }
  free(recv_buff);
  recv_buff = NULL;
}
void uart_init() {
  int ret = 0;
  ql_uart_config_s uart_cfg = {
    0
  };
  uart_cfg.baudrate = QL_UART_BAUD_115200;
  uart_cfg.flow_ctrl = QL_FC_NONE;
  uart_cfg.data_bit = QL_UART_DATABIT_8;
  uart_cfg.stop_bit = QL_UART_STOP_1;
  uart_cfg.parity_bit = QL_UART_PARITY_NONE;
  ret = ql_uart_set_dcbconfig(QL_UART_PORT_1, & uart_cfg);
  QL_MQTT_LOG("-1-COCONT:ret: 0x%x", ret);
  if (QL_UART_SUCCESS != ret) {
    QL_MQTT_LOG("-1-COCONT:uart fail\r\n");
  }
  ret = ql_uart_open(QL_UART_PORT_1);
  QL_MQTT_LOG("-1-COCONT:ret: 0x%x", ret);

  if (QL_UART_SUCCESS == ret) {
    ret = ql_uart_register_cb(QL_UART_PORT_1, ql_uart_notify_cb);
    QL_MQTT_LOG("-1-COCONT:ret: 0x%x", ret);

    memset( & uart_cfg, 0, sizeof(ql_uart_config_s));
    ret = ql_uart_get_dcbconfig(QL_UART_PORT_1, & uart_cfg);
    QL_MQTT_LOG("-1-COCONT:ret: 0x%x, baudrate=%d, flow_ctrl=%d, data_bit=%d, stop_bit=%d, parity_bit=%d",
      ret, uart_cfg.baudrate, uart_cfg.flow_ctrl, uart_cfg.data_bit, uart_cfg.stop_bit, uart_cfg.parity_bit);
  }
}


void uart_data_req(unsigned char * data) {
  int write_len = 0;
  ql_uart_tx_status_e tx_status;
  memset(ptr, 0, sizeof(ptr));
  write_len = ql_uart_write(QL_UART_PORT_1, data, strlen((char * ) data));
  QL_MQTT_LOG("-1-COCONT:write_len :%d", write_len);
  ql_uart_get_tx_fifo_status(QL_UART_PORT_1, & tx_status);
  QL_MQTT_LOG("-1-COCONT:tx_status:%d", tx_status);
}


enum coControllerState_e{UC_RUBBISH=1,UC_START,UC_SYNC,UC_END};


int uC_input_handler(unsigned char c) {
  acParamNu_t acParamNu;
  uint8_t * dataBuffer = & acParamNu.ucData[3];
  
  static char state = UC_RUBBISH;
//  QL_UART_DEMO_LOG("uc_input=%c",c);
  switch (state) {
  case UC_START:
    if (startSync[startSyncCnt] == c)
      startSyncCnt++;
    else {
   //   QL_MQTT_LOG("1st UC_RUBBISH");
      state = UC_RUBBISH;
    }
    if (startSyncCnt == SYNC_LEN) {
      // printf("Sync Recieved \n");
      state = UC_SYNC;
      endSyncCnt = 0;
      dataBufferCnt = 0;
    }
    break;
  case UC_SYNC:
    if (dataBufferCnt < D_DATA_BUFF_LENGTH) {
      i = dataBufferCnt;
      dataBuffer[dataBufferCnt++] = c;
     // printf(" %x\n", dataBuffer[i]);
    } else {
//QL_MQTT_LOG("2nd UC_RUBBISH");
      state = UC_RUBBISH;
    }
    if (dataBufferCnt == D_EXP_DATA_LENGTH)
      state = UC_END;
    break;

  case UC_END:

    if (endSync[endSyncCnt] == c)
      endSyncCnt++;
    else {
    //  QL_MQTT_LOG("3rd UC_RUBBISH");
      state = UC_RUBBISH;
    }
    if (endSyncCnt == END_LEN) {
      //   printf("end");
      uC_pktRcd = 1;
      state = UC_RUBBISH;
    }
    break;

  case UC_RUBBISH:
    if (c == 0xFF) {
      //printf("inside rubbish\r\n");
      state = UC_START;
      startSyncCnt = 0;
    }
    break;
  }
  return 0;
}

