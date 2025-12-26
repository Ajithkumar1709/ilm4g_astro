/**  @file
  quec_virt_func_common.c

  @brief
  This file is used to run virt function.

*/

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
22/06/2021        neo     Init version
=================================================================*/
#include "at_response.h"
#include "at_engine.h"
#include "quec_cust_feature.h"
#include "quec_common.h"
#include "at_command.h"
#include "at_param.h"
#include "ql_api_osi.h"
#include "quec_at_engine.h"
#include "quec_log.h"
#include "ql_lcd.h"
#include "ql_sdmmc.h"
#include "ql_audio.h"
#include "ql_gnss.h"
#include "ql_usb.h"
#include "ql_api_spi4_ext_nor_sffs.h"
#include "ql_api_spi6_ext_nor_flash.h"
#include "ql_api_spi_nand_flash.h"

#define QUEC_VIRT_FUNC_LOG(msg, ...)  custom_log("VIRT_FUNC", msg, ##__VA_ARGS__)

__attribute__((weak)) const ql_at_desc_t *ql_atcmd_search_desc(const char *name, size_t length)
{
    QUEC_VIRT_FUNC_LOG("weak ql at search");
    return NULL;
}

//LCD
#ifndef CONFIG_QUEC_PROJECT_FEATURE_LCD
__attribute__((weak)) int ql_compute_threshold(uint16_t *in_buf, int width, int height)
{
    return 0;
}

__attribute__((weak)) ql_errcode_lcd ql_spi_lcd_write_cmd(uint8_t cmd)
{
    return QL_LCD_EXECUTE_ERR;
}

__attribute__((weak)) ql_errcode_lcd ql_spi_lcd_write_data(uint8_t data)
{
    return QL_LCD_EXECUTE_ERR;
}
#endif/*CONFIG_QUEC_PROJECT_FEATURE_LCD*/


#ifndef CONFIG_QUEC_PROJECT_FEATURE_WIFISCAN
__attribute__((weak)) bool ql_wifiscan_get_open_state(void)
{
    return false;
}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_BT
__attribute__((weak)) bool ql_bt_ble_is_using()
{
    return false;
}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_SDMMC
__attribute__((weak)) void quec_exec_sdmmc_mkfs_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void quec_exec_sdmmc_mount_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void quec_exec_sdmmc_qft_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) bool ql_sdmmc_is_mount(void)
{
    return FALSE; 
}

__attribute__((weak)) bool ql_sdmmc_is_mount_ex(ql_sdmmc_partition_num_e part)
{
    return FALSE; 
}

__attribute__((weak)) ql_errcode_sdmmc_e ql_sdmmc_mkfs(uint8_t opt)
{
    return QL_SDMMC_MKFS_ERR;
}

__attribute__((weak)) ql_errcode_sdmmc_e ql_sdmmc_mount(void)
{
    return QL_SDMMC_MOUNT_ERR;
}

__attribute__((weak)) ql_errcode_sdmmc_e ql_sdmmc_umount(void)
{
    return QL_SDMMC_MOUNT_ERR;
}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
uint8_t ql_spi6_ext_nor_flash_clk = SPI6_EXT_NOR_FLASH_CLK_83MHZ;
__attribute__((weak)) bool ql_get_spi6_ext_nor_status(void)
{
    return false;
}

__attribute__((weak)) void quec_set_spi6_ext_nor_flash_capacity(void)
{
    return ;
}

__attribute__((weak)) int ql_spi6_ext_nor_flash_clk_set(uint8_t clk)
{
    return 0;
}

__attribute__((weak)) int quec_spi6_ext_nor_flash_mount(bool format_on_fail)
{
    return 0;
}

__attribute__((weak)) void quec_boot_spi6_ext_norflash_gpio_init(void)
{
    return ;
}

__attribute__((weak)) void quec_boot_get_spi6_ext_norflash_gpio_default_fuction(void)
{
    return ;
}

__attribute__((weak)) void quec_boot_spi6_ext_norflash_gpio_deinit(void)
{
    return ;
}
__attribute__((weak)) void quec_exec_efs_mkfs_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void ql_boot_set_skip_spi6_ext_norflash_init(bool onff)
{
    return ;
}
__attribute__((weak)) void quec_spi6_ext_norflash_set_mount_status(bool onff)
{
    return ;
}
__attribute__((weak)) bool quec_spi6_ext_norflash_get_mount_status(void)
{
    return false;
}
__attribute__((weak)) ql_errcode_spi6_nor_e ql_spi6_ext_nor_flash_sffs_mkfs(void)
{
    return QL_SPI6_EXT_NOR_FLASH_NOT_INIT_ERROR;
}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_SPI4_EXT_NOR_SFFS

__attribute__((weak)) bool quec_spi4_exnsffs_is_mount(void)
{
    return false;
}

__attribute__((weak)) void quec_exec_exnsffs_mkfs_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) ql_spi4_extnsffs_status_e ql_spi4_ext_nor_sffs_is_mount(void)
{
    return (ql_spi4_extnsffs_status_e)0;
}

#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE_FS_NAND_FLASH
__attribute__((weak)) uint32_t ql_nand_total_erase_cnt = 0;
__attribute__((weak)) unsigned int  *ql_nand_erase_count_table = NULL;
__attribute__((weak)) unsigned int ql_nand_block_totalnum = 0;

__attribute__((weak)) ql_spi_port_e ql_nand_spi_port = QL_SPI_PORT1;
__attribute__((weak)) ql_nand_ops_t *nand_opt = NULL;

__attribute__((weak)) void quec_exec_nand_mkfs_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void quec_exec_nand_mount_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}
__attribute__((weak)) bool ql_nand_flash_is_mount(void)
{
    return false;
}

#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_GNSS
nmeasrc_sentences nmea_handle = {0};
uint8_t quec_gnss_nwrtc_update_flag = 0;
quec_gps_handle quec_gps_config_param = {0};
__attribute__((weak)) ql_GnssState ql_gnss_state_info_get(void)
{
    return GNSS_CLOSE;
}

__attribute__((weak)) void quec_gps_config_param_read(void)
{
    return ;
}
#endif

//FILE_AT
#ifndef CONFIG_QUEC_PROJECT_FEATURE_FILE_AT
__attribute__((weak)) QuecOSStatus quec_file_task_init(void)
{
    QUEC_VIRT_FUNC_LOG("weak name function");
    return QUEC_GENERAL_ERROR;
}

__attribute__((weak)) void quec_exec_file_qfmkdir_cmd(atCommand_t *cmd)
{
	quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
	return ;
}

__attribute__((weak)) void quec_exec_file_qfrmdir_cmd(atCommand_t *cmd)
{
	quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
	return ;
}

__attribute__((weak)) void quec_exec_qfupl_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
	return ;
}

__attribute__((weak)) void  quec_exec_qfopen_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
	return ;
}

__attribute__((weak)) void  quec_exec_file_qflst_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qfdel_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_file_qfclose_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qfread_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qfdwl_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qflds_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qfwrite_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qfseek_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qfposition_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qftestwr_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void  quec_exec_file_qftucat_cmd(atCommand_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

#endif /*CONFIG_QUEC_PROJECT_FEATURE_FILE_AT*/


#ifndef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
__attribute__((weak)) void ql_audio_start_ring_handler(void)
{
	return;
}

__attribute__((weak)) void ql_audio_stop_ring_handler(void)
{
	return;
}

__attribute__((weak)) QL_AUD_RING_TYPE_E ql_aud_get_ringtone_type(void)
{
	return QL_AUD_RING_NONE;
}

__attribute__((weak)) ql_audio_errcode_e ql_aud_set_ringtone_type(QL_AUD_RING_TYPE_E type)
{
	return QL_AUDIO_OPER_NOT_SUPPORTED;
}

__attribute__((weak)) QL_AUD_SPKPA_TYPE_E ql_aud_get_pa_type(void)
{
	return QL_AUD_SPKPA_TYPE_CLASSAB;
}

__attribute__((weak)) bool ql_aud_nvm_init(void)
{
	return -1;
}

__attribute__((weak)) uint8 uinsoc_volume_to_ql_vol(uint8 volume, uint8 *res)
{
	return 0;
}
__attribute__((weak)) uint8 ql_volume_to_unisoc_vol(AUDIOHAL_SPK_LEVEL_T volume, uint8 *res)
{
	return 0;
}

__attribute__((weak)) ql_audio_errcode_e ql_set_volume(AUDIOHAL_SPK_LEVEL_T vol)
{
	return QL_AUDIO_OPER_NOT_SUPPORTED;
}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_USB
__attribute__((weak)) ql_errcode_usb_e ql_set_usb_detect_max_time(uint32_t ms)
{
    return QL_USB_SYS_ERROR;
}

__attribute__((weak)) ql_errcode_usb_e ql_usb_set_detmode(ql_usb_detmdoe_t *usb_mode)
{
    return QL_USB_SYS_ERROR;
}

__attribute__((weak)) ql_errcode_usb_e ql_usb_get_detmode(ql_usb_detmdoe_t *mode)
{
    return QL_USB_SYS_ERROR;
}

__attribute__((weak)) ql_errcode_usb_e ql_usb_hotplug_init(void)
{
    return QL_USB_SYS_ERROR;
}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_FIREWALL_AT
__attribute__((weak)) void at_firewall_qfirewallcfg_cmd_func(ql_at_cmd_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}
#endif

#ifndef CONFIG_AT_WIFISCAN_SUPPORT
__attribute__((weak)) void atCmdHandleWifiOpen(ql_at_cmd_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void atCmdHandleWifiClose(ql_at_cmd_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}

__attribute__((weak)) void atCmdHandleWifiScan(ql_at_cmd_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}
#ifndef CONFIG_AT_WIFISCAN_SUPPORT
__attribute__((weak)) void atCmdHandleWifiSensitivityTest(ql_at_cmd_t *cmd)
{
    quec_atResp(cmd->engine, ATCI_RESULT_CODE_CME_ERROR, CMD_RC_NOTSUPPORT, NULL, 1);
    return ;
}
#endif
#endif



