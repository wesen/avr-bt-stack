#include <string.h>
#include "bt_stack.h"

void bt_hci_pack_cmd(bt_dev_t *dev, unsigned short ogf,
                                    unsigned short ocf,
                                    unsigned char  len) {
   dev->ptr = dev->buf;

   UINT8_PACK(dev->ptr,  hci_cmd_pkt);
   UINT16_PACK(dev->ptr, CMD_OPCODE_PACK(ogf, ocf));
   UINT8_PACK(dev->ptr,  len);
}

void bt_hci_pack_acl(bt_dev_t *dev, unsigned short handle,
                                    unsigned char  boundary,
                                    unsigned char  broadcast,
                                    unsigned char  len) {
   dev->ptr = dev->buf;

   UINT8_PACK(dev->ptr,  hci_acl_pkt);
   UINT16_PACK(dev->ptr, ACL_HANDLE_PACK(handle, boundary, broadcast));
   UINT16_PACK(dev->ptr, len);
}

void bt_hci_pack_sco(bt_dev_t *dev, unsigned short handle,
                                    unsigned char  len) {
   dev->ptr = dev->buf;

   UINT8_PACK(dev->ptr,  hci_sco_pkt);
   UINT16_PACK(dev->ptr, handle);
   UINT16_PACK(dev->ptr, len);
}

bt_dev_evt_e bt_hci_unpack_hci(bt_dev_t *dev) {
   hci_type_e type;

   dev->ptr = dev->buf;
   type = UINT8_UNPACK(dev->ptr);

   switch (type) {
      case hci_evt_pkt:
         DEBUG_STR("HCI EVT");
         return bt_hci_unpack_evt(dev);

      case hci_acl_pkt:
         DEBUG_STR("HCI ACL");
         return dev_evt_acl;

      case hci_sco_pkt:
         DEBUG_STR("HCI SCO");
         return dev_evt_sco;
         
      default:
         return dev_evt_garbage;
   }
}

bt_dev_evt_e bt_hci_unpack_evt(bt_dev_t *dev) {
   unsigned char code, len;

   if (bt_serial_read(dev, dev->ptr, HCI_EVT_SIZE) != HCI_EVT_SIZE)
      return dev_evt_garbage;

   code = UINT8_UNPACK(dev->ptr);
   len  = UINT8_UNPACK(dev->ptr);
   
   if (bt_serial_read(dev, dev->ptr, len) != len)
      return dev_evt_garbage;

   switch (code) {
      case BTEVT_CMD_STATUS:
         DEBUG_STR("CMD STATUS");
         if (len < 4)
            return dev_evt_garbage;
         else 
            return bt_hci_unpack_cmd_status(dev, len);

      case BTEVT_CMD_COMPLETE:
         DEBUG_STR("CMD COMPLETE");
         if (len < 3)
            return dev_evt_garbage;
         else
            return bt_hci_unpack_cmd_complete(dev, len);

      case BTEVT_INQUIRY_COMPLETE:
         DEBUG_STR("INQUIRY COMPLETE");
         if (len < 1) {
            return dev_evt_garbage;
         } else {
            unsigned char status = UINT8_UNPACK(dev->ptr);
            
            if (status == HCI_SUCCESS)
               return dev_evt_inquiry_complete_succ;
            else 
               return dev_evt_inquiry_complete_unsucc;
         }

      case BTEVT_INQUIRY_RESULT:
         DEBUG_STR("INQUIRY RESULT");
         if (len < 1) {
            return dev_evt_garbage;
         } else {
            unsigned char num;

            num = UINT8_UNPACK(dev->ptr);
            if (len < (num * INQUIRY_RESULT_SIZE) + 1) {
               return dev_evt_garbage;
            } else {
               return dev_evt_inquiry_results;
            }
         }

      case BTEVT_CONN_REQUEST:
         DEBUG_STR("CONN REQUEST");
         if (len < 10) {
            return dev_evt_garbage;
         } else {
            return dev_evt_conn_request;
         }

      case BTEVT_CONN_COMPLETE:
         DEBUG_STR("CONN COMPLETE");
         if (len < 11) {
            return dev_evt_garbage;
         } else {
            unsigned char status;
            
            status = UINT8_UNPACK(dev->ptr);

            if (status == HCI_SUCCESS) {
               return dev_evt_conn_complete_succ;
            } else {
               return dev_evt_conn_complete_unsucc;
            }
         }

      case BTEVT_AUTH_COMPLETE:
         DEBUG_STR("AUTH COMPLETE");
         if (len < 3) {
            return dev_evt_garbage;
         } else {
            unsigned char status;

            status = UINT8_UNPACK(dev->ptr);

            if (status == HCI_SUCCESS) {
               return dev_evt_auth_complete_succ;
            } else {
               return dev_evt_auth_complete_unsucc;
            }
         }

      case BTEVT_DISCONN_COMPLETE:
         DEBUG_STR("DISCONN COMPLETE");
         if (len < 4) {
            return dev_evt_garbage;
         } else {
            unsigned char status;

            status = UINT8_UNPACK(dev->ptr);

            if (status == HCI_SUCCESS) {
               return dev_evt_disconn_complete_succ;
            } else {
               return dev_evt_disconn_complete_unsucc;
            }
         }

      case BTEVT_NUM_COMP_PKTS:
         DEBUG_STR("NUM OF COMPLETED PACKETS");
         if (len < 1) {
            return dev_evt_garbage;
         } else {
            unsigned char num;

            num = UINT8_UNPACK(dev->ptr);

            if (len < (num * 4)) {
               return dev_evt_garbage;
            } else {
               return dev_evt_num_comp_pkts;
            }
         }
         break;

      case BTEVT_READ_FEATURES:
         DEBUG_STR("READ REMOTE SUPPORTED FEATURES");
         if (len < 11) {
            return dev_evt_garbage;
         } else {
            unsigned char status;

            status = UINT8_UNPACK(dev->ptr);

            if (status == HCI_SUCCESS) {
               return dev_evt_read_features_succ;
            } else {
               return dev_evt_read_features_unsucc;
            }
         }

      case BTEVT_MAX_SLOTS_CHG:
         DEBUG_STR("MAX SLOTS CHANGE EVENT");
         if (len < 3) {
            return dev_evt_garbage;
         } else {
            return dev_evt_max_slots_chg;
         }

      case BTEVT_PINCODE_REQ:
         if (len < 6) {
            return dev_evt_garbage;
         } else {
            return dev_evt_pincode_req;
         }

      case BTEVT_LINK_KEY_REQ:
         if (len < 6) {
            return dev_evt_garbage;
         } else {
            return dev_evt_link_key_req;
         }

      case BTEVT_LINK_KEY_NOT:
         if (len < 33) {
            return dev_evt_garbage;
         } else {
            return dev_evt_link_key_not;
         }

      default:
         DEBUG_STR("UNKNOWN code");
         DEBUG_INT(code);
         return dev_evt_none;
   }
}

bt_dev_evt_e bt_hci_unpack_cmd_status(bt_dev_t *dev, int len) {
   unsigned char  status, ncmds;
   unsigned short opcode, ogf, ocf;

   status = UINT8_UNPACK(dev->ptr);
   ncmds  = UINT8_UNPACK(dev->ptr);
   opcode = UINT16_UNPACK(dev->ptr);

   if (opcode == 0) {
      return dev_evt_none;
   } else {
      ogf = CMD_OPCODE_OGF(opcode);
      ocf = CMD_OPCODE_OCF(opcode);
      DEBUG_STR("CMD STATUS OGF, OCF");
      DEBUG_INT(ogf);
      DEBUG_INT(ocf);
   }

   return dev_evt_none;
}

bt_dev_evt_e bt_hci_unpack_cmd_complete(bt_dev_t *dev, int len) {
   unsigned char  ncmds;
   unsigned short opcode, ogf, ocf;

   ncmds  = UINT8_UNPACK(dev->ptr);
   opcode = UINT16_UNPACK(dev->ptr);

   if (opcode == 0) {
      return dev_evt_none;
   } else {
      ogf = CMD_OPCODE_OGF(opcode);
      ocf = CMD_OPCODE_OCF(opcode);
   }

   switch (ogf) {
      case OGF_LINK_CTL:
         DEBUG_STR("OGF LINK CTL");
         switch (ocf) {
            case OCF_LINK_KEY_REPLY:
               {
                  unsigned char status;

                  if (len < 7) {
                     return dev_evt_garbage;
                  } else {
                     status = UINT8_UNPACK(dev->ptr);
                     if (status == HCI_SUCCESS) {
                        return dev_evt_link_key_reply_succ;
                     } else {
                        return dev_evt_link_key_reply_unsucc;
                     }
                  }
                  break;
               }

            case OCF_LINK_KEY_REPLY_NEG:
               {
                  unsigned char status;

                  if (len < 7) {
                     return dev_evt_garbage;
                  } else {
                     status = UINT8_UNPACK(dev->ptr);
                     if (status == HCI_SUCCESS) {
                        return dev_evt_link_key_reply_neg_succ;
                     } else {
                        return dev_evt_link_key_reply_neg_unsucc;
                     }
                  }
                  break;
               }

            case OCF_PINCODE_REPLY:
               {
                  unsigned char status;

                  if (len < 7) {
                     return dev_evt_garbage;
                  } else {
                     status = UINT8_UNPACK(dev->ptr);
                     if (status == HCI_SUCCESS) {
                        return dev_evt_pincode_reply_succ;
                     } else {
                        return dev_evt_pincode_reply_unsucc;
                     }
                  }
                  break;
               }

            case OCF_PINCODE_REPLY_NEG:
               {
                  unsigned char status;

                  if (len < 7) {
                     return dev_evt_garbage;
                  } else {
                     status = UINT8_UNPACK(dev->ptr);
                     if (status == HCI_SUCCESS) {
                        return dev_evt_pincode_reply_neg_succ;
                     } else {
                        return dev_evt_pincode_reply_neg_unsucc;
                     }
                  }
                  break;
               }
         }
         break;

      case OGF_INFO_PARAM:
         DEBUG_STR("OGF INFO PARAM");
         switch (ocf) {
            case OCF_READ_LOCAL_FEATURES:
               DEBUG_STR("READ LOCAL FEATURES");
               if (len < 9)
                  return dev_evt_garbage;
               else 
                  return bt_hci_unpack_cc_read_local_features(dev);

            case OCF_READ_BD_ADDR:
               DEBUG_STR("READ BD ADDR");
               if (len < 7)
                  return dev_evt_garbage;
               else 
                  return bt_hci_unpack_cc_read_bd_addr(dev);
               
            default:
               DEBUG_STR("UNKNOWN ocf");
               DEBUG_INT(ocf);
               return dev_evt_none;
         }

      case OGF_HOST_CTL:
         DEBUG_STR("OGF HOST CTL");
         switch (ocf) {
            case OCF_WRITE_SCAN_ENABLE:
               DEBUG_STR("WRITE SCAN ENABLE");
               if (len < 1) {
                  return dev_evt_garbage;
               } else {
                  unsigned char status = UINT8_UNPACK(dev->ptr);

                  if (status == HCI_SUCCESS)
                     return dev_evt_write_scan_enable_succ;
                  else 
                     return dev_evt_write_scan_enable_unsucc;
               }

            case OCF_CHANGE_LOCAL_NAME:
               DEBUG_STR("CHANGE LOCAL NAME");
               if (len < 1) {
                  return dev_evt_garbage;
               } else {
                  unsigned char status = UINT8_UNPACK(dev->ptr);

                  if (status == HCI_SUCCESS)
                     return dev_evt_change_name_succ;
                  else 
                     return dev_evt_change_name_unsucc;
               }

            case OCF_SET_EVENT_FLT:
               DEBUG_STR("SET EVENT FILTER");
               if (len < 1) {
                  return dev_evt_garbage;
               } else {
                  unsigned char status = UINT8_UNPACK(dev->ptr);

                  if (status == HCI_SUCCESS)
                     return dev_evt_set_evt_flt_succ;
                  else 
                     return dev_evt_set_evt_flt_unsucc;
               }

            case OCF_WRITE_AUTH_ENABLE:
               DEBUG_STR("WRITE AUTH ENABLE");
               if (len < 1) {
                  return dev_evt_garbage;
               } else {
                  unsigned char status = UINT8_UNPACK(dev->ptr);

                  if (status == HCI_SUCCESS)
                     return dev_evt_write_auth_enable_succ;
                  else 
                     return dev_evt_write_auth_enable_unsucc;
               }

            default:
               DEBUG_STR("UNKNOWN ocf");
               DEBUG_INT(ocf);
               return dev_evt_none;
         }

      default:
         DEBUG_STR("UNKNOWN ogf");
         DEBUG_INT(ogf);
         return dev_evt_none;
   }

   return dev_evt_none;
}

bt_dev_evt_e bt_hci_unpack_cc_read_local_features(bt_dev_t *dev) {
   unsigned char status, pkt_types[2];

   status = UINT8_UNPACK(dev->ptr);
   if (status == HCI_SUCCESS) {
      pkt_types[0] = UINT8_UNPACK(dev->ptr);
      pkt_types[1] = UINT8_UNPACK(dev->ptr);
      if (pkt_types[0] & LMP_3SLOT)
         dev->pkt_type |= (HCI_DM3 | HCI_DH3);
      if (pkt_types[0] & LMP_5SLOT)
         dev->pkt_type |= (HCI_DM5 | HCI_DH5);
      if (pkt_types[1] & LMP_HV2)
         dev->pkt_type |= (HCI_HV2);
      if (pkt_types[1] & LMP_HV3)
         dev->pkt_type |= (HCI_HV3);
      return dev_evt_read_local_feat_succ;
   } else {
      return dev_evt_read_local_feat_unsucc;
   }
}

bt_dev_evt_e bt_hci_unpack_cc_read_bd_addr(bt_dev_t *dev) {
   unsigned char status;

   status = UINT8_UNPACK(dev->ptr);

   if (status == HCI_SUCCESS) {
      return dev_evt_read_bd_addr_succ;
   } else {
      return dev_evt_read_bd_addr_unsucc;
   }
}

void bt_hci_unpack_inquiry_result(bt_dev_t *dev, bt_peer_t *peer) {
   memcpy(peer->bd_addr, dev->ptr, 6); dev->ptr += 6;
   peer->pscan_rep_mode = UINT8_UNPACK(dev->ptr);
   peer->pscan_per_mode = UINT8_UNPACK(dev->ptr);
   peer->pscan_mode     = UINT8_UNPACK(dev->ptr);
   memcpy(peer->cod, dev->ptr, 3); dev->ptr += 3;
   peer->clock_offset   = UINT16_UNPACK(dev->ptr);
}
