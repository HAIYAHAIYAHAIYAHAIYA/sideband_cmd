#ifndef __MCTP_CTL_H__
#define __MCTP_CTL_H__

#include "protocol_device.h"

#define MCTP_CTL_CMD        (0x14 + 1)

#define MCTP_EID_NULL       0x0
#define MCTP_EID_BROADCAST  0xFF

/* 数据包结构： mctp_smbus_hdr_t + mctp_hdr_t + mctp_ctrl_request_t/mctp_ctrl_response_t + Message_Data(≥0byte) + pec(crc校验) */
/* 对于MCTP控制消息，MCTP控制消息具有意外或不正确的标志位值将被消息的接收方静默丢弃 */
/* d: 当消息类型为command/request、response、Broadcast_request时，此field固定为0；当消息类型为datagram、Broadcast_Datagram时，此field固定为1
   rq: 当消息类型为command/request、Broadcast_request、datagram、Broadcast_Datagram时，此field固定为1；当消息类型为response时，此field固定为0
   instance_id: req_msg可以随意设置，没有对该field的判断操作*/
#pragma pack(1)
typedef struct {
    u8 instance_id  : 5;    /* 实例id，用于标识MCTP控制请求或数据报的新实例，以区分发送到给定消息终端的新请求或数据报 与 发送到相同消息终端的重试消息
                            此字段还用于MCTP_rsp_msg的特定实例 与 MCTP_req_msg的对应实例进行匹配*/
    u8 rsvd         : 1;
    u8 d            : 1;    /* 全称Datagram bit，用于表明instance_id字段是用于跟踪和匹配请求及响应，还是仅用于标识重传的消息，详见10.5 */
    u8 rq           : 1;    /* 全称Request bit，用于帮助区分MCTP Control请求消息和其他消息类型，详见10.5 */
    u8 cmd_code;            /* 可以看作cmd idx，对应mctp_cmd_table */
} mctp_ctrl_request_t;

typedef struct {
    u8 instance_id  : 5;
    u8 rsvd         : 1;
    u8 d            : 1;
    u8 rq           : 1;
    u8 cmd_code;
    u8 cpl_code;    /* rsp_msg独有，全称Completion Code，用于表明相应是否正常完成，如果命令没有正常完成，该值可以提供有关错误条件的附加信息 */
} mctp_ctrl_response_t;

// set Endpoint ID
typedef struct {
    u8 operation : 2;  // 00 set eid, 01 force eid, 10 reset eid, 11 set disc flag
    u8 resv : 6;
    u8 eid;  // request eid;  0xFF,0x00 = illegal, return ERROR_INVALID_DATA
} set_eid_req_dat;

typedef struct {
    u8 eid_alloc_status : 2;
    u8 resv1 : 2;
    u8 eid_assign_status : 2;
    u8 resv2 : 2;
    u8 eid_set;
    u8 eid_pool_sz;
} set_eid_rsp_dat;

// get Endpoint ID
// not have request data, only command code
typedef struct {
    u8 eid;  // endpoint id
    u8 eid_type : 2;  // endpoint id type
    u8 resv1 : 2;
    u8 e_type : 2;  // endpoint type
    u8 resv2 : 2;
    u8 medium_spec_info; // hold additional information
} get_eid_rsp_dat;

// get Endpoint UUID
// UUID bytes 1:16, respectively
typedef struct {
    u32 time_low;
    u16 time_mid;
    u16 time_high;
    u8 clk_seq_hi;
    u8 clk_seq_low;
    u8 node[6];
} get_uuid_rsp_dat;

// get Version support
typedef struct {
    u8 msg_type_num;
} get_ver_sup_req_dat;

typedef struct {
    u8 ver_entry_cnt;
    u8 alpha_byte;
    u8 upd_ver_num;
    u8 minor_ver_num;
    u8 major_ver_num;
} get_ver_sup_rsp_dat;

// get Message Type support
typedef struct {
    u8 msg_type_cnt;
    u8 msg_type_code[0];
} get_msg_type_sup_rsp_dat;

// get Vendor-Defined Message Support
typedef struct {
    u8 vendor_id_set_selector;
} get_vendor_defined_msg_sup_req_dat;

typedef struct {
    u8 vendor_id_set_selector;
    u16 vendor_id;
    u8 pci_id_indicator;
    u16 ver;
} get_vendor_defined_msg_sup_rsp_dat;

typedef union {
    struct {
        u8 bus_num;
        u8 func_num : 3;
        u8 dev_num  : 5;
    };
    u16 val;
} phy_addr_t;

typedef struct {
    u8 cpl_code;
    u8 bridge_eid;
    phy_addr_t phy_addr;        /* BDF ID */
} resolve_eid_rsp_dat;

typedef struct {
    u32 recv_buf_size;                  /* in bytes */
    u32 max_recv_data_rate_limit;       /* packets/sec */
    u32 max_sup_rate_limit;             /* packets/sec */
    u32 min_sup_rate_limit;             /* packets/sec */
    u8 max_sup_brust_size[3];
    u8 max_brust_size[3];
    u32 eid_max_trans_data_rate_limit;
    u8 rate_limit_sup_on_eid   : 1;     /* Rate limiting Support on EID */
    u8 trans_rate_limit_op_cap : 1;     /* Transmit Rate limiting operation capability */
    u8 rsvd                    : 6;
} query_rate_limit_rsp_dat;

typedef struct {
    u8 eid_trans_max_burst_size[3];
    u32 eid_max_trans_data_rate_limit;
} request_tx_rate_limit_req_dat;

typedef struct {
    u32 eid_trans_burst_size;
    u32 eid_trans_data_rate_limit;
} request_tx_rate_limit_rsp_dat;

typedef struct {
    u8 eid_trans_max_burst_size[3];
    u32 eid_max_trans_data_rate_limit;
} update_rate_limit_req_dat;

typedef struct {
    u8 type;
    u8 eid;
} interface_info_dat;

typedef struct {
    u8 sup_interface_cnt;
    interface_info_dat interface_info[0];
} query_supported_interface_rsp_dat;

// MCTP BASE property
typedef struct {
    u8 discovered;
    u8 dev_eid;
    get_uuid_rsp_dat uuid;
} mctp_base_info;
#pragma pack()

enum {
    SET_EID = 0,
    FORCE_EID,
    RESET_EID,
    SET_DISC_FLAG,
};

enum {
    USE_BY_BROADCAST = 0,
    USE_BY_ID,
    USE_BY_RC
};

enum {
    SMBUS_SPD_100K = 0x01,
    SMBUS_SPD_400K = 0x04,
    SMBUS_SPD_1M = 0x05
};

enum {
    PCIE_2_1_DEV = 0x0a,
    PCIE_3_0_DEV = 0x0b,
    PCIE_4_0_DEV = 0x0c
};

typedef int (*mctp_cmd_func)(protocol_msg_t *dev, int *dev_len);

int mctp_ctl_process(protocol_msg_t *pkt);
void mctp_ctrl_init(void);
void mctp_discovery_notify_req(int hw_id);
void mctp_resolve_eid_req(int hw_id, u8 target_eid);

#endif /* __MCTP_CTL_H__ */