#ifndef BT_DEV_H__
#define BT_DEV_H__

#ifdef STACK_UNIX
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif  /* STACK_UNIX */

typedef enum {
   dev_evt_none = 0,
   dev_evt_timeout,

   dev_evt_garbage,
   dev_evt_acl,
   dev_evt_sco,

   dev_evt_inquiry_results,
   dev_evt_inquiry_complete_succ,
   dev_evt_inquiry_complete_unsucc,

   dev_evt_conn_request,
   dev_evt_conn_complete_succ,
   dev_evt_conn_complete_unsucc,
   dev_evt_disconn_complete_succ,
   dev_evt_disconn_complete_unsucc,

   dev_evt_auth_complete_succ,
   dev_evt_auth_complete_unsucc,

   dev_evt_pincode_req,
   dev_evt_link_key_not,
   dev_evt_link_key_req,

   dev_evt_link_key_reply_succ,
   dev_evt_link_key_reply_unsucc,
   dev_evt_link_key_reply_neg_succ,
   dev_evt_link_key_reply_neg_unsucc,

   dev_evt_pincode_reply_succ,
   dev_evt_pincode_reply_unsucc,
   dev_evt_pincode_reply_neg_succ,
   dev_evt_pincode_reply_neg_unsucc,

   dev_evt_read_features_succ,
   dev_evt_read_features_unsucc,

   dev_evt_num_comp_pkts,
   dev_evt_max_slots_chg,

#ifdef  BT_ERICSSON
   dev_evt_ericsson_set_baudrate_succ,
   dev_evt_ericsson_set_baudrate_unsucc,
#endif /* BT_ERICSSON */

   dev_evt_set_evt_flt_succ,
   dev_evt_set_evt_flt_unsucc,
   dev_evt_write_auth_enable_succ,
   dev_evt_write_auth_enable_unsucc,
   dev_evt_write_scan_enable_succ,
   dev_evt_write_scan_enable_unsucc,
   dev_evt_change_name_succ,
   dev_evt_change_name_unsucc,
   dev_evt_read_local_feat_succ,
   dev_evt_read_local_feat_unsucc,
   dev_evt_read_bd_addr_succ,
   dev_evt_read_bd_addr_unsucc


} bt_dev_evt_e;

#define PEER_STATE_EMPTY          0
#define PEER_STATE_FOUND          1
#define PEER_STATE_SHOULD_CONNECT 2
#define PEER_STATE_CONNECTING     3

typedef struct bt_peer_s {
   unsigned char  state;
   unsigned char  bd_addr[6], cod[3];
   unsigned short clock_offset, pkt_type;
   unsigned char  pscan_rep_mode, pscan_per_mode, pscan_mode;
} bt_peer_t;

#define ACL_STATE_EMPTY          0
#define ACL_STATE_SHOULD_CONNECT 1
#define ACL_STATE_CONNECTED      2
#define ACL_STATE_SHOULD_CLOSE   3
#define ACL_STATE_CLOSED         4

typedef struct bt_acl_s {
   unsigned char  state;
   unsigned short handle;
   unsigned short pkt_types;
   unsigned char  max_slots;
   unsigned char  encrypt_mode;
   bt_peer_t      *peer;
} bt_acl_t;

#define BT_DEV_STATE_RESET 0
#define BT_DEV_STATE_INITIALIZING 1
#define BT_DEV_STATE_INITIALIZED  2
#define BT_DEV_STATE_READY        3
#define BT_DEV_STATE_INQUIRY      4
#define BT_DEV_STATE_CONNECTING   5
#define BT_DEV_STATE_DISCONNECTING 6

typedef struct bt_dev_s {
   unsigned char  state;

   unsigned char  ncmds;

   unsigned char  buf[256], *ptr;
   char  name[32];
   unsigned short pkt_type;

   struct bt_peer_s peers[NUM_PEERS];
   struct bt_acl_s  acls[NUM_ACLS];

#ifdef STACK_UNIX
   char           tty[128];
   int            ttyfd;
   struct termios save_termios;
#endif /* STACK_UNIX */
} bt_dev_t;

typedef struct bt_callbacks_s {
   void (*cb_ready)(bt_dev_t *dev);
   int  (*cb_peer_found)(bt_dev_t *dev, bt_peer_t *peer);
   int  (*cb_conn_req)(bt_dev_t *dev, bt_peer_t *peer, 
                       unsigned char link_type);
   void (*cb_conn_succ)(bt_dev_t *dev, bt_acl_t *acl);
   void (*cb_conn_unsucc)(bt_dev_t *dev, bt_peer_t *peer);
   void (*cb_disconn_succ)(bt_dev_t *dev, bt_acl_t *acl, 
                           unsigned char reason);
   void (*cb_conn_data)(bt_dev_t *dev, bt_acl_t *acl);
   unsigned char *(*cb_pincode_req)(bt_dev_t *dev, bt_peer_t *peer);
   unsigned char *(*cb_link_key_req)(bt_dev_t *dev, bt_peer_t *peer);
   void (*cb_idle)(bt_dev_t *dev);
} bt_callbacks_t;

void bt_dev_init(bt_dev_t *dev);

bt_peer_t *bt_dev_get_free_peer(bt_dev_t *dev);
bt_peer_t *bt_dev_get_peer_by_bd_addr(bt_dev_t *dev, unsigned char *bd_addr);
bt_acl_t *bt_dev_get_free_acl(bt_dev_t *dev);
bt_acl_t *bt_dev_get_acl_by_handle(bt_dev_t *dev, 
                                   unsigned short handle);

int          bt_dev_flush_hci(bt_dev_t *dev);
bt_dev_evt_e bt_dev_read_hci(bt_dev_t *dev, bt_callbacks_t *cb);

void bt_dev_pack_reset(bt_dev_t *dev);
void bt_dev_pack_accept_conn(bt_dev_t *dev, 
                          unsigned char *bd_addr, 
                          unsigned char role);
void bt_dev_pack_reject_conn(bt_dev_t *dev, 
                          unsigned char *bd_addr, 
                          unsigned char reason);
void bt_dev_pack_change_local_name(bt_dev_t *dev, const char *s);
void bt_dev_pack_inq_scan_enable(bt_dev_t *dev);
void bt_dev_pack_clear_evt_flt(bt_dev_t *dev);
void bt_dev_pack_disconn(bt_dev_t *dev, 
                         unsigned short handle,
                         unsigned char reason);
void bt_dev_pack_write_auth_enable(bt_dev_t *dev);
void bt_dev_pack_inquiry(bt_dev_t      *dev, 
                         unsigned char len, 
                         unsigned char num_peers);
void bt_dev_pack_create_conn(bt_dev_t *dev, bt_peer_t *peer,
                             unsigned char  roleswitch);
void bt_dev_pack_link_key_reply(bt_dev_t *dev, 
                                unsigned char *bd_addr,
                                unsigned char *link_key);
void bt_dev_pack_link_key_reply_neg(bt_dev_t *dev, 
                                    unsigned char *bd_addr);
void bt_dev_pack_pincode_reply(bt_dev_t *dev, 
                                unsigned char *bd_addr,
                                unsigned char len,
                                unsigned char *pincode);
void bt_dev_pack_pincode_reply_neg(bt_dev_t *dev, 
                                    unsigned char *bd_addr);
void bt_dev_pack_read_local_features(bt_dev_t *dev);
void bt_dev_pack_read_buffer_size(bt_dev_t *dev);
void bt_dev_pack_read_bd_addr(bt_dev_t *dev);
void bt_dev_pack_l2cap(bt_dev_t *dev, unsigned short handle, 
                       unsigned char *data, int len);

#endif /* BT_DEV_H__ */
