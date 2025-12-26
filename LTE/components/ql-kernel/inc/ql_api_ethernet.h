/*============================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
 =============================================================================*/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


WHEN        WHO            WHAT, WHERE, WHY
----------  ------------   ----------------------------------------------------

=============================================================================*/

#ifndef QL_API_ETHERNET_H
#define QL_API_ETHERNET_H


#include "ql_api_common.h"
#include "ql_api_spi.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *  Marco Definition
 *========================================================================*/



/*========================================================================
 *  Enumeration Definition
 *========================================================================*/
typedef enum
{
    QL_ETHERNET_SUCCESS                          = 0,
    QL_ETHERNET_EXECUTE_ERR                      = 1| (QL_COMPONENT_ETHERNET << 16),
    QL_ETHERNET_MEM_ADDR_NULL_ERR,
    QL_ETHERNET_INVALID_PARAM_ERR,
    QL_ETHERNET_NO_MEMORY_ERR,
    QL_ETHERNET_REPEAT_REGISTER_ERR                  = 5| (QL_COMPONENT_ETHERNET << 16),
    QL_ETHERNET_NO_NETCARD_ERR,
    QL_ETHERNET_NETCARD_CREATED_ERR,
    QL_ETHERNET_IP_CONFLICT_ERR,
    QL_ETHERNET_INVALID_DNS_ERR,
    QL_ETHERNET_INVALID_DHCP_ERR,
    QL_ETHERNET_INVALID_CID_ERR,
    QL_ETHERNET_INVALID_PREFER_ROUTE_ERR,
    QL_ETHERNET_INVALID_PARAM_LEN_ERR,
    QL_ETHERNET_ASSIGN_PORT_ERR,
    QL_ETHERNET_NAT_CHECKED_ERR,
    QL_ETHERNET_NAT_LAN_CREATED_ERR,
    QL_ETHERNET_NAT_WAN_EXIST,
} ql_ethernet_errcode_e;


typedef enum
{
    QL_ETHERNET_MODE_NONE    = 0,
    QL_ETHERNET_MODE_ROUTER  = 1,
    QL_ETHERNET_MODE_MAX,
}ql_ethernet_mode_e;

typedef enum
{
    QL_ETHERNET_PHY_HW_SPI_MODE     = 0,    //HW SPI mode.The ctx passed to ql_ethernet_phy_init will be phased as hw_spi in ql_ethernet_phy_s.
    QL_ETHERNET_PHY_MODE_MAX
}ql_ethernet_phy_mode;

typedef enum
{
    QL_ETHERNET_NET_STATUS_DISCONNECTED = 0,
    QL_ETHERNET_NET_STATUS_CONNECTED,
}ql_ethernet_net_status_e;

typedef enum
{
    QL_ETHERNET_ROUTE_PREFER_NONE,
    QL_ETHERNET_ROUTE_PREFER_ETH,
    QL_ETHERNET_ROUTE_PREFER_PDN,
    QL_ETHERNET_ROUTE_PREFER_MAX,
}ql_ethernet_route_prefer_e;

typedef enum
{
    QL_ETHERNET_OPT_DNS,
    QL_ETHERNET_OPT_DHCP,           /* val_len: sizeof(uint8_t) */
    QL_ETHERNET_OPT_CID,            /* val_len: sizeof(ql_ethernet_cid_s),ethernet cid */
    QL_ETHERNET_OPT_ROUTE_PREFER,   /* val_len: sizeof(ql_ethernet_route_prefer_e) */ 
    QL_ETHERNET_OPT_MAX,
} ql_ethernet_opt_e;
/*========================================================================
  *  Callback Definition
  *========================================================================*/
typedef void (*ql_ethernet_linkoutput_cb)(uint8_t *data,uint32_t len);

typedef void (*ql_ethernet_net_status_cb)(void* ctx);  /* ctx see ql_ethernet_net_s. */

/*========================================================================
*  Type Definition
*========================================================================*/
typedef struct
{
    uint8_t sim_id;
    int cid;
    uint8_t enable;
}ql_ethernet_cid_s;

typedef struct
{
    uint8_t *ip4;
    uint8_t *gw;
    uint8_t *netmask;
    uint8_t *mac;
    uint8_t *dns1;
    uint8_t *dns2;
    ql_ethernet_mode_e mode;
    /* Physical data output for SPI/UART and so on. */
    ql_ethernet_linkoutput_cb cb;
    /* Used for getting IP configuration status. */
    ql_ethernet_net_status_cb net_cb;

    /* The following parameters can be ignored. */
    struct
    {
        uint8_t cover;  /* 1: Set the following options in ql_ethernet_register. 0: Do not set the following option. */
        uint8_t dhcp;
        /* 
            Specify sim_id and cid for ethernet.Make the connection of internal datacall and phy.
            The following must be set except for QL_ETHERNET_MODE_NONE(See ql_ethernet_mode_e).
        */
        ql_ethernet_cid_s ecid;
        ql_ethernet_route_prefer_e prefer;
    }option;
}ql_ethernet_ctx_s;


typedef struct
{
    ql_ethernet_phy_mode mode;
    union
    {
        struct
        {
            uint8_t mosi_pin_num;
            uint8_t mosi_func_sel;
            uint8_t miso_pin_num;
            uint8_t miso_func_sel;
            uint8_t clk_pin_num;
            uint8_t clk_func_sel;
            uint8_t cs_pin_num;
            uint8_t cs_func_sel;
            ql_spi_config_s config;
        }hw_spi;
    };
}ql_ethernet_phy_s;

typedef struct
{
    uint8_t status;
}ql_ethernet_net_status_s;

typedef struct
{
    ip_addr_t dns_server[LWIP_NETIF_IP_TYPE_MAX][LWIP_NETIF_DNS_MAX];
}ql_ethernet_dns_s;
/*========================================================================
 *	function Definition
 *========================================================================*/

/*****************************************************************
* Function: ql_ethernet_phy_init
*
* Description: Init the physical device by mode.
* 
* Parameters:
* ctx                       [in]    see ql_ethernet_phy_mode and ql_ethernet_phy_s.
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_phy_init(ql_ethernet_phy_s *ctx);

/*****************************************************************
* Function: ql_ethernet_phy_write
*
* Description: Write to data bus,e.g. SPI or UART.Depend on mode after ql_ethernet_phy_init is called.
* 
* Parameters:
* data                          [in]    output data which write to data bus
* len                           [in]    output data len which write to data bus
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_phy_write(uint8_t *data,uint32_t len);

/*****************************************************************
* Function: ql_ethernet_phy_read
*
* Description: Read from data bus,e.g. SPI or UART.Depend on mode after ql_ethernet_phy_init is called.
* 
* Parameters:
* data                          [out]    input data which read from data bus
* len                           [out]    input data len which read from data bus
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_phy_read(uint8_t *data,uint32_t len);

/*****************************************************************
* Function: ql_ethernet_phy_deinit
*
* Description: Deinit the physical device.
* 
* Parameters:
* ctx                       [in]    No defined yet.
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_phy_deinit(void* ctx);

/*****************************************************************
* Function: ql_ethernet_phy_append
*
* Description: An append data bus function defined by user.Use for adapter.See ethernet_demo.h
*
* 
* Parameters:
* cb                             [in]    callback function defined by user
* ctx                            [in]    parameter pass to callback function
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_phy_append(void *cb,void *ctx);

/*****************************************************************
* Function: ql_ethernet_register
*
* Description: Register a netcard for ethernet PHY.
* 
* Parameters:
* ql_ethernet_ctx_s        		[in]    see ql_ethernet_ctx_s.
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_register(ql_ethernet_ctx_s *ctx);

/*****************************************************************
* Function: ql_ethernet_data_input
*
* Description: Data input from ethernet PHY. 
* 
* Parameters:
* data                          [in]    ethernet PHY input data    
* len                           [in]    ethernet PHY input data len
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_data_input(uint8_t *data,uint32_t len);

/*****************************************************************
* Function: ql_ethernet_deregister
*
* Description: Unregister exist netcard.Must be called when no data transfer.
* 
* Parameters:
* 
*
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_deregister(void);

/*****************************************************************
* Function: ql_ethernet_set_down
*
* Description: Set netcard down.Must be called when no data transfer.
* 
* Parameters:
* 
*
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_set_down(void);

/*****************************************************************
* Function: ql_ethernet_set_up
*
* Description: Set netcard up.
* 
* Parameters:
* 
*
* Return:
*	0			success
*	other 	    error code
*
*****************************************************************/
ql_ethernet_errcode_e ql_ethernet_set_up(void);

/*****************************************************************
 * Function: ql_ethernet_opt_set
 *
 * Description: Set ethernet option.See ql_ethernet_opt_e.
 *
 * Parameters:
 * ql_ethernet_opt_e    [in]    Option type.See ql_ethernet_opt_e.
 * val                  [in]    The address of input parameter.
 * val_len              [in]    The length of input parameter.See ql_ethernet_opt_e.
 *
 * Return:
 *  0           success
 *  other       error code
 *
 *****************************************************************/
ql_ethernet_errcode_e ql_ethernet_opt_set(ql_ethernet_opt_e opt, void *val, int val_len);

/*****************************************************************
 * Function: ql_ethernet_opt_get
 *
 * Description: Get ethernet option.See ql_ethernet_opt_e.
 *
 * Parameters:
 * ql_ethernet_opt_e    [in]        Option type.See ql_ethernet_opt_e.
 * val                  [Out]       The address of output parameter.
 * val_len              [in]        The length of output parameter.
 *                      [out]       Some options will return a valid "val_len".See ql_ethernet_opt_e.
 *
 * Return:
 *  0           success
 *  other       error code
 *
 *****************************************************************/
ql_ethernet_errcode_e ql_ethernet_opt_get(ql_ethernet_opt_e opt, void *val, int *val_len);

#ifdef __cplusplus
}/*"C" */
#endif

#endif   /*QL_API_ETHERNET_H*/

