#include "bt_stack.h"

void bt_l2cap_pack_hdr(bt_dev_t *dev, 
                       unsigned short len, 
                       unsigned short cid) {
   UINT16_PACK(dev->ptr, len);
   UINT16_PACK(dev->ptr, cid);
}

void bt_l2cap_pack_cmd(bt_dev_t *dev, 
                       unsigned char code, 
                       unsigned char id, 
                       unsigned short len) {
   bt_l2cap_pack_hdr(dev, len + 4, 1);
   UINT8_PACK(dev->ptr, code);
   UINT8_PACK(dev->ptr, id);
   UINT16_PACK(dev->ptr, len);
}

void bt_l2cap_pack_cmd_not_understood(bt_dev_t *dev) {
   bt_l2cap_pack_cmd(dev, L2CAP_CMD_REJ, 0, 0);
}

bt_l2cap_evt_e bt_l2cap_unpack(bt_dev_t *dev) {
   unsigned short cid, len;

   len = UINT16_UNPACK(dev->ptr);
   cid = UINT16_UNPACK(dev->ptr);

   switch (cid) {
      case 0:
         return l2cap_evt_garbage;

      case 1:
         if (len < 4) {
            return l2cap_evt_garbage;
         } else {
            return bt_l2cap_unpack_cmd(dev);
         }

      case 2:
         return l2cap_evt_connless_data;

      default:
         return l2cap_evt_conn_data;
   }
}

bt_l2cap_evt_e bt_l2cap_unpack_cmd(bt_dev_t *dev) {
   unsigned char  code, id;
   unsigned short len;

   code = UINT8_UNPACK(dev->ptr);
   id   = UINT8_UNPACK(dev->ptr);
   len  = UINT16_UNPACK(dev->ptr);

   switch (code) {
      case L2CAP_CMD_REJ:
         switch (id) {
            case 0:
               return l2cap_evt_cmd_rej_not_understood;

            case 1:
               if (len < 2) {
                  return l2cap_evt_garbage;
               } else {
                  return l2cap_evt_cmd_rej_mtu_exceeded;
               }

            case 2:
               if (len < 4) {
                  return l2cap_evt_garbage;
               } else {
                  return l2cap_evt_cmd_rej_cid_invalid;
               }

            default:
               return l2cap_evt_garbage;
         }

      case L2CAP_CONN_REQ:
         if (len < 4) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_conn_req;
         }

      case L2CAP_CONN_RSP:
         if (len < 8) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_conn_rsp;
         }

      case L2CAP_CONF_REQ:
         if (len < 4) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_conf_req;
         }

      case L2CAP_CONF_RSP:
         if (len < 8) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_conf_rsp;
         }

      case L2CAP_DISCONN_REQ:
         if (len < 4) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_disconn_req;
         }

      case L2CAP_DISCONN_RSP:
         if (len < 4) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_disconn_rsp;
         }

      case L2CAP_ECHO_REQ:
         return l2cap_evt_echo_req;

      case L2CAP_ECHO_RSP:
         return l2cap_evt_echo_rsp;
         
      case L2CAP_INFO_REQ:
         if (len < 2) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_info_req;
         }
         
      case L2CAP_INFO_RSP:
         if (len < 4) {
            return l2cap_evt_garbage;
         } else {
            return l2cap_evt_info_rsp;
         }
         
      default:
         return l2cap_evt_garbage;
   }
}
