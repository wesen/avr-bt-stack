#ifndef BT_L2CAP_H__
#define BT_L2CAP_H__

#define L2CAP_MTU 256

typedef enum {
   l2cap_evt_cmd_rej_not_understood,
   l2cap_evt_cmd_rej_mtu_exceeded,
   l2cap_evt_cmd_rej_cid_invalid,

   l2cap_evt_conn_req,
   l2cap_evt_conn_rsp,

   l2cap_evt_conf_req,
   l2cap_evt_conf_rsp,

   l2cap_evt_disconn_req,
   l2cap_evt_disconn_rsp,

   l2cap_evt_echo_req,
   l2cap_evt_echo_rsp,

   l2cap_evt_info_req,
   l2cap_evt_info_rsp,

   l2cap_evt_conn_data,
   l2cap_evt_connless_data,

   l2cap_evt_garbage
} bt_l2cap_evt_e;


typedef enum {
   state_closed = 0,
   state_w4_connect_rsp,
   state_w4_rconnect_rsp,
   state_config,
   state_open,
   state_w4_disconnect_rsp,
   state_w4_rdisconnect_rsp
} l2cap_ch_state_e;

typedef struct bt_l2cap_ch_s {
   unsigned char    buf[L2CAP_MTU], *ptr;
   unsigned short   src_cid, dst_cid, len, psm;
   l2cap_ch_state_e state;
} bt_l2cap_ch_t;

#define L2CAP_CMD_REJ     0x01
#define L2CAP_CONN_REQ    0x02
#define L2CAP_CONN_RSP    0x03
#define L2CAP_CONF_REQ    0x04
#define L2CAP_CONF_RSP    0x05
#define L2CAP_DISCONN_REQ 0x06
#define L2CAP_DISCONN_RSP 0x07
#define L2CAP_ECHO_REQ    0x08
#define L2CAP_ECHO_RSP    0x09
#define L2CAP_INFO_REQ    0x0a
#define L2CAP_INFO_RSP    0x0b

#endif /* BT_L2CAP_H__ */
