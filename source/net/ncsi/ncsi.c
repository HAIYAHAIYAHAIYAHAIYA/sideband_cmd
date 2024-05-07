// #include "ncsi.h"
// #include "mctp.h"

// /* 0 for deselect package */
// /* 1 for select package */
// static u8             __attribute__((aligned(4))) gs_ncsi_pkg_status = 0xFF;
// static u8             __attribute__((aligned(4))) gs_ncsi_current_chn = 0xFF;
// //static ncsi_cmd_pkt   __attribute__((aligned(4))) gs_ncsi_chn_cmd;
// //static ncsi_cmd_pkt   __attribute__((aligned(4))) gs_ncsi_request[NCSI_DEV_NUMS];
// //static ncsi_rsp_msg   __attribute__((aligned(4))) gs_ncsi_respond[NCSI_DEV_NUMS];
// static ncsi_chn_dev   __attribute__((aligned(4))) gs_ncsi_channel[NCSI_DEV_NUMS];

// //static SemaphoreHandle_t ncsi_sem = NULL;

// extern link_sts_t g_link_sts[MAX_LAN_NUM];    /* read from lan task */
// extern sys_ctrl_t g_sys;

// int rbt_send_to(protocol_msg_t *skb, int pkt_len);
// void lan_to_bmc_pause(u8 en);

// //static u8 gs_ncsi_rbuf_mem[4096];
// //rbuf_t ncsi_buf;

// //#define LOG(INFO, ...)

// /* return current connected port */
// static u8 ncsi_get_valid_port(void)
// {
//     u8 port;
//     static u8 port_bak = 0;
//     u8 found = FALSE;

//     for (port = 0; port < g_sys.cfg.port_num; port++) {
//         if (g_link_sts[port].link == LINK_UP) {
//             found = TRUE;
//             break;
//         }
//     }

//     if (found == FALSE) {
//         port = (port_bak + 1) % g_sys.cfg.port_num;     /* circle */
//     }

//     port_bak = port;
//     return port;
// }

// /*!
//  *  ref linux ncsi-cmd.c
//  *  param:
//  *      len = sizeof(ncsi_pkt_hdr) + dev->respond->rsp.common.length;
//  *      dat = &dev->respond->rsp.common;
//  */
// UNUSED u32 ncsi_calc_checksum(u8 *data, int len)
// {
//     u32 checksum = 0;
//     int i;

//     for (i = 0; i < len; i += 2)
//         checksum += (((u32)data[i] << 8) | data[i + 1]);

//     checksum = (~checksum + 1);
//     return checksum;
// }

// static u32 ncsi_lan_status(int port)
// {
//     link_sts_t sts = g_link_sts[port];

//     u32 status = 0;

//     LOG("ncsi port %d link %x, speed 0x%X", port, sts.link, sts.speed);

//     if (sts.link == LINK_UP) {
//         status = 1;           // Link is up
//         status |= 1 << 6;     // Auto-negotiation has completed
//     }

//     switch (sts.speed) {
//         case LINK_SPEED_10_FULL:
//             status |= 0x02 << 1;
//             break;
//         case LINK_SPEED_100_FULL:
//             status |= 0x05 << 1;
//             break;
//         case LINK_SPEED_1GB_FULL:
//             status |= 0x07 << 1;
//             break;
//         default:
//             status |= 0x02 << 1;  // 10BASE-T full-duplex
//             break;
//     }

//     // Auto-negotiation is enabled
//     status |= 1 << 5;
//     // Transmission of Pause frames by the NC onto the external
//     // network interface is enabled
//     status |= 1 << 16;
//     // Reception of Pause frames by the NC from the external
//     // network interface is enabled
//     status |= 1 << 17;
//     // Link partner supports symmetric pause
//     status |= 1 << 18;

//     return htonl(status);
// }

// static int ncsi_check_chn(ncsi_chn_dev *dev, int pos, int flag)
// {
//     if (pos == NCSI_CHN_READY) { /* set init / ready status */
//         dev->channel_status = flag;
//     } else if (flag) {
//         dev->channel_status |= 1 << pos;
//     } else {
//         dev->channel_status &= ~(1 << pos);
//     }

//     return 0;
// }

// #if 0
// // return new tail or NULL
// u8 *fifo_push_data(u8 *buf, int len, u8 *head, u8 *tail, u8 *pbgn, u8 *pend)
// {
//    int plen;
//    int free_buf_len = 0;

//    plen = ALIGN(len, 16);//(len + 15) & 0xFFFFFFF0; /* 16B align */

//    free_buf_len = (int)(tail - head);          // busy

//    if (free_buf_len >= 0)
//        free_buf_len = (int)(pend - pbgn) - free_buf_len;       // fifo.size - busy
//    else
//        free_buf_len = -free_buf_len;   /* rollback case */     //

//    if (free_buf_len < plen) {
//        return NULL; /* No enough buf */
//    }

//    if (tail >= head) {
//         int len2;
//         len2 = (int)(pend - tail);      // left
//         if (len2 > plen) {
//             memcpy_fast(tail, buf, plen);
//             return tail + plen;
//         } else if (len2 == plen) {
//             memcpy_fast(tail, buf, plen);
//             return pbgn;
//         } else {
//             memcpy_fast(tail, buf, len2);
//             memcpy_fast(pbgn, buf + len2, plen - len2);
//             return pbgn + plen - len2;
//         }
//    }

//    /* rollback case */
//    memcpy_fast(tail, buf, plen);

//    return tail + plen;
// }
// #endif

// void ncsi_init_dev(ncsi_chn_dev *dev, int ch_id)
// {
//     cm_memcpy(dev, 0, sizeof(ncsi_chn_dev));

//     dev->channel_id = ch_id;
//     //dev->request = gs_ncsi_request + ch_id;
//     //dev->respond = gs_ncsi_respond + ch_id;

//     //memset(dev->respond->rsp.ether.src, 0xFF, sizeof(dev->respond->rsp.ether.src));
//     //memset(dev->respond->rsp.ether.dst, 0xFF, sizeof(dev->respond->rsp.ether.dst));

//     //dev->respond->rsp.ether.pro = htons(NCSI_ETHER_TYPE);
//     //dev->respond->rsp.common.revision = 0x01;
// }

// void ncsi_init(int port_num)
// {
//     gs_ncsi_current_chn = 0xFF;
//     gs_ncsi_pkg_status = 0xFF;

//     for (int i = 0; i < port_num; ++i) {
//         ncsi_init_dev(gs_ncsi_channel + i, i);

//         /*! Atention! set dev->channel_status ready as default;
//         *    It may different to NCSI protocol, cause resp reason code should be Interface Initialization Required
//         *    before recieve Clear Initial State command.
//         *    But linux kernal unsupport reason-code expect NO_ERROR. Ref: linux/net/ncsi/ncsi_pkt.h
//         *          #define NCSI_PKT_RSP_R_INTERFACE    0x0001  // Interface not ready
//         */
//         ncsi_check_chn(gs_ncsi_channel + i, NCSI_CHN_READY, 1);
//     }

//     //rbuf_init(&ncsi_buf, gs_ncsi_rbuf_mem, sizeof(u8), sizeof(gs_ncsi_rbuf_mem));
// }

// void ncsi_init_resp(ncsi_chn_dev *dev)
// {
//     cm_memcpy(dev->respond, 0, sizeof(ncsi_rsp_msg));
//     //memset(dev->respond->rsp.ether.src, 0xFF, sizeof(dev->respond->rsp.ether.src));
//     //memset(dev->respond->rsp.ether.dst, 0xFF, sizeof(dev->respond->rsp.ether.dst));

//     //dev->respond->rsp.ether.pro = htons(NCSI_ETHER_TYPE);
//     dev->respond->rsp.common = dev->request->cmd.common;
//     dev->respond->rsp.common.revision = 0x01;
//     dev->respond->rsp.common.type += 0x80;
//     dev->respond->rsp.common.length = 0x04; /* reason + code */
//     // �ᵼ�²��� BMC �����ݾܾ���Ӧ����
//     // dev->respond->rsp.common.channel = (NCSI_PKG_ID << NCSI_PACKAGE_SHIFT) | dev->channel_id;

//     dev->respond->rsp.code = COMMAND_COMPLETED;
//     dev->respond->rsp.reason = NO_ERROR;

//     // set by cmd 0 or init status.
//     if ((dev->channel_status & BIT(NCSI_CHN_READY)) == 0) {
//         dev->respond->rsp.reason = INTERFACE_INIT_REQUIRED;
//         LOG("chan id %d cmd 0x%X reason 0x%X", dev->channel_id, dev->respond->rsp.common.type, dev->respond->rsp.reason);
//     }
// }

// void ncsi_init_aen(ncsi_chn_dev *dev)
// {
//     ncsi_aen_pkt_hdr *resp = (ncsi_aen_pkt_hdr *)dev->respond;

//     cm_memcpy(dev->respond, 0, sizeof(ncsi_aen_lsc_pkt));
//     //memset(resp->ether.src, 0xFF, sizeof(resp->ether.src));
//     //memset(resp->ether.dst, 0xFF, sizeof(resp->ether.dst));

//     //resp->ether.pro = htons(NCSI_ETHER_TYPE);
//     resp->common.mc_id        = 0;
//     resp->common.revision     = NCSI_PKT_REVISION;
//     resp->common.type         = NCSI_PKT_AEN;
//     resp->common.channel      = dev->channel_id;
//     resp->common.length       = 4;
// }

// #if 0
// void ncsi_tx(ncsi_chn_dev *dev)
// {
//     int           plen;
//     //u8            *ptr;
//     //volatile u8   *head;
//     //volatile u8   *tail;
//     //volatile u8   *pbgn;
//     //volatile u8   *pend;
//     //mng_bmc_poh_t *poh;
//     ncsi_rsp_msg  *respond;

//     respond = dev->respond;
//     plen = sizeof(ncsi_pkt_hdr)      /* 16 */
//             + respond->rsp.common.length    /* each cmd diff */
//             + 4;                            /* checksum */

//     LOG("plen %d, len %d", plen, respond->rsp.common.length);

//     // length must be network order
//     respond->rsp.common.length = htons(respond->rsp.common.length);

//     poh = (mng_bmc_poh_t *)(&respond->rsp.ether.po[0]);

//     poh->pkt_len = plen - POH_LEN;
//     poh->maclen = ETH_HDR_LEN;
//     poh->crcp = FALSE;      /* let hw do insert */

//     head = (volatile u8 *)REG(EBFRH);  // (volatile u8 *)(EBFRH);
//     tail = (volatile u8 *)REG(EBFRT);  // (volatile u8 *)(*EBFRT);
//     pbgn = (u8 *)fifo_table[EB].bgn;
//     pend = (u8 *)fifo_table[EB].end;
//     ptr = fifo_push_data((u8 *)respond, plen,
//                     (u8 *)head, (u8 *)tail,
//                     (u8 *)pbgn, (u8 *)pend);
//     if (!ptr) {
//     //        g_hal_pt->Unlock(key);
//         LOG("ncsi tx not enough fifo, head %x, tail %x, pbgn %x, pend %x",
//                 head, tail, pbgn, pend);
//         return;
//     }

//     LOG("ncsi_tx: ABFRT %x", (u32)ptr);
//     REG(EBFRT) = (u32)ptr;
//     // g_hal_pt->Unlock(key);
// }
// #endif

// void ncsi_tx2(protocol_msg_t *skb)
// {
//     ncsi_rsp_msg  *respond = (ncsi_rsp_msg  *)skb->rsp_buf;
//     int plen = sizeof(ncsi_pkt_hdr)      /* 16 */
//             + respond->rsp.common.length    /* each cmd diff */
//             + 4;                            /* checksum */

//     LOG("plen %d, len %d", plen, respond->rsp.common.length);

//     // length must be network order
//     respond->rsp.common.length = htons(respond->rsp.common.length);
//     skb->send_to_2(skb, plen);
// }

// // cmd 0
// static void clear_initial_state(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     if (ncsi_check_chn(dev, NCSI_CHN_READY, 1) < 0)
//         return;
//     LOG("cmd: 00, channel %d", dev->channel_id);

//     ncsi_init_resp(dev);

//     ncsi_tx2(skb);

// //    ncsi_try_setup_lanx(dev->channel_id);
// }

// // cmd 1
// static void select_package(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     gs_ncsi_pkg_status = NCSI_PKG_ENABLE;

//     LOG("cmd: 01, channel %d", dev->channel_id);

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 2
// static void deselect_package(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     gs_ncsi_pkg_status = NCSI_PKG_DISABLE;

//     LOG("cmd: 02, channel %d", dev->channel_id);

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 3
// static void enable_channel(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     if (ncsi_check_chn(dev, NCSI_CHN_ENABLE, 1) < 0)
//         return;
//     LOG("cmd: 03, enable channel %d", dev->channel_id);

//     ncsi_init_resp(dev);

//     if (g_link_sts[dev->channel_id].link == LINK_DOWN) {
//         /* if code == COMMAND_FAILED, some BMCs will consider the network card as not supporting NCSI.*/
//         //dev->respond->rsp.code = COMMAND_FAILED;
//         dev->respond->rsp.reason = CHANNEL_NOT_READY;
//         LOG("ncsi chan enable, ignore");
//         goto resp;
//     }
//     lan_port_enable(dev->channel_id, ENABLE);
// resp:
//     ncsi_tx2(skb);
// }

// // cmd 4
// static void disable_channel(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     if (ncsi_check_chn(dev, NCSI_CHN_ENABLE, 0) < 0)
//         return;
//     LOG("cmd: 04, disable channel %d", dev->channel_id);
//     lan_port_enable(dev->channel_id, DISABLE);

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 5 ignore
// static void reset_channel(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_cmd_pkt *req = dev->request;
//     ncsi_rsp_msg *rsp = dev->respond;

//     ncsi_init_dev(dev, dev->channel_id);

//     LOG("cmd: 05, reset channel %d", dev->channel_id);

//     /* do nothing */

//     dev->request = req;
//     dev->respond = rsp;

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// /* cmd 6 Atention:
//  *   if lan disconnect, bmc_port_enable would couse fifo overflow
//  *   and ncsi no ack,  so check link status before bmc_port_enable !
//  */
// static void enable_network_tx(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     if (ncsi_check_chn(dev, NCSI_CHN_TX_ENABLE, 1) < 0) return;

//     LOG("cmd: 06, enable tx for chan %d", dev->channel_id);

//     ncsi_init_resp(dev);
//     gs_ncsi_current_chn = dev->channel_id;

//     if (g_link_sts[dev->channel_id].link == LINK_DOWN) {
//         /* if code == COMMAND_FAILED, some BMCs will consider the network card as not supporting NCSI.*/
//         //dev->respond->rsp.code = COMMAND_FAILED;
//         dev->respond->rsp.reason = CHANNEL_NOT_READY;
//         LOG("ncsi tx enable, ignore");
//         goto resp;
//     }

//     bmc_port_enable(dev->channel_id);          // enable bmc to lan
//     lan_port_enable(dev->channel_id, ENABLE);  // enable lan to bmc

// resp:
//     ncsi_tx2(skb);
// }

// // cmd 7
// static void disable_network_tx(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     if (ncsi_check_chn(dev, NCSI_CHN_TX_ENABLE, 0) < 0) return;
//     LOG("cmd: 07, disable tx for channel %d", dev->channel_id);

//     // todo : set cfg to hardware
//     if (dev->channel_id == gs_ncsi_current_chn) {
//         bmc_port_disable();                         // disable bmc to lan
//         lan_port_enable(dev->channel_id, DISABLE);  // disable lan to bmc
//         gs_ncsi_current_chn = 0xFF;
//     }

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 8
// static void enable_aen(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_cmd_ae_pkt *req = (ncsi_cmd_ae_pkt *)dev->request;
//     LOG("cmd: 08, enable aen");

//     dev->aen_mcid = req->mc_id;
//     dev->aen_ctrl = ntohl(req->mode);

//     // todo : set cfg to hardware
//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 9
// static void set_link(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_cmd_sl_pkt *req = (ncsi_cmd_sl_pkt *)dev->request;

//     dev->link_mode = ntohl(req->mode);
//     LOG("cmd: 09, link mode 0x%x for channel %d", dev->link_mode, dev->channel_id);

//     // set link config to hardware

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 10
// static void get_link_status(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_rsp_gls_pkt *resp = (ncsi_rsp_gls_pkt *)dev->respond;

//     ncsi_init_resp(dev);
//     resp->rsp.common.length += 12;

//     resp->status = ncsi_lan_status(dev->channel_id);
//     LOG("cmd: 0a, status 0x%x for channel %d", (u32)resp->status, dev->channel_id);
//     ncsi_tx2(skb);
// }

// // cmd 11
// static void set_vlan_filter(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     int index;
//     ncsi_cmd_svf_pkt *req = (ncsi_cmd_svf_pkt *)dev->request;
//     int vlan = ntohs(req->vlan);

//     LOG("cmd: 0b, set vlan 0x%x flag 0x%x for channel %d", vlan, req->enable, dev->channel_id);

//     ncsi_init_resp(dev);

//     index = req->index;     /* count start with 1 ... */
//     if (index < 1 || index > NCSI_VLAN_TAG_NUM) {
//         LOG("set_vlan_filter error index %d", index);
//         dev->respond->rsp.reason = PARAMETER_IS_INVALID;
//         goto resp;
//     }

//     // Set vlan to hardware
//     index -= 1;
//     if (req->enable) {
//         psr_vlan_add(vlan, index, dev->channel_id);
//     } else {
//         psr_vlan_del(index, dev->channel_id);
//     }

// resp:
//     ncsi_tx2(skb);
// }

// // cmd 12, risk !
// static void enable_vlan(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_cmd_ev_pkt *req = (ncsi_cmd_ev_pkt *)dev->request;

//     ncsi_init_resp(dev);

//     if (req->mode != 1) {
//         LOG("enable_vlan error mode 0x%x", req->mode);
//         dev->respond->rsp.reason = PARAMETER_IS_INVALID;
//         goto resp;
//     }

//     // Enable vlan to hardware
//     dev->vlan_mode = req->mode;
//     psr_vlan_enable(dev->channel_id);

// resp:
//     ncsi_tx2(skb);
// }

// // cmd 13
// static void disable_vlan(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     dev->vlan_mode = 0;
//     LOG("cmd: 0d, disable vlan for channel %d", dev->channel_id);

//     // Disable vlan to hardware
//     psr_vlan_disable(dev->channel_id);

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 14
// static void set_mac_address(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     int ret = 0;
//     ncsi_cmd_sma_pkt *req = (ncsi_cmd_sma_pkt *)dev->request;

//     LOG("cmd: e, at_e %x mac: %02x:%02x:%02x:%02x:%02x:%02x",
//             req->at_e,
//             req->mac[0], req->mac[1], req->mac[2],
//             req->mac[3], req->mac[4], req->mac[5]);

//     //int port = dev->channel_id;

//     if (req->at_e & 1) {  // add
//         ret = psr_mng_mac_add(req->mac, dev->channel_id, 0);
//     } else {              // del
//         ret = psr_mng_mac_del(dev->channel_id, 0);
//     }

//     if (ret) {
//         LOG("set_mac_address: error %d", ret);
//     }

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 16
// static void enable_broadcast_filter(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_cmd_ebf_pkt *req = (ncsi_cmd_ebf_pkt *)dev->request;

//     dev->brdc_mode = ntohl(req->mode);
//     // Enable broadcast filter to hardware
//     LOG("cmd: 10, port %d brdc en: 0x%X", dev->channel_id, dev->brdc_mode);

//     if (dev->brdc_mode & CBIT(0)) { // arp
//         if(dev->channel_id == gs_ncsi_current_chn) {  // only for ncsi port
//             psr_arp_enable(dev->channel_id);
//         }
//     } else {
//         psr_arp_disable(dev->channel_id);
//     }

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 17
// static void disable_broadcast_filter(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     // Receive all broadcast pkt
//     dev->brdc_mode = 0;
//     psr_arp_disable(dev->channel_id);

//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// // cmd 18
// static void enable_global_multicast_filter(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_cmd_egmf_pkt *req = (ncsi_cmd_egmf_pkt *)dev->request;

//     dev->mulc_mode = ntohl(req->mode);
//     // Enable global multicast filter to hardware

//     psr_global_multicast_enable(dev->channel_id, dev->mulc_mode);

//     ncsi_init_resp(dev);
//     dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//     ncsi_tx2(skb);
// }

// // cmd 19
// static void disable_global_multicast_filter(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     // Receive all global multicast pkt
//     dev->mulc_mode = 0;

//     psr_global_multicast_disable(dev->channel_id);

//     ncsi_init_resp(dev);
//     dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//     ncsi_tx2(skb);
// }

// // cmd 20
// static void set_flow_control(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     //int mode;
//     //ncsi_cmd_snfc_pkt *req = (ncsi_cmd_snfc_pkt *)dev->request;

//     //mode = req->mode;
//     // Set flow control to hardware
//     // Flow control regs: XOFF_thre, XON_thre

//     ncsi_init_resp(dev);
//     dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//     ncsi_tx2(skb);
// }

// // cmd 21
// static void get_version_id(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_rsp_gvi_pkt *resp = (ncsi_rsp_gvi_pkt *)dev->respond;

//     ncsi_init_resp(dev);
//     resp->rsp.common.length += 36;

//     // NC-SI Spec Version 1.0.1
//     resp->ncsi_version = htonl(0xF1F0F100);
//     // Firmware Name
//     cm_strncpy((char *)resp->fw_name, NCSI_FW_NAME, NCSI_FW_NAME_SIZE);

// #if AMBER == 1
//     // Firmware Version
//     resp->fw_version = htonl(g_sys.version);
//     // PCI, 0x00 for NOT Used
//     resp->pci_ids[0] = 0x00;
// #elif AMLITE == 1
//     resp->fw_version = htonl(g_sys.fw_version);
//     cm_memcpy(resp->pci_ids, g_sys.pci_ids, sizeof(resp->pci_ids));
// #endif

//     // IANA 0x57584B4A 'WXKJ'
//     resp->mf_id = htonl(WXKJ);

//     ncsi_tx2(skb);
// }

// // cmd 22
// static void get_capabilities(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_rsp_gc_pkt *resp = (ncsi_rsp_gc_pkt *)dev->respond;

//     ncsi_init_resp(dev);
//     resp->rsp.common.length += 28;

//     // Network Controller to Management Controller flow
//     // control is supported
//     resp->cap = 1 << 2;
//     // Management Controller to Network Controller flow
//     // control is supported
//     resp->cap |= 1 << 3;
//     resp->cap = htonl(resp->cap);

//     // Spec. Table 69
//     // ARP Packets
//     resp->bc_cap = 1 << 0;
//     resp->bc_cap = htonl(resp->bc_cap);

//     // Spec. Table 74
//     // Not support
//     resp->mc_cap = 0;

//     // Buffering Capability 4KB
//     resp->buf_cap = htonl(1024 * 4);

//     // AEN Control Support
//     // Spec. Table 38
//     // Link Status Change AEN control     bit0
//     // Configuration Required AEN control bit1
//     resp->aen_cap = htonl(CBIT(0) | CBIT(1));

//     // VLAN Filter Count
//     resp->vlan_cnt = NCSI_VLAN_TAG_NUM;
//     // Mixed Filter Count
//     resp->mixed_cnt = 1;
//     // Multicast Filter Count
//     resp->mc_cnt = 1;
//     // Unicast Filter Count
//     resp->uc_cnt = 1;
//     // VLAN Mode Support
//     resp->vlan_mode = CBIT(0);// |   // VLAN only
//                     //CBIT(1) |   // VLAN + non-VLAN, NOT support in SP
//                     //CBIT(2);    // Any VLAN + non-VLAN, so just mac filter
//     // Channel Count
//     resp->channel_cnt = g_sys.cfg.port_num;

//     ncsi_tx2(skb);
// }

// // cmd 23
// static void get_parameters(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_init_resp(dev);
//     dev->respond->rsp.code = COMMAND_COMPLETED;
//     dev->respond->rsp.reason = NO_ERROR;

//     ncsi_rsp_gp_pkt *resp = (ncsi_rsp_gp_pkt *)dev->respond;
//     resp->rsp.common.length += sizeof(ncsi_rsp_gp_pkt) - sizeof(ncsi_rsp_pkt_hdr) - 4;

//     /* Number of MAC addr    */
//     resp->mac_cnt = 1;              // The number of MAC addresses supported by the channel

//     /* MAC addr enable flags */
//     resp->mac_enable = CBIT(0);     // MAC address 1 status enable

//     /* VLAN tag count        */
//     resp->vlan_cnt = NCSI_VLAN_TAG_NUM;

//     /* VLAN tag enable flags */
//     u16 vlan_mode = dev->vlan_mode ? CBIT(0) : 0;  /* VLAN Tag 1 status enable */
//     resp->vlan_enable = htons(vlan_mode);

//     /* Link setting          */
//     resp->link_mode = htonl(dev->link_mode);

//     /* BC filter mode        */
//     resp->bc_mode = htonl(dev->brdc_mode);

//     /* Valid mode parameters configuration flags */
//     resp->valid_modes |= dev->channel_status & (BIT(NCSI_CHN_TX_ENABLE) | BIT(NCSI_CHN_ENABLE));
//     resp->valid_modes |= dev->brdc_mode ? CBIT(0) : 0;
//     //resp->valid_modes |= 0;//dev->mulc_mode ? CBIT(3) : 0; // no support now !
//     resp->valid_modes = htonl(resp->valid_modes);

//     /* VLAN mode             */
//     resp->vlan_mode = dev->vlan_mode;

//     /* Flow control mode     */
//     resp->fc_mode = 0;                     // no support now, set_flow_control

//     /* AEN mode              */
//     resp->aen_mode = htonl(dev->aen_ctrl);

//     /* Supported MAC addr, bmc mac   */
//     psr_mng_mac_read(dev->channel_id, 0, resp->mac);

//     /* Supported VLAN tags, read from reg   */
//     u32 vlan = 0;
//     psr_vlan_read(&vlan, 0, dev->channel_id);

//     resp->vlan = htons(vlan);    // The current contents of up to 15 16-bit VLAN Tag filter values

//     resp->checksum = 0;                     /* ignore */

//     ncsi_tx2(skb);
// }

// // cmd 24
// static void get_controller_statistics(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_init_resp(dev);
//     dev->respond->rsp.code = COMMAND_FAILED;
//     dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//     ncsi_tx2(skb);
// }

// // cmd 25
// static void get_ncsi_statistics(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_init_resp(dev);
//     dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//     ncsi_tx2(skb);
// }

// // cmd 26
// static void get_ncsi_pass_through(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_init_resp(dev);
//     dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//     ncsi_tx2(skb);
// }

// #if 0
// // cmd 27
// static void get_package_status(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//   ncsi_init_resp(dev);
//   dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//   ncsi_tx2(skb);
// }

// // cmd 0x51
// static void pldm(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//   ncsi_init_resp(dev);
//   dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//   ncsi_tx2(skb);
// }

// // cmd 0x52
// static void get_package_uuid(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//   ncsi_init_resp(dev);
//   dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//   ncsi_tx2(skb);
// }
// #endif

// static void oem_get_sn(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_rsp_oem_pkt *resp =  (ncsi_rsp_oem_pkt *)dev->respond;

//     ncsi_init_resp(dev);
//     fw_get_sn(resp->sn);

//     resp->m_id = htonl(WXKJ);

//     resp->rsp.common.length += SN_LEN + 4;

//     ncsi_tx2(skb);
// }

// // cmd 0x5A
// static void oem_power_events(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//     ncsi_init_resp(dev);
//     ncsi_tx2(skb);
// }

// static void unsupported_cmd(protocol_msg_t *skb, ncsi_chn_dev *dev)
// {
//   LOG("the cmd is not supported");
//   ncsi_init_resp(dev);
//   dev->respond->rsp.reason = COMMAND_UNSUPPORTED;
//   ncsi_tx2(skb);
// }

// /* normal ncsi cmd (except oem) */
// static ncsi_cmd_func ncsi_org_cmd_table[NCSI_NORMAL_CMD] =
// {
//     clear_initial_state,                // 0x00, optional, phy reset
//     select_package,                     // 0x01
//     deselect_package,                   // 0x02
//     enable_channel,                     // 0x03, key
//     disable_channel,                    // 0x04, key
//     reset_channel,                      // 0x05
//     enable_network_tx,                  // 0x06, key
//     disable_network_tx,                 // 0x07, key
//     enable_aen,                         // 0x08
//     set_link,                           // 0x09
//     get_link_status,                    // 0x0A, key
//     set_vlan_filter,                    // 0x0B, optional
//     enable_vlan,                        // 0x0C, optional
//     disable_vlan,                       // 0x0D, optional
//     set_mac_address,                    // 0x0E, key
//     unsupported_cmd,                    // 0x0F
//     enable_broadcast_filter,            // 0x10, optional
//     disable_broadcast_filter,           // 0x11, optional
//     enable_global_multicast_filter,     // 0x12, optional
//     disable_global_multicast_filter,    // 0x13, optional
//     set_flow_control,                   // 0x14
//     get_version_id,                     // 0x15
//     get_capabilities,                   // 0x16
//     get_parameters,                     // 0x17
//     get_controller_statistics,          // 0x18
//     get_ncsi_statistics,                // 0x19
//     get_ncsi_pass_through,              // 0x1A
// };

// #if 0
// void ncsi_cmd_proc(ncsi_cmd_pkt *req)
// {
//     u8 channel;
//     ncsi_chn_dev *dev;
//     ncsi_cmd_func cmd_proc = NULL;
//     u8 cmd;
//     u8 pkg_id;

//     channel = GET_CID(req->cmd.common.channel);
//     pkg_id = GET_PID(req->cmd.common.channel);
//     cmd = req->cmd.common.type;

//     if (cmd != 0x0A) // polling link status cmd
//         LOG("ncsi cmd %x, chan %X", req->cmd.common.type, channel);

//     if (channel == 0x1F) {  // 0x1F for all channels
//         channel = ncsi_get_valid_port();
//         LOG("[W] valid port: 0x1F -> %d", channel);
//     }

//     if (channel >= g_sys.cfg.port_num || pkg_id != NCSI_PKG_ID) {
//         LOG("[W] unsupport ! pkg_id %d, chan %d", pkg_id, channel);
//         return; // do not respones
//     }

//     dev = gs_ncsi_channel + channel;
//     *dev->request = *req;

//     if (cmd < NCSI_NORMAL_CMD) {
//         cmd_proc = ncsi_org_cmd_table[cmd];
//     } else if (cmd == NCSI_OEM_CMD) {
//         cmd_proc = oem_get_sn;          // get serial number
//     } else if (cmd == 0x5A) {           // to be remove this case
//         cmd_proc = oem_power_events;
//     } else {
//         cmd_proc = unsupported_cmd;
//     }

//     cmd_proc(NULL, dev);

//     return ;
// }
// #endif

// /* backup data ro ncsi_rbuf
//  * auto remove the po header
//  * add a new header = (F888 << 16 | len);
//  */
// #if 0
// void ncsi_recv_data(fifo_t *fifo)
// {
//     u32 rd;     /* temp of fifo_rd */
//     u32 pkt_len;
//     u32 hdr;
//     sts_t sts;
//     int pkt_cnt = 0;

//     while (fifo->wt != fifo->rd)
//     {
//         if (pkt_cnt++ > FIFO_LEN / 64) {
//             g_sys.event |= EV_PKT;
//             MARKER(g_sys.marker);
//             break;
//         }

//         /* fast check len, BMC_PO_GET_PKT_LEN */
//         pkt_len = MEM32(fifo->rd) & 0x3FFF;
//         if (pkt_len == 0 || pkt_len >= FIFO_LEN) {
//             g_sys.event |= EV_PKT;
//             MARKER(g_sys.marker);
//             break;
//         }

//         /* rm header, rollback proc */
//         rd = fifo->bgn + (fifo->rd - fifo->bgn + POH_LEN) % fifo->size;

//         pkt_len = ALIGN(pkt_len, 4);

//         /* make header */
//         hdr = pkt_len;  /* len < 2048 */

//         sts = rbuf_write(&ncsi_buf, &hdr, 1);
//         CHECK(sts, STS_SUC, break);

//         /* copy msg data */
//         if (rd + pkt_len <= fifo->end) {
//             sts = rbuf_write(&ncsi_buf, (u32 *)rd, pkt_len >> 2);
//             CHECK(sts, STS_SUC, break);
//         } else { /* rollback bgn_tail_____rd___end */
//             /* proc [rd end) */
//             sts = rbuf_write(&ncsi_buf, (u32 *)rd, (fifo->end - rd) >> 2);
//             CHECK(sts, STS_SUC, break);

//             /* proc [bgn tail) */
//             sts = rbuf_write(&ncsi_buf, (u32 *)fifo->bgn, (pkt_len - (fifo->end - rd)) >> 2);
//             CHECK(sts, STS_SUC, break);
//         }

//         fifo->rd = fifo->bgn + (fifo->rd - fifo->bgn + ALIGN(pkt_len, PKT_ALIGN) + POH_LEN) % fifo->size;

//         if (fifo->wt != fifo->rd)
//             LOG("more data %x %x", fifo->rd, fifo->wt);
//     }

//     /* post sem */
//     portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
//     xSemaphoreGiveFromISR(ncsi_sem, &xHigherPriorityTaskWoken);
//     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// }
// #endif

// /* format: hrd (F8888) + len (16bit) + ncsi cmd */
// #if 0
// void ncsi_rx(void)
// {
//     ncsi_cmd_pkt *hdr_cmd = &gs_ncsi_chn_cmd;
//     sts_t sts;
//     u32 hdr = 0;
//     u16 ether_type;
//     u16 pkt_len;

//     while (1)
//     {
//         sts = rbuf_read(&ncsi_buf, (u32 *)&hdr, 1);
//         if (sts != STS_SUC)     // no need to check, it must be ok
//             break;

//         ether_type = hdr >> 16;
//         pkt_len = hdr & 0xFFFF;

//         /*  just for a check */
//         if ((hdr >> 16) != 0xF888 || pkt_len == 0 || pkt_len >= FIFO_LEN) {
//             /* never reach here */
//             LOG("type %x len %d", ether_type, pkt_len);
//             rbuf_dump(&ncsi_buf, 16);
//             break;
//         }

//         //sts = rbuf_read(&ncsi_buf, (u32 *)hdr_cmd->cmd.ether.dst, pkt_len >> 2);
//         sts = rbuf_read(&ncsi_buf, (u32 *)hdr_cmd, pkt_len >> 2);
//         CHECK(sts, STS_SUC, break);

//         ncsi_cmd_proc(hdr_cmd);
//     }
// }
// #endif

// void ncsi_rx2(protocol_msg_t *skb)
// {
//     u8 channel;
//     ncsi_chn_dev *dev;
//     ncsi_cmd_func cmd_proc = NULL;
//     u8 cmd;
//     u8 pkg_id;
//     ncsi_cmd_pkt *req = (ncsi_cmd_pkt *)skb->req_buf;

//     channel = GET_CID(req->cmd.common.channel);
//     pkg_id = GET_PID(req->cmd.common.channel);
//     cmd = req->cmd.common.type;

//     if (cmd != 0x0A) // polling link status cmd
//         LOG("ncsi cmd %x, chan %X", req->cmd.common.type, channel);

//     if (channel == 0x1F) {  // 0x1F for all channels
//         channel = ncsi_get_valid_port();
//         LOG("[W] valid port: 0x1F -> %d", channel);
//     }

//     if (channel >= g_sys.cfg.port_num || pkg_id != NCSI_PKG_ID) {
//         LOG("[W] unsupport ! pkg_id %d, chan %d", pkg_id, channel);
//         return; // do not respones
//     }

//     // cmd2 is deselect package, so pause lan2bmc flow
//     lan_to_bmc_pause(cmd == 0x02);

//     dev = gs_ncsi_channel + channel;
//     dev->request = req;
//     dev->respond = (ncsi_rsp_msg *)skb->rsp_buf;

//     if (cmd < NCSI_NORMAL_CMD) {
//         cmd_proc = ncsi_org_cmd_table[cmd];
//     } else if (cmd == NCSI_OEM_CMD) {
//         cmd_proc = oem_get_sn;          // get serial number
//     } else if (cmd == 0x5A) {           // to be remove this case
//         cmd_proc = oem_power_events;
//     } else {
//         cmd_proc = unsupported_cmd;
//     }

//     cmd_proc(skb, dev);
// }

// static u8 gs_ncsi_aen_buf[128];
// // AEN msg send to rbt only
// void ncsi_set_lan_status(int port)
// {
//     ncsi_chn_dev *dev = gs_ncsi_channel + port;
//     dev->respond = (ncsi_rsp_msg *)(gs_ncsi_aen_buf + 48); // give some buf for header
//     ncsi_aen_lsc_pkt *resp = (ncsi_aen_lsc_pkt *)dev->respond;

// //    if ((dev->aen_ctrl & NCSI_PKT_AEN_LSC) == 0)
// //        return ;

//     LOG("AEN LSC port %d", port);

//     ncsi_init_aen(dev);
//     resp->aen.common.length += 8;
//     resp->aen.type = NCSI_PKT_AEN_LSC;

//     resp->status = ncsi_lan_status(dev->channel_id);

//     protocol_msg_t msg;
//     msg.req_buf = NULL;
//     msg.rsp_buf = (u8 *)dev->respond;
//     msg.send_to_2 = rbt_send_to; // AEN only send to rbt
//     ncsi_tx2(&msg);
// }

// /* Configuration request */
// // AEN msg send to rbt only
// void ncsi_aen_cfg_req(int port)
// {
//     ncsi_chn_dev *dev = gs_ncsi_channel + port;
//     dev->respond = (ncsi_rsp_msg *)(gs_ncsi_aen_buf + 48); // give some buf for header
//     ncsi_aen_cr_pkt *resp = (ncsi_aen_cr_pkt *)dev->respond;

// //    if ((dev->aen_ctrl & NCSI_PKT_AEN_CR) == 0)
// //        return ;

//     LOG("AEN CR port %d", port);

//     ncsi_init_aen(dev);
//     resp->aen.type = NCSI_PKT_AEN_CR;     // CR type 0x01

//     protocol_msg_t msg;
//     msg.req_buf = NULL;
//     msg.rsp_buf = (u8 *)dev->respond;
//     msg.send_to_2 = rbt_send_to; // AEN only send to rbt
//     ncsi_tx2(&msg);
// }

// int ncsi_is_current_lan(u8 port)
// {
//     return gs_ncsi_current_chn == port;
// }

// u8 ncsi_get_current_lan(void)
// {
//     return gs_ncsi_current_chn;
// }

// u8 ncsi_get_pkg_status(void)
// {
//     return gs_ncsi_pkg_status;
// }

// void ncsi_set_pkg_status(u8 val)
// {
//     gs_ncsi_pkg_status = val;
// }

// #if AMBER == 1
// #include "bmac.h"
// /* task list */
// int ncsi_task_init(void)
// {
//     u8 bmac_addr[] = {0x4E, 0x66, 0x6B, 0x86, 0x8D, 0xA3};		// for test
//     lan_id_t lan = {.id  = 0};

//     TRACE(rpsr_init(lan.chan));

//     /* mac */
//     TRACE(bmc_mac_init());

//     /* io drive set to max */
//     ncsi_set_io_drive();

//     ncsi_auto_send(ENABLE);

//     /* */
//     ncsi_64byte_pkt_limit(DISABLE);

//     /* ncsi protocal */
//     TRACE(ncsi_init(g_sys.cfg.port_num));

//     /* bmc to mng */
//     flt_bmc_to_ecu();

//     /* bmc to lan */
//     flt_bmc_to_lan(lan);

//     /* lan to bmc */
//     {
//         rpsr_lan_enable(lan);           // move to base init

//         rlan_mask_init(lan.chan, TRUE);

//         rpsr_mng_share_to_host(lan.chan, BIT(RPSR_MDID_ARP) | BIT(RPSR_MDID_MC));

//         rpsr_md_arp_enable(lan.chan);

//         rpsr_md_uc_enable(lan.chan);

//         rpsr_md_mc_enable(lan.chan);
//     }

//     /* bmc mac addr */
//     rpsr_mng_mac_add(lan.chan, 0, bmac_addr);

//     //ncsi_sem = xSemaphoreCreateBinary();
//     //CHECK(ncsi_sem != NULL, TRUE, return STS_ERR);

//     return 0;
// }
// #endif

// // void ncsi_task(void *param)
// // {
// //     u8 xStatus;

// //     LOG("ncsi clk vld %d", ncsi_rmii_clk_valid());

// //     while (1)
// //     {
// //         xStatus = xSemaphoreTake(ncsi_sem, portMAX_DELAY);
// //         CHECK(xStatus, pdPASS, continue);

// //         //ncsi_rx();
// //     }
// // }
