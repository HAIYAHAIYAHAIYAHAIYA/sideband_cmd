#ifndef __MCTP_H__
#define __MCTP_H__

#include "protocol_device.h"

#define REAL_MCTP_HDR           (4)

/* mctp header */
#pragma pack(1)
typedef struct {
    u8 ver      : 4;
    u8 rsvd     : 4;
    u8 dest_eid;
    u8 src_eid;
    u8 msg_tag  : 3;
    u8 to       : 1;
    u8 pkt_seq  : 2;
    u8 eom      : 1;
    u8 som      : 1;

    u8 msg_type : 7;    /* MCTP消息类型，0表示MCTP Control Message，2表示NC-SI over MCTP Message ，其他数值可查阅DSP0239_1.9.0_MCTP_IDS&Codes_Spec.pdf*/
    u8 ic       : 1;    /* 消息完整性检查位，固定0 */
} mctp_hdr_t;
#pragma pack()

typedef enum {
    MCTP_MSG_TYPE_CONTROL = 0,  /* 用于在MCTP网络中支持初始化和配置MCTP通信的消息 */
    MCTP_MSG_TYPE_PLDM,         /* 用于在MCTP上 传输 平台级数据模型(PLDM)的消息 */
    MCTP_MSG_TYPE_NCSI,         /* 用于在MCTP上传输NC-SI控制通信的消息 */
    MCTP_MSG_TYPE_ETHERNET,     /* 用于在MCTP上传输以太网通信的消息。此消息类型也可以由其他规范单独使用。 */
    MCTP_MSG_TYPE_NVMEMI,       /* 用于在MCTP上传输NVME管理信息的消息 */
    MCTP_MSG_TYPE_SPDM,         /* 用于在MCTP上传输安全协议和数据模型规范(SPDM)的消息 */
    MCTP_MSG_TYPE_SECURED,      /* 用于在MCTP绑定规范流量上使用SPDM传输安全消息的消息 */

    MCTP_MSG_TYPE_VDPCI = 0x7E, /* 用于支持vdm的消息类型，其中供应商使用基于pci的供应商ID标识。此规范中提供了此消息类型的的初始Message Header字节的规范。
                                此消息的格式规格在DSP0236中给出。否则消息体内容有给定供应商ID标识的供应商、公司或组织指定 */
    MCTP_MSG_TYPE_VDIANA,       /* 用于支持vdm的消息类型，其中供应商使用基于iana的供应商ID标识。
                                这种格式使用由Internet assigned Numbers Authority (IANA)分配和维护的私有企业数字表中的数字作为识别特定供应商、公司或组织的手段。
                                此消息的格式规格在DSP0236中给出。否则，消息体内容由给定供应商ID标识的供应商、公司或组织指定 */

    MCTP_MSG_TYPE_MCTPBASE = 0xFF,
} mctp_msg_type;     /* DSP0239_1.9.0_MCTP_IDS&Codes_Spec.pdf, 5 MCTP Message Type codes, table1 */

/* Standard MCTP Completion Code */
typedef enum {
    MCTP_COMMAND_SUCCESS = 0x00,            /* The command was accepted and completed normally. */
    MCTP_COMMAND_ERROR,                     /* This is a generic failure message to indicate an error processing the corresponding request message. It should not be used when a more specific error code applies. */
    MCTP_COMMAND_INVALIDDATA,               /* The request message payload contained invalid data or an illegal parameter value. */
    MCTP_COMMAND_INVALIDLEN,                /* The request message length was invalid. (The request message body was larger or smaller than expected for the particular command.) */
    MCTP_COMMAND_NOTREADY,                  /* The Receiver is in a transient state where it is not ready to process the corresponding command. */
    MCTP_COMMAND_UNSUPPORT,                 /* The command field in the request message is unspecified or not supported for this Type. This completion code shall be returned for any unsupported command values received. */

    MCTP_COMMAND_INVALID_PLDMTYPE = 0x20,   /* The PLDM Type field value in the PLDM request message is invalid or unsupported. */

    MCTP_COMMAND_MSGTYPEUNSP = 0x80,
    MCTP_PLDM_INVALID_DATA_TRANSFER_HANDLE = 0x80,
    MCTP_PLDM_INVALID_TRANSFER_OPERATION_FLAG,
    MCTP_PLDM_INVALID_PLDM_TYPE_IN_REQUEST_DATA = 0x83,
    MCTP_PLDM_INVALID_PLDM_VERSION_IN_REQUEST_DATA = 0x84,
} mctp_cpl_code_t;


void *mctp_memcpy_fast(void *dest, const void *src, size_t count);
int mctp_pkt_process(protocol_msg_t *skb, int pkt_len);
int mctp_as_requester_send_to(protocol_msg_t *pkt, int pkt_len);
int mctp_as_responser_send_to(protocol_msg_t *pkt, int pkt_len);
void mctp_fill_common_field(u8 *buf, u8 src_eid, u8 msg_type);
#endif /* __MCTP_H__ */