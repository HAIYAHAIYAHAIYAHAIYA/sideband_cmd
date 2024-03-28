#ifndef __PROTOCOL_DEVICE_H__
#define __PROTOCOL_DEVICE_H__

#include "main.h"

typedef struct protocol_msg protocol_msg_t;
typedef int (*sendto_func)(protocol_msg_t *pkt, int pkt_len);

struct protocol_msg {
    u8          *req_buf;
    u8          *rsp_buf;
    sendto_func send_to_2; // ncsi -> mctp / rbt send_to
    sendto_func send_to_1; // mctp -> vdm / smbus send_to
    u16         mctp_max_payload;
    u16         mctp_hw_hdr_len;
    u8          mctp_hw_id; //0 - smbus, 1 - vdm0, 2 - vdm1
};


typedef struct protocol_device protocol_device;
struct protocol_device {
    sendto_func     send_to;
};

void protocol_device_register(protocol_device *dev, sendto_func send_to);

#endif /* __PROTOCOL_DEVICE_H__ */