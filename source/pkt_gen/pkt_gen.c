#include "pkt_gen.h"
#include "mctp_ctrl_gen.h"
#include "pldm_ctrl_gen.h"
#include "pldm_fwup_gen.h"
#include "pldm_monitor_gen.h"
#include "pldm_redfish_gen.h"

void gen_cmd_unsupport(u8 *buf)
{

}

void pldm_gen_req_hdr_update(u8 *buf, int cmd)
{
    pldm_request_t *req_hdr = (pldm_request_t *)(buf - sizeof(pldm_request_t));
    req_hdr->cmd_code = cmd;
}

void mctp_gen(int cmd, u8 *buf)
{
    mctp_ctrl_gen(cmd, buf);
}

void pldm_gen_init(void)
{
    pldm_fwup_gen_init();
}

u8 pldm_gen(int type, int cmd, u8 *buf)
{
    u8 ret = 0;
    pldm_request_t *req_hdr = (pldm_request_t *)buf;
    req_hdr->pldm_type = type;
    // req_hdr->cmd_code = cmd;
    buf += sizeof(pldm_request_t);

    switch (type) {
        case MCTP_PLDM_CONTROL:
            // pldm_ctrl_gen(cmd, buf);
            break;

        case MCTP_PLDM_MONITOR:
            // pldm_monitor_gen(cmd, buf);
            break;

        case MCTP_PLDM_UPDATE:
            // pldm_fwup_gen(cmd, buf);
            ret = pldm_fwup_state_transform_switch(cmd, buf);
            break;

        case MCTP_PLDM_REDFISH:

            break;

        default:
            LOG("ERR PLDM PROTOCOL! : %d", type);
            break;
    }
    buf -= sizeof(pldm_request_t);
    return ret;
}

void pldm_gen_recv(u8 *msg, u8 type, u8 cmd_code)
{
    switch (type) {
        case MCTP_PLDM_CONTROL:
            break;

        case MCTP_PLDM_MONITOR:
            break;

        case MCTP_PLDM_UPDATE:
            pldm_fwup_gen_recv(cmd_code, msg);
            break;

        case MCTP_PLDM_REDFISH:
            break;

        default:
            LOG("ERR PLDM PROTOCOL!");
            break;
    }
}