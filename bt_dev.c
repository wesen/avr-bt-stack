#include <string.h>
#include "bt_stack.h"

int bt_dev_flush_hci(bt_dev_t *dev) {
   return bt_serial_send(dev, dev->buf, dev->ptr - dev->buf);
}

bt_dev_evt_e bt_dev_read_hci(bt_dev_t *dev) {
   dev->ptr = dev->buf;

   if (bt_serial_read(dev, dev->ptr, 1) != 1)
      return dev_evt_timeout;

   return bt_hci_unpack_hci(dev);
}

void bt_dev_pack_reset(bt_dev_t *dev) {
   bt_hci_pack_cmd(dev, OGF_HOST_CTL, OCF_RESET, 0);
}

void bt_dev_pack_accept_conn(bt_dev_t *dev, 
                          unsigned char *bd_addr, 
                          unsigned char role) {
   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_ACCEPT_CONN_REQ, 7);
   memcpy((char *)dev->ptr, bd_addr, 6);
   dev->ptr += 6;
   UINT8_PACK(dev->ptr, role);
}

void bt_dev_pack_reject_conn(bt_dev_t *dev, 
                          unsigned char *bd_addr, 
                          unsigned char reason) {
   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_REJECT_CONN_REQ, 7);
   memcpy((char *)dev->ptr, bd_addr, 6);
   dev->ptr += 6;
   UINT8_PACK(dev->ptr, reason);
}

void bt_dev_pack_disconn(bt_dev_t *dev, 
                         unsigned short handle,
                         unsigned char reason) {
   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_DISCONNECT, 3);
   UINT16_PACK(dev->ptr, handle);
   UINT8_PACK(dev->ptr, reason);
}

void bt_dev_pack_create_conn(bt_dev_t *dev, bt_peer_t *peer,
                             unsigned char  roleswitch) {
   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_CREATE_CONN, 13);
   memcpy(dev->ptr, peer->bd_addr, 6); dev->ptr += 6;
   UINT16_PACK(dev->ptr, dev->pkt_type & ACL_PTYPE_MASK);
   UINT8_PACK(dev->ptr,  peer->pscan_rep_mode);
   UINT8_PACK(dev->ptr,  peer->pscan_mode);
   UINT16_PACK(dev->ptr, peer->clock_offset);
   UINT8_PACK(dev->ptr,  roleswitch);
}

void bt_dev_pack_change_local_name(bt_dev_t *dev, const char *s) {
   unsigned char len = strlen(s);

   bt_hci_pack_cmd(dev, OGF_HOST_CTL, OCF_CHANGE_LOCAL_NAME, 248);
   memset((char *)dev->ptr, 0, 248);
   strncpy((char *)dev->ptr, s, len);
   dev->ptr += 248;
}

void bt_dev_pack_inq_scan_enable(bt_dev_t *dev) {
   unsigned char scan_enable = PAGE_SCAN_ENABLE | INQUIRY_SCAN_ENABLE;

   bt_hci_pack_cmd(dev, OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, 1);
   UINT8_PACK(dev->ptr, scan_enable);
}

void bt_dev_pack_clear_evt_flt(bt_dev_t *dev) {
   unsigned char evt_flt = FLT_CLEAR_ALL;

   bt_hci_pack_cmd(dev, OGF_HOST_CTL, OCF_SET_EVENT_FLT, 1);
   UINT8_PACK(dev->ptr, evt_flt);
}

void bt_dev_pack_write_auth_enable(bt_dev_t *dev) {
   unsigned char auth_enable = 1;

   bt_hci_pack_cmd(dev, OGF_HOST_CTL, OCF_WRITE_AUTH_ENABLE, 1);
   UINT8_PACK(dev->ptr, auth_enable);
}

void bt_dev_pack_inquiry(bt_dev_t      *dev, 
                         unsigned char len, 
                         unsigned char num_peers) {
   unsigned char inq_data[5] = {
      GIAC_LAP, /* requested LAP */
      0,       /* Inquiry length in * 1.28 sec */
      0 /* limited number of responses */
   };
   
   inq_data[3] = len;
   inq_data[4] = num_peers;

   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_INQUIRY, 5);
   memcpy(dev->ptr, inq_data, 5); dev->ptr += 5;
}

void bt_dev_pack_link_key_reply(bt_dev_t *dev, 
                                unsigned char *bd_addr,
                                unsigned char *link_key) {
   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_LINK_KEY_REPLY, 22);
   memcpy(dev->ptr, bd_addr, 6); dev->ptr += 6;
   memcpy(dev->ptr, link_key, 16); dev->ptr += 16;
}

void bt_dev_pack_link_key_reply_neg(bt_dev_t *dev, 
                                    unsigned char *bd_addr) {
   bt_hci_pack_cmd(dev, OGF_LINK_CTL, OCF_LINK_KEY_REPLY, 6);
   memcpy(dev->ptr, bd_addr, 6); dev->ptr += 6;
}

void bt_dev_pack_l2cap(bt_dev_t *dev, unsigned short handle, 
                       unsigned char *data, int len) {
   bt_hci_pack_acl(dev, handle, HCI_ACL_START, HCI_ACL_PP, len+4);
   UINT16_PACK(dev->ptr, len);
   UINT16_PACK(dev->ptr, 0x40); /* CID: 0x40 */
   memcpy(dev->ptr, data, len);
   dev->ptr += len;
}
