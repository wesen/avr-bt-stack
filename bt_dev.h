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

   dev_evt_read_features_succ,
   dev_evt_read_features_unsucc,

   dev_evt_num_comp_pkts,
   dev_evt_max_slots_chg,

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

typedef struct bt_dev_s {
   unsigned char  buf[256], *ptr;
   unsigned short pkt_type;

#ifdef STACK_UNIX
   char           tty[128];
   int            ttyfd;
   struct termios save_termios;
#endif /* STACK_UNIX */
} bt_dev_t;

typedef struct bt_peer_s {
   unsigned char  bd_addr[6], cod[3];
   unsigned short clock_offset, pkt_type;
   unsigned char  pscan_rep_mode, pscan_per_mode, pscan_mode;
} bt_peer_t;

int          bt_dev_flush_hci(bt_dev_t *dev);
bt_dev_evt_e bt_dev_read_hci(bt_dev_t *dev);

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
void bt_dev_pack_l2cap(bt_dev_t *dev, unsigned short handle, 
                       unsigned char *data, int len);

#endif /* BT_DEV_H__ */
