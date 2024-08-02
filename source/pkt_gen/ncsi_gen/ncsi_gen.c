#include "ncsi_gen.h"
#include "ncsi.h"
#include "pkt_gen.h"

static void ncsi_gen_cmd_00(u8 *buf)
{

}

static void ncsi_gen_cmd_01(u8 *buf)
{

}

static void ncsi_gen_cmd_02(u8 *buf)
{

}

static void ncsi_gen_cmd_03(u8 *buf)
{

}

static void ncsi_gen_cmd_04(u8 *buf)
{

}

static void ncsi_gen_cmd_05(u8 *buf)
{

}

static void ncsi_gen_cmd_06(u8 *buf)
{

}

static void ncsi_gen_cmd_07(u8 *buf)
{

}

static void ncsi_gen_cmd_08(u8 *buf)
{
    ncsi_cmd_ae_pkt *req = (ncsi_cmd_ae_pkt *)buf;
    req->mc_id = 0x1;
    req->mode = 0x0;

    req->cmd.common.length += sizeof(req->mc_id) + sizeof(req->mode);
}

static void ncsi_gen_cmd_09(u8 *buf)
{
    ncsi_cmd_sl_pkt *req = (ncsi_cmd_sl_pkt *)buf;
    req->mode = 0;
    req->oem_mode = 0;

    req->cmd.common.length += sizeof(req->oem_mode) + sizeof(req->mode);
}

static void ncsi_gen_cmd_0a(u8 *buf)
{

}

static void ncsi_gen_cmd_0b(u8 *buf)
{
    ncsi_cmd_svf_pkt *req = (ncsi_cmd_svf_pkt *)buf;
    req->vlan = 0;
    req->index = 0;
    req->enable = TRUE;

    req->cmd.common.length += sizeof(req->vlan) + sizeof(req->index) + sizeof(req->enable);
}

static void ncsi_gen_cmd_0c(u8 *buf)
{
    ncsi_cmd_ev_pkt *req = (ncsi_cmd_ev_pkt *)buf;
    req->mode = 0;

    req->cmd.common.length += sizeof(req->mode);
}

static void ncsi_gen_cmd_0d(u8 *buf)
{

}

static void ncsi_gen_cmd_0e(u8 *buf)
{
    ncsi_cmd_sma_pkt *req = (ncsi_cmd_sma_pkt *)buf;
    u8 mac[6] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x1};
    req->at_e = 0;
    req->index = 0;
    cm_memcpy(req->mac, mac, 6);

    req->cmd.common.length += sizeof(req->at_e) + sizeof(req->index) + sizeof(mac);
}

static void ncsi_gen_cmd_0f(u8 *buf)
{

}

static void ncsi_gen_cmd_10(u8 *buf)
{
    ncsi_cmd_ebf_pkt *req = (ncsi_cmd_ebf_pkt *)buf;
    req->mode = 0;

    req->cmd.common.length += sizeof(req->mode);
}

static void ncsi_gen_cmd_11(u8 *buf)
{

}

static void ncsi_gen_cmd_12(u8 *buf)
{
    ncsi_cmd_egmf_pkt *req = (ncsi_cmd_egmf_pkt *)buf;
    req->mode = 0;

    req->cmd.common.length += sizeof(req->mode);
}

static void ncsi_gen_cmd_13(u8 *buf)
{

}

static void ncsi_gen_cmd_14(u8 *buf)
{
    ncsi_cmd_snfc_pkt *req = (ncsi_cmd_snfc_pkt *)buf;
    req->mode = 0;

    req->cmd.common.length += sizeof(req->mode);
}

static void ncsi_gen_cmd_15(u8 *buf)
{

}

static void ncsi_gen_cmd_16(u8 *buf)
{

}

static void ncsi_gen_cmd_17(u8 *buf)
{

}

static void ncsi_gen_cmd_18(u8 *buf)
{

}

static void ncsi_gen_cmd_19(u8 *buf)
{

}

static void ncsi_gen_cmd_1a(u8 *buf)
{

}

static void ncsi_init_req(int cmd, u8 *buf)
{
    ncsi_pkt_hdr *ncsi_hdr = (ncsi_pkt_hdr *)buf;
    ncsi_hdr->mc_id = 0;
    ncsi_hdr->revision = 0x1;
    ncsi_hdr->type = cmd;
    ncsi_hdr->length = 0;
    ncsi_hdr->channel = 0;
}

void ncsi_cmd_gen(int cmd, u8 *buf)
{
    gen_cmd cmds[] = {
        ncsi_gen_cmd_00,
        ncsi_gen_cmd_01,
        ncsi_gen_cmd_02,
        ncsi_gen_cmd_03,
        ncsi_gen_cmd_04,
        ncsi_gen_cmd_05,
        ncsi_gen_cmd_06,
        ncsi_gen_cmd_07,
        ncsi_gen_cmd_08,
        ncsi_gen_cmd_09,
        ncsi_gen_cmd_0a,
        ncsi_gen_cmd_0b,
        ncsi_gen_cmd_0c,
        ncsi_gen_cmd_0d,
        ncsi_gen_cmd_0e,
        ncsi_gen_cmd_0f,
        ncsi_gen_cmd_10,
        ncsi_gen_cmd_11,
        ncsi_gen_cmd_12,
        ncsi_gen_cmd_13,
        ncsi_gen_cmd_14,
        ncsi_gen_cmd_15,
        ncsi_gen_cmd_16,
        ncsi_gen_cmd_17,
        ncsi_gen_cmd_18,
        ncsi_gen_cmd_19,
        ncsi_gen_cmd_1a,
    };

    ncsi_init_req(cmd, buf);

    if (cmd < NCSI_CMD) {
        if (cmds[cmd])
            cmds[cmd](buf);
    } else {
        gen_cmd_unsupport(buf);
    }
}