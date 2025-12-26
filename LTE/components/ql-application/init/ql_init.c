/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_app_feature_config.h"
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_pin_cfg.h"
#include "nw_demo.h"
//#include "gpio_demo.h"
//#include "gpio_int_demo.h"
#include "datacall_demo.h"
#include "osi_demo.h"
#include "ql_dev_demo.h"
//#include "adc_demo.h"
#include "led_cfg_demo.h"
#include "ql_sim_demo.h"
#include "power_demo.h"
#include "ql_api_dev.h"
#include "ql_fs_demo.h"
//#include "gprs_data_transfer_demo.h"
//#include"application.h"
//#include"gpio_conf.h"
//#include"AC_param.h"
#ifdef QL_APP_FEATURE_MMS
#include "mms_demo.h"
#include"mqtt_demo.h"
#include"gnss_demo.h"

#endif

#ifdef QL_APP_FEATURE_UART
#include "uart_demo.h"
#endif
#ifdef QL_APP_FEATURE_RS485
#include "rs485_demo.h"
#endif
#ifdef QL_APP_FEATURE_AUDIO
#include "audio_demo.h"
#endif
#ifdef QL_APP_FEATURE_LCD
#include "lcd_demo.h"
#endif
#ifdef QL_APP_FEATURE_LVGL
#include "lvgl_demo.h"
#endif
#ifdef QL_APP_FEATURE_HTTP
#include "http_demo.h"
#endif
#ifdef QL_APP_FEATURE_FTP
#include "ftp_demo.h"
#endif
#ifdef QL_APP_FEATURE_MQTT
#include "mqtt_demo.h"
#endif
#ifdef QL_APP_FEATURE_SSL
#include "ssl_demo.h"
#endif
#ifdef QL_APP_FEATURE_PING
#include "ping_demo.h"
#endif
#ifdef QL_APP_FEATURE_NTP
#include "ntp_demo.h"
#endif
#ifdef QL_APP_FEATURE_LBS
#include "lbs_demo.h"
#endif
#ifdef QL_APP_FEATURE_SOCKET
#include "socket_demo.h"
#endif


#ifdef QL_APP_FEATURE_FILE_ZIP
#include "zip_demo.h"
#endif

#ifdef QL_APP_FEATURE_BT
//#include "bt_demo.h"
#ifdef QL_APP_FEATURE_BT_HFP
#include "bt_hfp_demo.h"
#endif
#ifdef QL_APP_FEATURE_BT_A2DP_AVRCP
#include "bt_a2dp_avrcp_demo.h"
#endif
#ifdef QL_APP_FEATURE_BLE_GATT
#include "ble_gatt_demo.h"
#endif

#endif

#ifdef QL_APP_FEATURE_FILE
#include "ql_fs_demo.h"
#endif
#ifdef QL_APP_FEATURE_SMS
#include "sms_demo.h"
#endif
#ifdef QL_APP_FEATURE_VOICE_CALL
#include "voice_call_demo.h"
#endif
#ifdef QL_APP_FEATURE_GNSS
#include "gnss_demo.h"
#endif

#ifdef QL_APP_FEATURE_SPI
#include "spi_demo.h"
#endif

#ifdef QL_APP_FEATURE_SPI_NOR_FLASH
#include "spi_nor_flash_demo.h"
#endif

#ifdef QL_APP_FEATURE_SPI6_EXT_NOR
#include "spi6_ext_nor_flash_demo.h"
#endif

#ifdef QL_APP_FEATURE_SPI_NAND_FLASH
#include "spi_nand_flash_demo.h"
#endif


#ifdef QL_APP_FEATURE_CAMERA
#include "camera_demo.h"
#endif

#ifdef QL_APP_FEATURE_WIFISCAN
#include "wifi_scan_demo.h"
#endif

#ifdef QL_APP_FEATURE_HTTP_FOTA
#include "fota_http_demo.h"
#endif

#ifdef QL_APP_FEATURE_FTP_FOTA
#include "fota_ftp_demo.h"
#endif

#ifdef QL_APP_FEATURE_DECODER
#include "decoder_demo.h"
#endif

#ifdef QL_APP_FEATURE_VOLTE
#include "volte_demo.h"
#endif

#ifdef QL_APP_FEATURE_KEYPAD
#include "keypad_demo.h"
#endif

#ifdef QL_APP_FEATURE_RTC
#include "ql_rtc_demo.h"
#endif

#ifdef QL_APP_FEATURE_SECURE_BOOT
#include "ql_api_dev.h"
#endif

#ifdef QL_APP_FEATURE_USB_CHARGE
#include "charge_demo.h"
#endif

#ifdef QL_APP_FEATURE_QCLOUD_IOT
#include "qcloud_demo.h"
#endif

#ifdef QL_APP_FEATURE_VIRT_AT
#include "ql_virt_at_demo.h"
#endif

#ifdef QL_APP_FEATURE_TTS
#include "tts_demo.h"
#endif

#ifdef QL_APP_FEATURE_I2C
#include "I2C_demo.h"
#endif

#ifdef QL_APP_FEATURE_PBK
#include "ql_pbk_demo.h"
#endif

#ifdef QL_APP_FEATURE_SDMMC
#include "ql_sdmmc_demo.h"
#endif

#ifdef QL_APP_FEATURE_ALI_LINKSDK
#include "ali_demo.h"
#endif

#ifdef QL_APP_FEATURE_USBNET
#include "usbnet_demo.h"
#endif

#ifdef QL_APP_FEATURE_FS_NAND_FLASH
#include "fs_nand_flash_demo.h"
#endif

#ifdef QL_APP_FEATURE_SFTP
#include "sftp_demo.h"
#endif

#ifdef QL_APP_FEATURE_MXML
#include "mxml_demo.h"
#endif

#ifdef QL_APP_FEATURE_CLOUDOTA
#include "cloudota_demo.h"
#endif

#ifdef QL_APP_FEATURE_EMBED_NOR_FLASH
#include "embed_nor_flash_demo.h"
#endif

#ifdef QL_APP_FEATURE_HILINK
#include "hilink.h"
#endif

#ifdef QL_APP_FEATURE_PSM
#include "psm_demo.h"
#endif

#ifdef QL_APP_FEATURE_GPRS_DATA_TRANSFER
#include "gprs_data_transfer_demo.h"
#endif

#define QL_INIT_LOG_LEVEL	QL_LOG_LEVEL_INFO
#define QL_INIT_LOG(msg, ...)			QL_LOG(QL_INIT_LOG_LEVEL, "ql_INIT", msg, ##__VA_ARGS__)
#define QL_INIT_LOG_PUSH(msg, ...)	QL_LOG_PUSH("ql_INIT", msg, ##__VA_ARGS__)

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

void ql_pin_cfg_init(void)
{
    uint8_t       index        = 0;
    uint8_t       pin_num      = 0;
    uint8_t       default_func = 0;
    uint8_t       gpio_func    = 0;
    ql_GpioNum    gpio_num     = 0;
    ql_GpioDir    gpio_dir     = 0;
    ql_PullMode   gpio_pull    = 0;
    ql_LvlMode    gpio_lvl     = 0;

    for( index = 0; index < QL_GPIO_PIN_MAX; index++ )
    {
        //QL_INIT_LOG("pin%d=%d", index, ql_pin_cfg_map[index].pin_num);
	    if (QUEC_PIN_NONE == ql_pin_cfg_map[index].pin_num)
        {
            QL_INIT_LOG("init exit %d!", index);
            break;
        }
        pin_num      = ql_pin_cfg_map[index].pin_num;
        default_func = ql_pin_cfg_map[index].default_func;
        gpio_func    = ql_pin_cfg_map[index].gpio_func;
        gpio_num     = ql_pin_cfg_map[index].gpio_num;
        gpio_dir     = ql_pin_cfg_map[index].gpio_dir;
        gpio_pull    = ql_pin_cfg_map[index].gpio_pull;
        gpio_lvl     = ql_pin_cfg_map[index].gpio_lvl;

        ql_pin_set_func(pin_num, default_func);
        if( default_func == gpio_func )
        {
            ql_gpio_init(gpio_num, gpio_dir, gpio_pull, gpio_lvl);
        }
    }
}

static void ql_init_demo_thread(void *param)
{
    QL_INIT_LOG("init demo thread enter, param 0x%x", param);

/*Caution:If the macro of secure boot and the function are opened, download firmware and restart will enable secure boot.
          the secret key cannot be changed forever*/    
#ifdef QL_APP_FEATURE_SECURE_BOOT
    //ql_dev_enable_secure_boot();
#endif

#if 1
//ql_gprs_data_transfer_app_init();
//ql_application_app_init();
//ql_fs_demo_init();
//ql_rtc_app_init();
//ql_ledcfg_app_init();
 //   ql_gpio_app_init();
//ql_gpioint_app_init();
//ql_uart_app_init();
//ql_adc_app_init();
//ql_application_app_init();
//ql_gnss_app_init();
//ql_mqtt_app_init();
ql_mqtt_app_init();
// ql_fs_demo_init();
//ql_fs_demo_init();
//ql_gprs_data_transfer_app_init();
//ql_datacall_app_init();
//ql_nw_app_init();
#endif

#ifdef QL_APP_FEATURE_LEDCFG
ql_ledcfg_app_init();
#endif

#ifdef QL_APP_FEATURE_AUDIO
    //ql_audio_app_init();
#ifdef QL_APP_FEATURE_HEADSET_DET
    //ql_headset_det_app_init();
#endif
#endif

#ifdef QL_APP_FEATURE_TTS
/*
 *demo1直接调用TTS库和audio播放api进行TTS播放，
 *demo2对TTS库和audio的播放api进行封装，封装后的TTS api与我司asr系列的TTS api类似
 *注意：由于audio播放器只有一个，因此这两个demo只能运行一个，不能两个demo同时播放
 *用户如需将TTS库放到内核，则在target.config中将 CONFIG_QUEC_PROJECT_FEATURE_TTS_IN_KERNEL 置为 y,
  并修改分区，将内核分区增大250k左右，其余的内容不变
 */
	//ql_tts_demo1_init();
	//ql_tts_demo2_init();
#endif

#ifdef QL_APP_FEATURE_LCD
    //ql_lcd_app_init();
#endif
#ifdef QL_APP_FEATURE_LVGL
    //ql_lvgl_app_init();
#endif
    //ql_nw_app_init();
    //ql_datacall_app_init();
	//ql_osi_demo_init();	
	
#ifdef QL_APP_FEATURE_SDMMC
	//ql_sdmmc_app_init();
#endif

#ifdef QL_APP_FEATURE_FILE
	//ql_fs_demo_init();
#endif

    //ql_dev_app_init();
    //ql_adc_app_init();
    
#ifdef QL_APP_FEATURE_UART
    //ql_uart_app_init();
#endif

#ifdef QL_APP_FEATURE_RS485
    //ql_rs485_app_init();
#endif

#ifdef QL_APP_FEATURE_BT
	//ql_bt_spp_server_demo_init();
	//ql_bt_spp_client_demo_init()
#ifdef QL_APP_FEATURE_BLE_GATT
	//ql_ble_gatt_server_demo_init();
    //ql_ble_gatt_client_demo_init();
#endif
#ifdef QL_APP_FEATURE_BT_HFP
    //ql_bt_hfp_demo_init();
#endif
#ifdef QL_APP_FEATURE_BT_A2DP_AVRCP
    //ql_bt_a2dp_avrcp_app_init();
#endif
#endif
    //ql_sim_app_init();
    ql_power_app_init();
#ifdef QL_APP_FEATURE_PSM
    //ql_psm_app_init();
#endif

#ifdef QL_APP_FEATURE_CLOUDOTA    
	//ql_cloudota_app_init();
#endif

#ifdef QL_APP_FEATURE_PWK
    //ql_pwrkey_app_init();
#endif

#ifdef QL_APP_FEATURE_FTP
	//ql_ftp_app_init();
#endif
#ifdef QL_APP_FEATURE_HTTP
	//ql_http_app_init();
	//ql_http_post_app_init();
	//ql_http_put_app_init();
	//ql_https_get_app_init();
#endif

#ifdef QL_APP_FEATURE_MQTT
	//ql_mqtt_app_init();
#endif
#ifdef QL_APP_FEATURE_SSL
	//ql_ssl_app_init();
#endif
#ifdef QL_APP_FEATURE_PING
	//ql_ping_app_init();
#endif
#ifdef QL_APP_FEATURE_NTP
	//ql_ntp_app_init();
#endif
#ifdef QL_APP_FEATURE_LBS
	//ql_lbs_app_init();
#endif
#ifdef QL_APP_FEATURE_SOCKET
	//ql_socket_app_init();
#endif
#ifdef QL_APP_FEATURE_MMS
   //ql_mms_app_init();
#endif

#ifdef QL_APP_FEATURE_SMS
	//ql_sms_app_init();
#endif

#ifdef QL_APP_FEATURE_PBK
    //ql_pbk_app_init();
#endif

#ifdef QL_APP_FEATURE_VOICE_CALL
	//ql_voice_call_app_init();
#endif
#ifdef QL_APP_FEATURE_VOLTE
	//ql_volte_app_init();
#endif

#ifdef QL_APP_FEATURE_GNSS
    //ql_gnss_app_init();
#endif

#ifdef QL_APP_FEATURE_FILE_ZIP
    //ql_zip_app_init();
#endif

#ifdef QL_APP_FEATURE_SPI
    //ql_spi_demo_init();
#endif

#ifdef QL_APP_FEATURE_CAMERA
    /*因为camera被i2c用来演示功能，所以打开camera demo时不可同时打开i2c demo*/
    /*because camera as a demonstration for i2c, so camera demo can not be opened when the i2c demo open */
    //ql_camera_app_init();
#endif

#ifdef QL_APP_FEATURE_SPI_NOR_FLASH
    //ql_spi_nor_flash_demo_init();
#endif

#ifdef QL_APP_FEATURE_SPI6_EXT_NOR
    //ql_spi6_ext_nor_flash_demo_init();
#endif

#ifdef QL_APP_FEATURE_SPI_NAND_FLASH
    //ql_spi_nand_flash_demo_init();
#endif


#if defined (QL_APP_FEATURE_WIFISCAN)
    //ql_wifiscan_app_init();
#endif

#ifdef QL_APP_FEATURE_HTTP_FOTA
	//ql_fota_http_app_init();
#endif

#ifdef QL_APP_FEATURE_FTP_FOTA
    //ql_fota_ftp_app_init();
#endif

#ifdef QL_APP_FEATURE_DECODER
    //ql_decoder_app_init();
#endif

#ifdef QL_APP_FEATURE_KEYPAD
    //ql_keypad_app_init();
#endif

#ifdef QL_APP_FEATURE_RTC
    //ql_rtc_app_init();
#endif

#ifdef QL_APP_FEATURE_USB_CHARGE
    //ql_charge_app_init();
#endif

#ifdef QL_APP_FEATURE_ALI_LINKSDK
	//ql_ali_sdk_mqtt_client_test_init();		/* mqtt_basic_demo.c       */
	//ql_ali_sdk_bootstrap_test_init();			/* bootstrap_posix_demo.c  */
	//ql_ali_sdk_cota_test_init();				/* cota_basic_demo.c       */
	//ql_ali_sdk_das_test_init();				/* das_basic_demo.c        */
	//ql_ali_sdk_data_model_test_init();		/* data_model_basic_demo.c */
	//ql_ali_sdk_devinfo_test_init();			/* devinfo_posix_demo.c    */
	//ql_ali_sdk_diag_test_init();				/* diag_basic_demo.c       */
	//ql_ali_sdk_dynreg_test_init();			/* dynreg_basic_demo.c     */
	//ql_ali_sdk_dynregmq_test_init();			/* dynregmq_basic_demo.c   */
	//ql_ali_sdk_fota_test_init();				/* fota_basic_demo.c       */
	//ql_ali_sdk_http_client_test_init();		/* http_basic_demo.c       */
	//ql_ali_sdk_logpost_test_init();			/* logpost_basic_demo.c    */
	//ql_ali_sdk_mqtt_broadcast_test_init();	/* mqtt_broadcast_demo.c   */
	//ql_ali_sdk_rrpc_test_init();				/* mqtt_rrpc_demo.c        */
	//ql_ali_sdk_ntp_test_init();				/* ntp_posix_demo.c        */
	//ql_ali_sdk_shadow_test_init();			/* shadow_basic_demo.c     */
#endif

#ifdef QL_APP_FEATURE_QCLOUD_IOT
    //light_data_template_sample_start();/* light_data_template_sample_start_demo.c  */
	//mqtt_sample_start();			   /* mqtt_sample_start_demo.c  */
    //data_template_sample_start();      /* data_template_sample_start_demo.c  */
	//ota_mqtt_sample_start();		   /* ota_mqtt_sample_start_demo.c  */
    //raw_data_sample_start();           /* raw_data_sample_start_demo.c  */
    //dynreg_dev_sample_start();         /* dynreg_dev_sample_start_demo.c  */
    //gateway_sample_start();            /* gateway_sample_start_demo.c  */
#endif
	
#ifdef QL_APP_FEATURE_VIRT_AT
    //ql_virt_at_app_init();
#endif

#ifdef QL_APP_FEATURE_I2C
    /*因为i2c通过camera来演示功能，所以打开i2c demo时不可同时打开camera demo*/
    /*because I2C uses the camera as a demonstration, so i2c demo can not be opened when the camera demo open */
    //ql_i2c_demo_init();      
#endif

#ifdef QL_APP_FEATURE_USBNET
	//ql_usbnet_app_init();
#endif

#ifdef QL_APP_FEATURE_FS_NAND_FLASH
    //ql_fs_nand_flash_demo_init();
#endif

#ifdef QL_APP_FEATURE_SFTP
	//ql_sftp_app_init();
#endif

#ifdef QL_APP_FEATURE_MXML
	//ql_mxml_app_init();
#endif

#ifdef QL_APP_FEATURE_EMBED_NOR_FLASH
    //ql_embed_nor_flash_app_init();
#endif

#ifdef QL_APP_FEATURE_HILINK
    //hilink_main();
#endif

#ifdef QL_APP_FEATURE_GPRS_DATA_TRANSFER
	//ql_gprs_data_transfer_app_init();
#endif
    ql_rtos_task_sleep_ms(1000); /*Chaos change: set to 1000 for the camera power on*/
    ql_rtos_task_delete(NULL);
}

int appimg_enter(void *param)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t ql_init_task = NULL;
	
    QL_INIT_LOG("init demo enter: %s @ %s", QL_APP_VERSION, QL_APP_BUILD_RELEASE_TYPE);
    prvInvokeGlobalCtors();
    if(0 == strcasecmp(QL_APP_BUILD_RELEASE_TYPE, "release"))
    {
    	ql_dev_cfg_wdt(1);
        //open the kernel log
    	//ql_quec_trace_enable(1);
    }
    else
    {
    	ql_dev_cfg_wdt(0);
        //close the kernel log
    	//ql_quec_trace_enable(0);
    }

    /*Caution: GPIO pin must be initialized here, otherwise the pin status cannot be determined*/
    ql_pin_cfg_init();
	
    err = ql_rtos_task_create(&ql_init_task, 1024*4, APP_PRIORITY_NORMAL, "ql_init", ql_init_demo_thread, NULL, 1);
	if(err != QL_OSI_SUCCESS)
    {
		QL_INIT_LOG("init failed");
	}
    
    return err;
}

void appimg_exit(void)
{
    QL_INIT_LOG("init demo exit");
}


