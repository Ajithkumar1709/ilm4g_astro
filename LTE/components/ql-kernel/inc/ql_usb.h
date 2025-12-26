
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
    
/*=================================================================

						EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN			  WHO		  WHAT, WHERE, WHY
------------	 -------	 -------------------------------------------------------------------------------

=================================================================*/


#ifndef QL_USB_H
#define QL_USB_H


#include "ql_api_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define USB_MSC_DEVICE_MAX				2 //USB Mass Storage功能最大支持映射2个存储器
#define USB_MSC_DEVICE_TYPE				4
#define USB_DETECT_TIME_MIN             0
#define USB_DETECT_TIME_MAX             100000000
#define USB_CHARGING_ONFF_TIME_MIN      0
#define USB_CHARGING_ONFF_TIME_MAX      100000

//usb interface id
#define QUEC_USBAT_INTF_NUM				2	//usb at口
#define QUEC_USBDAIG_INTF_NUM			3	//usb diag口
#define QUEC_USBMOS_INTF_NUM			4	//debug host口
#define QUEC_USBCPLOG_INTF_NUM			5	//cp log口
#define QUEC_USBAPLOG_INTF_NUM			6	//ap log口
#define QUEC_USBMODEM_INTF_NUM			7	//usb modem口
#define	QUEC_USBNMEA_INTF_NUM			8	//usb nmea口
#define	QUEC_USBATEXT_INTF_NUM			9	//已废弃不使用
#define	QUEC_UAC_CTRL_INTF_NUM			10	//UAC 控制接口
#define	QUEC_UAC_RX_INTF_NUM			11	//UAC 接收数据接口
#define	QUEC_UAC_TX_INTF_NUM			12	//UAC 发送数据接口
#define	QUEC_USBPRINTER_INTF_NUM		13	//usb打印机
#define	QUEC_USBPRINTER_INTF_MTP		14	//usb mtp
#define	QUEC_USBPRINTER_INTF_MSG		15	//usb mass storage


#define	QUEC_ACM_AT0_NOTIFY_INTF_NUM	20	//ACM at控制接口
#define	QUEC_ACM_AT0_DATA_INTF_NUM		21	//ACM at数据接口

#define USB_FUNC_ITEM_MAX 10
#define QUEC_MTP_NAME_MAX 15
/*===========================================================================
 * Enum
 ===========================================================================*/
typedef enum
{
	QL_USB_SUCCESS						= QL_SUCCESS,							 /*  operating is successful  */
	QL_USB_INVALID_PARAM				= (QL_COMPONENT_BSP_USB << 16) | 1000,   /*  invalid input param  */
	QL_USB_SYS_ERROR					= (QL_COMPONENT_BSP_USB << 16) | 1001,	 /*  system error  */
	QL_USB_DETECT_SAVE_NV_ERR           = (QL_COMPONENT_BSP_USB << 16) | 1002,	 /*  save detect time to NV failed */
	QL_USB_NO_SPACE						= (QL_COMPONENT_BSP_USB << 16) | 1003,   /*  no space to store data  */
	QL_USB_NOT_SUPPORT					= (QL_COMPONENT_BSP_USB << 16) | 1004,   /*  current operation not support  */
}ql_errcode_usb_e;

typedef enum
{
	QL_USB_HOTPLUG_OUT = 0,    //USB in plug out state
	QL_USB_HOTPLUG_IN  = 1,   //USB was inserted but not enum
	QL_USB_HOTPLUG_IN_ENUMED =2  //USB insert and enum done
}QL_USB_HOTPLUG_E;

typedef enum
{
	QL_USB_DET_MIN = 0,
	QL_USB_DET_CHARGER = 0,	   //usb will be enabled and disabled by VBUS high level and low level 
    QL_USB_DET_AON = 1,		   //usb will be enabled always, and won't be disabled 
    QL_USB_DET_NO_VBUS = 2,    //usb will be enabled and disabled by GPIO, the GPIO function not support now, so it is same as QL_USB_DET_AON now

	QL_USB_DET_MAX,
}QL_USB_DET_E;


/*
	1. QL_USB_ENUM_MASS_STORAGE用于将模块的SD卡/内置flash/外置6线flash映射到电脑,用于传输文件（可以用ql_usb_msc_config_set函数配置映射哪些存储器,默认映射内置flash和sd卡,最多支持映射2个存储器）
	   注意:如果出现电脑没有识别到SD卡/外置flash的情况,可以将sd卡的挂载函数提前;模块下载完代码后第一次开机时,进到app会相对慢,而USB枚举的位置比较靠前,因此烧录后第一次开机时可能会出现这种情况

	2. USB_AT,USB_MODEM,USB_NMEA口可用来作为CDC数据收发,这三个端口各占用300k左右RAM空间,用户若RAM空间紧张可裁减任意端口
*/
typedef enum
{
	QL_USB_ENUM_NONE 	= -1,		//NO USB port will be enumerated

	QL_USB_ENUM_USBNET_COM, 		//usbnet(rndis/ecm), USB_AT(ttyUSB0), USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM(ttyUSB5), USB_NMEA(ttyUSB6)				
	QL_USB_ENUM_USBNET_COM_UAC, 	//usbnet(rndis/ecm), USB_AT(ttyUSB0), USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM(ttyUSB5), UAC
	QL_USB_ENUM_USBNET_COM_PRINTER, //usbnet(rndis/ecm), USB_AT(ttyUSB0), USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM(ttyUSB5), USB_PRINTER
	QL_USB_ENUM_ACM_ONLY,			//ttyACM0			
	QL_USB_ENUM_MASS_STORAGE,		////usbnet(rndis/ecm), USB_AT(ttyUSB0), USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM(ttyUSB5), USB_MSC(用于将模块的SD等存储映射到电脑,用于传输文件)

	QL_USB_ENUM_MAX, 				//invalid
}QL_USB_ENMU_MODE_E;

typedef enum
{
	QL_USB_NV_TYPE_ENUM_MODE = 0,
	QL_USB_NV_TYPE_MSC = 1,

	QL_USB_NV_TYPE_MAX,
}QL_USB_NV_TYPE_E;

/*
	1. 选择映射模块的哪些存储器到PC端, 最大支持映射2个存储器; 例如需要映射sd卡和外部
	   6线flash,可以用 (QL_USB_MSC_SDCARD | QL_USB_MSC_EFS)
	2. 注意: 映射模块外部flash时,外部flash使用的文件系统需要是模块自带的,
	   内核需要用ql_fopen这些API去控制与PC之间的文件传输
*/
typedef enum
{
	QL_USB_MSC_DEFAULT = 0,		//默认设备,默认映射SD卡和内置flash
	QL_USB_MSC_SDCARD = 0x01,	//模块的SD卡
	QL_USB_MSC_UFS = 0x2,		//模块的内置flash
	QL_USB_MSC_EFS = 0x4,		//模块的外置6线flash
	QL_USB_MSC_EXNSFFS = 0x8	//模块的外置4线flash
}QL_USB_MSC_E;

typedef enum
{
/*
	MTP: 类似于手机接入PC, PC上弹出的磁盘名称为手机厂商(名称模块端可配置);该协议下支持挂
		 载2个存储器,如同时映射模块的SD卡和外置flash,且一个文件夹下只能显示200个文件,读写
		 速度慢于MSG协议(大文件简单测试读写速率约为2.7M/s 2.1M/s,以实际为准)
*/
	QL_USB_PROTOCOL_MTP, //Media Transfer Protocol

/*
	MSG: 类似U盘或读卡器接入PC, PC上弹出的磁盘名固定为"U盘",该模式下只能映射SD卡,显示的文件或文件夹
	     数量没有上限,并且读写速度高于MTP(大文件简单测试读写速率约为7M/s 6M/s,以实际为准)
*/
	QL_USB_PROTOCOL_MSG //USB mass storage device class
}ql_usb_protocol_e;
/*===========================================================================
 * STRUCT
 ===========================================================================*/
typedef struct
{
	QL_USB_DET_E det_mode;
	uint 		 det_pin;    //not used now, but will used feature
	uint 		 reserve[2]; //reserved for futher using
}ql_usb_detmdoe_t;           //this structure will be writed to NV

typedef struct
{
    uint32_t usb_detect_time;
    uint32_t charging_on_time;
    uint32_t charging_off_time;
	ql_usb_detmdoe_t usb_det_mode;
} ql_usb_setting_s;

typedef struct
{
	int msc_device;  //需要映射到PC的存储器,参考枚举QL_USB_MSC_E;多个存储之间用'|'连接,最大支持映射2个存储器 (只有protocol参数选择MTP协议时可配置)
	char dev_name[QUEC_MTP_NAME_MAX]; //PC上显示的设备名称,默认显示"ANDORID",修改后要卸载重装一次USB驱动,因为windows会记住第一次的设备名
	ql_usb_protocol_e protocol;		  //选择使用MTP协议还是MSG协议
}ql_usb_msc_cfg_t;

typedef struct
{
	uint8 enum_mode;
	ql_usb_msc_cfg_t msc_cfg;
}ql_usb_nv_t;

/*===========================================================================
 * function
 ===========================================================================*/

/*****************************************************************
	!!!!!   don't  block the callback , as is run in interrupt   !!!!!!
* Function: ql_usb_hotplug_cb
*
* Description: the defination of usb hotplug callback
* 
* Parameters:
* 	state	  		Indicates whether the USB action is unplugged or inserted 
*	ctx         	not used now 
*
* Return:
* 	0
*****************************************************************/
typedef int(*ql_usb_hotplug_cb)(QL_USB_HOTPLUG_E state, void *ctx); //

/*****************************************************************
* 
* Function: ql_usb_bind_hotplug_cb
*
* Description: bind usb hotplug callback to kernel
* 
* Attention:
   1. the callback will be run in interrupt, so it is forbidden to block the callback;
   2. it is forbidden to call Audio start/stop/close， file write/read，CFW（releated to RPC）in interrupt;
   3. it is forbidden to enter critical in interrupt
   4. it is suggested for users to  perform simple operations , or send event(no timeout) to inform your task in interrupt

* Parameters:
* 	hotplug_callback	  [in]callback
*
* Return:
* 	0
*****************************************************************/
ql_errcode_usb_e ql_usb_bind_hotplug_cb(ql_usb_hotplug_cb hotplug_callback);

/*****************************************************************
* Function: ql_usb_get_hotplug_state
*
* Description: get the usb hotplug state
* 
* Parameters:
* 	hotplug_callback	  [in]callback
*
* Return:
* 	QL_USB_HOTPLUG_OUT   : USB is not insrrted
*	QL_USB_HOTPLUG_IN    : USB is inserted
*****************************************************************/
QL_USB_HOTPLUG_E ql_usb_get_hotplug_state(void);

/*****************************************************************
* Function: ql_usb_set_detmode
*
* Description: set usb detect mode
* 
* Parameters:
* 	usb_mode    [in] detected mode, take effect after reset
*
* Return:
*   0:          success
*   others:     ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_set_detmode(ql_usb_detmdoe_t *usb_mode);

/*****************************************************************
* Function: ql_usb_get_detmode
*
* Description: get the usb detect mode
* 
* Parameters:
* 	ql_usb_detmdoe_t	[out]usb detect mode
*
* Return:
*   0:          success
*   others:     ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_get_detmode(ql_usb_detmdoe_t *mode);

/*****************************************************************
* Function: ql_set_usb_detect_max_time
*
* Description: Set the maximum time to detect the connection of 
*              USB DP/DM data line after USB insertion
*              设置USB插入后最大的检测时间，如果过了这个时间，还没有
*              检测到DP/DM线上有数据返回，则关闭检测，省电
* 
* Parameters:
*   ms      [in] the maximum detect time to set
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*
*****************************************************************/
ql_errcode_usb_e ql_set_usb_detect_max_time(uint32_t ms);

/*****************************************************************
* Function: ql_get_usb_detect_max_time
*
* Description: get the maximum USB detection time set
*              
* Parameters:
*   ms      [out]  the maximum detect time
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*
*****************************************************************/
ql_errcode_usb_e ql_get_usb_detect_max_time(uint32_t *ms);

/*****************************************************************
* Function: ql_usb_get_enum_mode
*
* Description: to get the usb enumeration mode             
*
* Return:
*	QL_USB_ENMU_MODE_E
*
*****************************************************************/
QL_USB_ENMU_MODE_E ql_usb_get_enum_mode(void);

/*****************************************************************
* Function: ql_usb_set_enum_mode
*
* Description: to set the enumeration mode of usb port.For UAC or
			   usb printer mode, the USB nmea PORT will not be enumerated		   
*
* Parameters:
*	mode	   [in] see it in QL_USB_ENMU_MODE_E for detail
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_set_enum_mode(QL_USB_ENMU_MODE_E mode);

#ifdef CONFIG_QUEC_PROJECT_FEATURE_USB_MASS_STORAGE
/*****************************************************************
* Function: ql_usb_msc_config_set
*
* Description: to set the configration of usb mass storage, need to set 
			   the enum mode to usb mass storage by ql_usb_get_enum_mode firstly;
			   Restart to take effect
* Parameters:
*	msc_device [in]see it in QL_USB_MSC_E for detail, to set the mapping storage device
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_msc_config_set(ql_usb_msc_cfg_t *msc_cfg);

/*****************************************************************
* Function: ql_usb_msc_config_get
*
* Description: to get the configration of usb mass storage	   
*
* Parameters:
*	msc_cfg	   [out]the space for getting the configration
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_msc_config_get(ql_usb_msc_cfg_t *msc_cfg);
#endif

#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QL_USB_H */


