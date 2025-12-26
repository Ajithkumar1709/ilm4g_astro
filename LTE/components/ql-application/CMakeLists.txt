# Copyright (C) 2020 QUECTEL Technologies Limited and/or its affiliates("QUECTEL").
# All rights reserved.
#

add_subdirectory_if_exist(init)

add_subdirectory_if_exist(nw)

add_subdirectory_if_exist(peripheral)

add_subdirectory_if_exist(osi)

add_subdirectory_if_exist(dev)

add_subdirectory_if_exist(power)
#add_subdirectory_if_exist(power)
#add_subdirectory_if_exist(fs)

add_subdirectory_if_exist(qnewlib)
add_subdirectory_if_exist(my_app_code)
add_subdirectory_if_exist(mqtt)
add_subdirectory_if_exist(gprs_data_transfer)


if(QL_APP_FEATURE_FILE)
add_subdirectory_if_exist(fs)
endif()

if(QL_APP_FEATURE_AUDIO)
add_subdirectory_if_exist(audio)
if(QL_APP_FEATURE_TTS)
add_subdirectory_if_exist(tts)
endif()
endif()

if(QL_APP_FEATURE_BT)
add_subdirectory_if_exist(bt)
endif()

if(QL_APP_FEATURE_GNSS)
add_subdirectory_if_exist(gnss)
endif()

if(QL_APP_FEATURE_FTP)
add_subdirectory_if_exist(ftp)
endif()

if(QL_APP_FEATURE_HTTP)
add_subdirectory_if_exist(http)
endif()

if(QL_APP_FEATURE_MMS)
add_subdirectory_if_exist(mms)
endif()

#if(QL_APP_FEATURE_MQTT)
#add_subdirectory_if_exist(mqtt)
#endif()

if(QL_APP_FEATURE_SSL)
add_subdirectory_if_exist(ssl)
endif()

if(QL_APP_FEATURE_PING)
add_subdirectory_if_exist(ping)
endif()

if(QL_APP_FEATURE_NTP)
add_subdirectory_if_exist(ntp)
endif()

if(QL_APP_FEATURE_LBS)
add_subdirectory_if_exist(lbs)
endif()

if(QL_APP_FEATURE_CTSREG)
add_subdirectory_if_exist(ctsreg)
endif()

if(QL_APP_FEATURE_SOCKET)
add_subdirectory_if_exist(socket)
endif()

if(QL_APP_FEATURE_LCD)
add_subdirectory_if_exist(lcd)
endif()

if(QL_APP_FEATURE_LVGL)
add_subdirectory_if_exist(lvgl_lib)
add_subdirectory_if_exist(lvgl)
endif()

if(QL_APP_FEATURE_CAMERA)
add_subdirectory_if_exist(camera)
endif()

if(QL_APP_FEATURE_WIFISCAN)
add_subdirectory_if_exist(wifi_scan)
endif()

if(QL_APP_FEATURE_SIM)
add_subdirectory_if_exist(sim)
endif()

if(QL_APP_FEATURE_SMS)
add_subdirectory_if_exist(sms)
endif()

if(QL_APP_FEATURE_VOICE_CALL)
add_subdirectory_if_exist(voice_call)
endif()

if(QL_APP_FEATURE_VOLTE)
add_subdirectory_if_exist(volte)
endif()

if(QL_APP_FEATURE_SPI)
add_subdirectory_if_exist(spi)
endif()

if(QL_APP_FEATURE_SPI_NOR_FLASH)
add_subdirectory_if_exist(spi_nor_flash)
endif()

if(QL_APP_FEATURE_SPI6_EXT_NOR)
add_subdirectory_if_exist(spi6_ext_nor_flash)
endif()

if(QL_APP_FEATURE_SPI_NAND_FLASH)
add_subdirectory_if_exist(spi_nand_flash)
endif()

if(QL_APP_FEATURE_HTTP_FOTA)
add_subdirectory_if_exist(http_fota)
endif()

if(QL_APP_FEATURE_FTP_FOTA)
add_subdirectory_if_exist(ftp_fota)
endif()

if(QL_APP_FEATURE_DECODER)
add_subdirectory_if_exist(decoder)
endif()

if(QL_APP_FEATURE_RTC)
add_subdirectory_if_exist(rtc)
endif()

if(QL_APP_FEATURE_VIRT_AT)
add_subdirectory_if_exist(virt_at)
endif()

if(QL_APP_FEATURE_ALI_LINKSDK)
add_subdirectory_if_exist(LinkSDK)
endif()

if(QL_APP_FEATURE_QCLOUD_IOT) 
add_subdirectory_if_exist(qcloud-iot) 
endif()

if(QL_APP_FEATURE_I2C)
add_subdirectory_if_exist(i2c)
endif()

if(QL_APP_FEATURE_SDMMC)
add_subdirectory_if_exist(sdmmc)
endif()

if(QL_APP_FEATURE_USBNET)
add_subdirectory_if_exist(usbnet)
endif()

if(QL_APP_FEATURE_FS_NAND_FLASH)
add_subdirectory_if_exist(fs_nand_flash)
endif()

if(QL_APP_FEATURE_FILE_ZIP)
add_subdirectory_if_exist(zip)
endif()

if(QL_APP_FEATURE_SFTP)
add_subdirectory_if_exist(sftp)
endif()

if(QL_APP_FEATURE_MXML)
add_subdirectory_if_exist(mxml)
endif()

if(QL_APP_FEATURE_CLOUDOTA)
add_subdirectory_if_exist(cloud_ota)
endif()

if(QL_APP_FEATURE_HILINK)
add_subdirectory_if_exist(HiLinkSDK)
endif()

if(QL_APP_FEATURE_EMBED_NOR_FLASH)
add_subdirectory_if_exist(embed_nor_flash)
endif()

#if(QL_APP_FEATURE_GPRS_DATA_TRANSFER)
#add_subdirectory_if_exist(gprs_data_transfer)
#endif()