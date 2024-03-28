#ifndef __PLDM_H__
#define __PLDM_H__

#include "main.h"
#include "protocol_device.h"

#pragma pack(1)
typedef struct {
    u8 instance_id  : 5;
    u8 rsvd         : 1;
    u8 d            : 1;
    u8 rq           : 1;

    u8 pldm_type    : 6;
    u8 hdr_ver      : 2;

    u8 cmd_code;
} pldm_request_t;

typedef struct {
    u8 instance_id  : 5;
    u8 rsvd         : 1;
    u8 d            : 1;
    u8 rq           : 1;

    u8 pldm_type    : 6;
    u8 hdr_ver      : 2;

    u8 cmd_code;
    u8 cpl_code;
} pldm_response_t;
#pragma pack()

typedef enum {
    MCTP_PLDM_CONTROL = 0x00,   /* E810_Datasheet_Rev2.3.pdf, 12.8.1.2 */
    MCTP_PLDM_MONITOR = 0x02,   /* E810_Datasheet_Rev2.3.pdf, 12.8.2.1 */
    MCTP_PLDM_UPDATE = 0x05,    /* E810_Datasheet_Rev2.3.pdf, 12.8.4 */
    MCTP_PLDM_REDFISH = 0x06,
} mctp_pldm_type_t;

typedef void (*pldm_cmd_func)(protocol_msg_t *dev, int *dev_len);
int pldm_pkt_process(protocol_msg_t *pkt);
void pldm_msg_send(u8 *msg, u16 msg_len, u8 pldm_type, u8 cmd_code, int hw_id, u8 dest_eid);

#endif /* __PLDM_H__ */