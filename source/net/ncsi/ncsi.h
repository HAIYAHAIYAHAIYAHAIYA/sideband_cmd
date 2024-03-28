#ifndef __NCSI_H__
#define __NCSI_H__

#include "protocol_device.h"

#define NCSI_ETHER_TYPE     (0x88F8)

#define NCSI_PKG_ID         0
#define NCSI_DEV_NUMS       MAX_LAN_NUM
#define NCSI_PACKAGE_SHIFT  5
#define GET_PID(chid)       ((chid) >> NCSI_PACKAGE_SHIFT)       // package id
#define GET_CID(chid)       ((chid) & 0x1F)     // channel id

#define NCSI_PKG_DISABLE    0
#define NCSI_PKG_ENABLE     1

#define NCSI_CHN_READY      0
#define NCSI_CHN_ENABLE     1
#define NCSI_CHN_TX_ENABLE  2

typedef unsigned short  __be16;
typedef unsigned int    __be32;

#pragma pack(1)
/* 16 bytes */
typedef struct {
  unsigned char mc_id;        /* Management controller ID */
  unsigned char revision;     /* NCSI version - 0x01      */
  unsigned char reserved;     /* Reserved                 */
  unsigned char id;           /* Packet sequence number   */
  unsigned char type;         /* Packet type              */
  unsigned char channel;      /* Network controller ID    */

  /**
   * This value does not include the length of the NC-SI header
   * checksum value, or any padding that might be present.
   */
  __be16        length;       /* Payload length           */
  __be32        reserved1[2]; /* Reserved                 */
} ncsi_pkt_hdr;

typedef struct {
  //ether_pkt_hdr ether;        /* Ethernet packet header    */
  ncsi_pkt_hdr  common;       /* Common NCSI packet header */
} ncsi_cmd_pkt_hdr;

typedef struct {
  //ether_pkt_hdr       ether;  /* Ethernet packet header    */
  ncsi_pkt_hdr        common; /* Common NCSI packet header */
  __be16              code;   /* Response code             */
  __be16              reason; /* Response reason           */
} ncsi_rsp_pkt_hdr;

typedef struct {
  //ether_pkt_hdr       ether;        /* Ethernet packet header    */
  ncsi_pkt_hdr        common;       /* Common NCSI packet header */
  unsigned char       reserved2[3]; /* Reserved                  */
  unsigned char       type;         /* AEN packet type           */
} ncsi_aen_pkt_hdr;

/* NCSI common command packet */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header */
  __be32                  checksum; /* Checksum       */
  unsigned char           pad[26];
} ncsi_cmd_pkt;

typedef struct {
  ncsi_rsp_pkt_hdr        rsp;      /* Response header */
  __be32                  checksum; /* Checksum        */
  unsigned char           pad[22];
} ncsi_rsp_pkt;

/* 252 bytes */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;      /* Response header */
  __be32                  checksum; /* Checksum        */
  unsigned char           pad[214];
} ncsi_rsp_msg;


/* Select Package */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;            /* Command header */
  unsigned char           reserved[3];    /* Reserved       */
  unsigned char           hw_arbitration; /* HW arbitration */
  __be32                  checksum;       /* Checksum       */
  unsigned char           pad[22];
} ncsi_cmd_sp_pkt;

/* Disable Channel */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;         /* Command header  */
  unsigned char           reserved[3]; /* Reserved        */
  unsigned char           ald;         /* Allow link down */
  __be32                  checksum;    /* Checksum        */
  unsigned char           pad[22];
} ncsi_cmd_dc_pkt;

/* Reset Channel */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header */
  __be32                  reserved; /* Reserved       */
  __be32                  checksum; /* Checksum       */
  unsigned char           pad[22];
} ncsi_cmd_rc_pkt;

/* AEN Enable */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;         /* Command header   */
  unsigned char           reserved[3]; /* Reserved         */
  unsigned char           mc_id;       /* MC ID            */
  __be32                  mode;        /* AEN working mode */
  __be32                  checksum;    /* Checksum         */
  unsigned char           pad[18];
} ncsi_cmd_ae_pkt;

/* Set Link */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header    */
  __be32                  mode;     /* Link working mode */
  __be32                  oem_mode; /* OEM link mode     */
  __be32                  checksum; /* Checksum          */
  unsigned char           pad[18];
} ncsi_cmd_sl_pkt;

/* Set VLAN Filter */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;       /* Command header    */
  __be16                  reserved;  /* Reserved          */
  __be16                  vlan;      /* VLAN ID           */
  __be16                  reserved1; /* Reserved          */
  unsigned char           index;     /* VLAN table index  */
  unsigned char           enable;    /* Enable or disable */
  __be32                  checksum;  /* Checksum          */
  unsigned char           pad[18];
} ncsi_cmd_svf_pkt;

/* Enable VLAN */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;         /* Command header   */
  unsigned char           reserved[3]; /* Reserved         */
  unsigned char           mode;        /* VLAN filter mode */
  __be32                  checksum;    /* Checksum         */
  unsigned char           pad[22];
} ncsi_cmd_ev_pkt;

/* Set MAC Address */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header          */
  unsigned char           mac[6];   /* MAC address             */
  unsigned char           index;    /* MAC table index         */
  unsigned char           at_e;     /* Addr type and operation */
  __be32                  checksum; /* Checksum                */
  unsigned char           pad[18];
} ncsi_cmd_sma_pkt;

/* Enable Broadcast Filter */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header */
  __be32                  mode;     /* Filter mode    */
  __be32                  checksum; /* Checksum       */
  unsigned char           pad[22];
} ncsi_cmd_ebf_pkt;

/* Enable Global Multicast Filter */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header */
  __be32                  mode;     /* Global MC mode */
  __be32                  checksum; /* Checksum       */
  unsigned char           pad[22];
} ncsi_cmd_egmf_pkt;

/* Set NCSI Flow Control */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;         /* Command header    */
  unsigned char           reserved[3]; /* Reserved          */
  unsigned char           mode;        /* Flow control mode */
  __be32                  checksum;    /* Checksum          */
  unsigned char           pad[22];
} ncsi_cmd_snfc_pkt;

/* Get Link Status */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;        /* Response header   */
  __be32                  status;     /* Link status       */
  __be32                  other;      /* Other indications */
  __be32                  oem_status; /* OEM link status   */
  __be32                  checksum;
  unsigned char           pad[10];
} ncsi_rsp_gls_pkt;

/* Get Version ID */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;          /* Response header */
  __be32                  ncsi_version; /* NCSI version    */
  unsigned char           reserved[3];  /* Reserved        */
  unsigned char           alpha2;       /* NCSI version    */
  unsigned char           fw_name[12];  /* fw name string  */
  __be32                  fw_version;   /* fw version      */
  __be16                  pci_ids[4];   /* PCI IDs         */
  __be32                  mf_id;        /* Manufacture ID  */
  __be32                  checksum;
} ncsi_rsp_gvi_pkt;

/* Get Capabilities */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;         /* Response header   */
  __be32                  cap;         /* Capabilities      */
  __be32                  bc_cap;      /* Broadcast cap     */
  __be32                  mc_cap;      /* Multicast cap     */
  __be32                  buf_cap;     /* Buffering cap     */
  __be32                  aen_cap;     /* AEN cap           */
  unsigned char           vlan_cnt;    /* VLAN filter count */
  unsigned char           mixed_cnt;   /* Mix filter count  */
  unsigned char           mc_cnt;      /* MC filter count   */
  unsigned char           uc_cnt;      /* UC filter count   */
  unsigned char           reserved[2]; /* Reserved          */
  unsigned char           vlan_mode;   /* VLAN mode         */
  unsigned char           channel_cnt; /* Channel count     */
  __be32                  checksum;    /* Checksum          */
} ncsi_rsp_gc_pkt;

/* Get Parameters */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;          /* Response header       */
  unsigned char           mac_cnt;      /* Number of MAC addr    */
  unsigned char           reserved[2];  /* Reserved              */
  unsigned char           mac_enable;   /* MAC addr enable flags */
  unsigned char           vlan_cnt;     /* VLAN tag count        */
  unsigned char           reserved1;    /* Reserved              */
  __be16                  vlan_enable;  /* VLAN tag enable flags */
  __be32                  link_mode;    /* Link setting          */
  __be32                  bc_mode;      /* BC filter mode        */
  __be32                  valid_modes;  /* Valid mode parameters */
  unsigned char           vlan_mode;    /* VLAN mode             */
  unsigned char           fc_mode;      /* Flow control mode     */
  unsigned char           reserved2[2]; /* Reserved              */
  __be32                  aen_mode;     /* AEN mode              */
  unsigned char           mac[6];       /* Supported MAC addr    */
  __be16                  vlan;         /* Supported VLAN tags   */
  __be32                  checksum;     /* Checksum              */
} ncsi_rsp_gp_pkt;

/* Get Controller Packet Statistics */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;            /* Response header            */
  __be32                  cnt_hi;         /* Counter cleared            */
  __be32                  cnt_lo;         /* Counter cleared            */
  __be32                  rx_bytes;       /* Rx bytes                   */
  __be32                  tx_bytes;       /* Tx bytes                   */
  __be32                  rx_uc_pkts;     /* Rx UC packets              */
  __be32                  rx_mc_pkts;     /* Rx MC packets              */
  __be32                  rx_bc_pkts;     /* Rx BC packets              */
  __be32                  tx_uc_pkts;     /* Tx UC packets              */
  __be32                  tx_mc_pkts;     /* Tx MC packets              */
  __be32                  tx_bc_pkts;     /* Tx BC packets              */
  __be32                  fcs_err;        /* FCS errors                 */
  __be32                  align_err;      /* Alignment errors           */
  __be32                  false_carrier;  /* False carrier detection    */
  __be32                  runt_pkts;      /* Rx runt packets            */
  __be32                  jabber_pkts;    /* Rx jabber packets          */
  __be32                  rx_pause_xon;   /* Rx pause XON frames        */
  __be32                  rx_pause_xoff;  /* Rx XOFF frames             */
  __be32                  tx_pause_xon;   /* Tx XON frames              */
  __be32                  tx_pause_xoff;  /* Tx XOFF frames             */
  __be32                  tx_s_collision; /* Single collision frames    */
  __be32                  tx_m_collision; /* Multiple collision frames  */
  __be32                  l_collision;    /* Late collision frames      */
  __be32                  e_collision;    /* Excessive collision frames */
  __be32                  rx_ctl_frames;  /* Rx control frames          */
  __be32                  rx_64_frames;   /* Rx 64-bytes frames         */
  __be32                  rx_127_frames;  /* Rx 65-127 bytes frames     */
  __be32                  rx_255_frames;  /* Rx 128-255 bytes frames    */
  __be32                  rx_511_frames;  /* Rx 256-511 bytes frames    */
  __be32                  rx_1023_frames; /* Rx 512-1023 bytes frames   */
  __be32                  rx_1522_frames; /* Rx 1024-1522 bytes frames  */
  __be32                  rx_9022_frames; /* Rx 1523-9022 bytes frames  */
  __be32                  tx_64_frames;   /* Tx 64-bytes frames         */
  __be32                  tx_127_frames;  /* Tx 65-127 bytes frames     */
  __be32                  tx_255_frames;  /* Tx 128-255 bytes frames    */
  __be32                  tx_511_frames;  /* Tx 256-511 bytes frames    */
  __be32                  tx_1023_frames; /* Tx 512-1023 bytes frames   */
  __be32                  tx_1522_frames; /* Tx 1024-1522 bytes frames  */
  __be32                  tx_9022_frames; /* Tx 1523-9022 bytes frames  */
  __be32                  rx_valid_bytes; /* Rx valid bytes             */
  __be32                  rx_runt_pkts;   /* Rx error runt packets      */
  __be32                  rx_jabber_pkts; /* Rx error jabber packets    */
  __be32                  checksum;       /* Checksum                   */
} ncsi_rsp_gcps_pkt;

/* Get NCSI Statistics */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;           /* Response header         */
  __be32                  rx_cmds;       /* Rx NCSI commands        */
  __be32                  dropped_cmds;  /* Dropped commands        */
  __be32                  cmd_type_errs; /* Command type errors     */
  __be32                  cmd_csum_errs; /* Command checksum errors */
  __be32                  rx_pkts;       /* Rx NCSI packets         */
  __be32                  tx_pkts;       /* Tx NCSI packets         */
  __be32                  tx_aen_pkts;   /* Tx AEN packets          */
  __be32                  checksum;      /* Checksum                */
} ncsi_rsp_gns_pkt;

/* Get NCSI Pass-through Statistics */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;            /* Response header     */
  __be32                  tx_pkts;        /* Tx packets          */
  __be32                  tx_dropped;     /* Tx dropped packets  */
  __be32                  tx_channel_err; /* Tx channel errors   */
  __be32                  tx_us_err;      /* Tx undersize errors */
  __be32                  rx_pkts;        /* Rx packets          */
  __be32                  rx_dropped;     /* Rx dropped packets  */
  __be32                  rx_channel_err; /* Rx channel errors   */
  __be32                  rx_us_err;      /* Rx undersize errors */
  __be32                  rx_os_err;      /* Rx oversize errors  */
  __be32                  checksum;       /* Checksum            */
} ncsi_rsp_gnpts_pkt;

/* Get package status */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;      /* Response header             */
  __be32                  status;   /* Hardware arbitration status */
  __be32                  checksum;
  unsigned char           pad[18];
} ncsi_rsp_gps_pkt;

/* Get package UUID */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;      /* Response header */
  unsigned char           uuid[16]; /* UUID            */
  __be32                  checksum;
  unsigned char           pad[6];
} ncsi_rsp_gpuuid_pkt;

/* AEN: Link State Change */
typedef struct {
  ncsi_aen_pkt_hdr        aen;        /* AEN header      */
  __be32                  status;     /* Link status     */
  __be32                  oem_status; /* OEM link status */
  __be32                  checksum;   /* Checksum        */
  unsigned char           pad[14];
} ncsi_aen_lsc_pkt;

/* AEN: Configuration Required */
typedef struct {
  ncsi_aen_pkt_hdr        aen;      /* AEN header */
  __be32                  checksum; /* Checksum   */
  unsigned char           pad[22];
} ncsi_aen_cr_pkt;

/* AEN: Host Network Controller Driver Status Change */
typedef struct {
  ncsi_aen_pkt_hdr        aen;      /* AEN header */
  __be32                  status;   /* Status     */
  __be32                  checksum; /* Checksum   */
  unsigned char           pad[18];
} ncsi_aen_hncdsc_pkt;

/* OEM CMD: 0x5A, power events flag */
typedef struct {
  ncsi_cmd_pkt_hdr        cmd;         /* Command header    */
  unsigned char           reserved[3]; /* Reserved          */
  unsigned char           flag;        /* power events flag */
  __be32                  checksum;    /* Checksum          */
  unsigned char           pad[22];
} ncsi_oem_pef_pkt;

/* Get SN */
typedef struct {
  ncsi_rsp_pkt_hdr        rsp;      /* Response header             */
  __be32                  m_id;
  unsigned char           sn[9];    /* serial number, SN_LEN */
  __be32                  checksum;
  unsigned char           pad[8];
} ncsi_rsp_oem_pkt;

typedef struct {
  ncsi_cmd_pkt_hdr        cmd;      /* Command header */
  __be32                  m_id;     /* Manufacturer ID (IANA)  */
  __be32                  checksum; /* Checksum       */
  unsigned char           pad[22];
} ncsi_cmd_oem_pkt;
#pragma pack()

/* NCSI packet revision */
#define NCSI_PKT_REVISION   0x01

/* NCSI packet commands */
#define NCSI_PKT_CMD_CIS    0x00 /* Clear Initial State              */
#define NCSI_PKT_CMD_SP     0x01 /* Select Package                   */
#define NCSI_PKT_CMD_DP     0x02 /* Deselect Package                 */
#define NCSI_PKT_CMD_EC     0x03 /* Enable Channel                   */
#define NCSI_PKT_CMD_DC     0x04 /* Disable Channel                  */
#define NCSI_PKT_CMD_RC     0x05 /* Reset Channel                    */
#define NCSI_PKT_CMD_ECNT   0x06 /* Enable Channel Network Tx        */
#define NCSI_PKT_CMD_DCNT   0x07 /* Disable Channel Network Tx       */
#define NCSI_PKT_CMD_AE     0x08 /* AEN Enable                       */
#define NCSI_PKT_CMD_SL     0x09 /* Set Link                         */
#define NCSI_PKT_CMD_GLS    0x0a /* Get Link                         */
#define NCSI_PKT_CMD_SVF    0x0b /* Set VLAN Filter                  */
#define NCSI_PKT_CMD_EV     0x0c /* Enable VLAN                      */
#define NCSI_PKT_CMD_DV     0x0d /* Disable VLAN                     */
#define NCSI_PKT_CMD_SMA    0x0e /* Set MAC address                  */
#define NCSI_PKT_CMD_EBF    0x10 /* Enable Broadcast Filter          */
#define NCSI_PKT_CMD_DBF    0x11 /* Disable Broadcast Filter         */
#define NCSI_PKT_CMD_EGMF   0x12 /* Enable Global Multicast Filter   */
#define NCSI_PKT_CMD_DGMF   0x13 /* Disable Global Multicast Filter  */
#define NCSI_PKT_CMD_SNFC   0x14 /* Set NCSI Flow Control            */
#define NCSI_PKT_CMD_GVI    0x15 /* Get Version ID                   */
#define NCSI_PKT_CMD_GC     0x16 /* Get Capabilities                 */
#define NCSI_PKT_CMD_GP     0x17 /* Get Parameters                   */
#define NCSI_PKT_CMD_GCPS   0x18 /* Get Controller Packet Statistics */
#define NCSI_PKT_CMD_GNS    0x19 /* Get NCSI Statistics              */
#define NCSI_PKT_CMD_GNPTS  0x1a /* Get NCSI Pass-throu Statistics   */
#define NCSI_PKT_CMD_GPS    0x1b /* Get package status               */
#define NCSI_PKT_CMD_OEM    0x50 /* OEM                              */
#define NCSI_PKT_CMD_PLDM   0x51 /* PLDM request over NCSI over RBT  */
#define NCSI_PKT_CMD_GPUUID 0x52 /* Get package UUID                 */

/* NCSI packet responses */
#define NCSI_PKT_RSP_CIS    (NCSI_PKT_CMD_CIS    + 0x80)
#define NCSI_PKT_RSP_SP     (NCSI_PKT_CMD_SP     + 0x80)
#define NCSI_PKT_RSP_DP     (NCSI_PKT_CMD_DP     + 0x80)
#define NCSI_PKT_RSP_EC     (NCSI_PKT_CMD_EC     + 0x80)
#define NCSI_PKT_RSP_DC     (NCSI_PKT_CMD_DC     + 0x80)
#define NCSI_PKT_RSP_RC     (NCSI_PKT_CMD_RC     + 0x80)
#define NCSI_PKT_RSP_ECNT   (NCSI_PKT_CMD_ECNT   + 0x80)
#define NCSI_PKT_RSP_DCNT   (NCSI_PKT_CMD_DCNT   + 0x80)
#define NCSI_PKT_RSP_AE     (NCSI_PKT_CMD_AE     + 0x80)
#define NCSI_PKT_RSP_SL     (NCSI_PKT_CMD_SL     + 0x80)
#define NCSI_PKT_RSP_GLS    (NCSI_PKT_CMD_GLS    + 0x80)
#define NCSI_PKT_RSP_SVF    (NCSI_PKT_CMD_SVF    + 0x80)
#define NCSI_PKT_RSP_EV     (NCSI_PKT_CMD_EV     + 0x80)
#define NCSI_PKT_RSP_DV     (NCSI_PKT_CMD_DV     + 0x80)
#define NCSI_PKT_RSP_SMA    (NCSI_PKT_CMD_SMA    + 0x80)
#define NCSI_PKT_RSP_EBF    (NCSI_PKT_CMD_EBF    + 0x80)
#define NCSI_PKT_RSP_DBF    (NCSI_PKT_CMD_DBF    + 0x80)
#define NCSI_PKT_RSP_EGMF   (NCSI_PKT_CMD_EGMF   + 0x80)
#define NCSI_PKT_RSP_DGMF   (NCSI_PKT_CMD_DGMF   + 0x80)
#define NCSI_PKT_RSP_SNFC   (NCSI_PKT_CMD_SNFC   + 0x80)
#define NCSI_PKT_RSP_GVI    (NCSI_PKT_CMD_GVI    + 0x80)
#define NCSI_PKT_RSP_GC     (NCSI_PKT_CMD_GC     + 0x80)
#define NCSI_PKT_RSP_GP     (NCSI_PKT_CMD_GP     + 0x80)
#define NCSI_PKT_RSP_GCPS   (NCSI_PKT_CMD_GCPS   + 0x80)
#define NCSI_PKT_RSP_GNS    (NCSI_PKT_CMD_GNS    + 0x80)
#define NCSI_PKT_RSP_GNPTS  (NCSI_PKT_CMD_GNPTS  + 0x80)
#define NCSI_PKT_RSP_GPS    (NCSI_PKT_CMD_GPS    + 0x80)
#define NCSI_PKT_RSP_OEM    (NCSI_PKT_CMD_OEM    + 0x80)
#define NCSI_PKT_RSP_PLDM   (NCSI_PKT_CMD_PLDM   + 0x80)
#define NCSI_PKT_RSP_GPUUID (NCSI_PKT_CMD_GPUUID + 0x80)

/* NCSI response code/reason */
#define NCSI_PKT_RSP_C_COMPLETED    0x0000 /* Command Completed        */
#define NCSI_PKT_RSP_C_FAILED       0x0001 /* Command Failed           */
#define NCSI_PKT_RSP_C_UNAVAILABLE  0x0002 /* Command Unavailable      */
#define NCSI_PKT_RSP_C_UNSUPPORTED  0x0003 /* Command Unsupported      */
#define NCSI_PKT_RSP_R_NO_ERROR     0x0000 /* No Error                 */
#define NCSI_PKT_RSP_R_INTERFACE    0x0001 /* Interface not ready      */
#define NCSI_PKT_RSP_R_PARAM        0x0002 /* Invalid Parameter        */
#define NCSI_PKT_RSP_R_CHANNEL      0x0003 /* Channel not Ready        */
#define NCSI_PKT_RSP_R_PACKAGE      0x0004 /* Package not Ready        */
#define NCSI_PKT_RSP_R_LENGTH       0x0005 /* Invalid payload length   */
#define NCSI_PKT_RSP_R_UNKNOWN      0x7fff /* Command type unsupported */

/* NCSI AEN packet type */
#define NCSI_PKT_AEN        0xFF /* AEN Packet               */
#define NCSI_PKT_AEN_LSC    0x00 /* Link status change       */
#define NCSI_PKT_AEN_CR     0x01 /* Configuration required   */
#define NCSI_PKT_AEN_HNCDSC 0x02 /* HNC driver status change */

typedef struct {
  ncsi_cmd_pkt    *request;
  ncsi_rsp_msg    *respond;

  unsigned char   channel_id;
  unsigned char   channel_status;  // ready bit0, bit1 chan_enable bit2 tx_enable

  // bit0, 0 - lan down, 1 - lan up
  // bit1, 0 - Auto negotiation, 1 - Auto negotiation done
  unsigned char   lan_status;

  unsigned char   aen_mcid;       // AEN MC ID get from AEN enable cmd
  unsigned char   aen_ctrl;       // AEN control flag

  unsigned char   vlan_mode;      // 0 - disable, 1 - 3 for enable_vlan()
  unsigned char   brdc_mode;      // 0 - disable, Broadcast Packet Filter Settings
  unsigned char   mulc_mode;      // 0 - disable, Multicast Packet Filter Settings

  unsigned int    link_mode;      // Set link mode
} ncsi_chn_dev;

//Standard Response Code
#define COMMAND_COMPLETED               0x0000
#define COMMAND_FAILED                  0x0001
#define COMMAND_UNAVAILABLE           0x0002
#define COMMAND_UNSUPPORTED           0x0003

//Standard Reason Code
#define NO_ERROR                          0x0000
#define INTERFACE_INIT_REQUIRED   0x0001
#define PARAMETER_IS_INVALID        0x0002
#define CHANNEL_NOT_READY               0x0003
#define PACKAGE_NOT_READY               0x0004
#define INVALID_PAYLOAD_LENGTH    0x0005
#define UNKNOWN_COMMAND_TYPE      0x7FFF

/* vlan fiter num = tag num */
#define NCSI_VLAN_TAG_NUM    1

/* ref  NC-SI spec 1.01.pdf */
#define NCSI_NORMAL_CMD         (0x1A + 1)     // ncsi cmd max idx is 0x1A
#define NCSI_OEM_CMD            (0x50)
typedef void (*ncsi_cmd_func)(protocol_msg_t *skb, ncsi_chn_dev *dev);

#define WXKJ                        0x57584B4A
#define NCSI_FW_NAME                "WXKJ-FW"
#define NCSI_FW_NAME_SIZE           7


// void ncsi_task(void *param);
#if AMBER == 1
int ncsi_task_init(void);
#endif
void ncsi_rx2(protocol_msg_t *skb);
void ncsi_init(int max_port);

#endif /* __NCSI_H__ */
