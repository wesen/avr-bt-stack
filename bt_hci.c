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

bt_dev_evt_e bt_hci_unpack_hci(bt_dev_t *dev, bt_callbacks_t *cb) {
   hci_type_e type;

   dev->ptr = dev->buf;
   type = UINT8_UNPACK(dev->ptr);

   switch (type) {
      case hci_evt_pkt:
         DEBUG_STR("HCI EVT");
         return bt_hci_unpack_evt(dev, cb);

      case hci_acl_pkt:
         DEBUG_STR("HCI ACL");
         return bt_hci_unpack_acl(dev, cb);

      case hci_sco_pkt:
         DEBUG_STR("HCI SCO");
         return bt_hci_unpack_sco(dev, cb);
         
      default:
         return dev_evt_garbage;
   }
}

bt_dev_evt_e bt_hci_unpack_acl(bt_dev_t *dev, bt_callbacks_t *cb) {
   return dev_evt_acl;
}

bt_dev_evt_e bt_hci_unpack_sco(bt_dev_t *dev, bt_callbacks_t *cb) {
   return dev_evt_sco;
}

bt_dev_evt_e bt_hci_unpack_evt(bt_dev_t *dev, bt_callbacks_t *cb) {
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
         return bt_hci_unpack_inquiry_complete(dev, len);

      case BTEVT_INQUIRY_RESULT:
         DEBUG_STR("INQUIRY RESULT");
         return bt_hci_unpack_inquiry_results(dev, len);

      case BTEVT_CONN_REQUEST:
         DEBUG_STR("CONN REQUEST");
         return bt_hci_unpack_conn_request(dev, len, cb);

      case BTEVT_CONN_COMPLETE:
         DEBUG_STR("CONN COMPLETE");
         return bt_hci_unpack_conn_complete(dev, len, cb);

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
         return bt_hci_unpack_disconn_complete(dev, len, cb);

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
         return bt_hci_unpack_read_features(dev, len);


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
            unsigned char bd_addr[6], *pincode;
            bt_peer_t *peer;

            memcpy(bd_addr, dev->ptr, 6); dev->ptr += 6;
            if ((peer = bt_dev_get_peer_by_bd_addr(dev, bd_addr)) &&
                (cb->cb_pincode_req)) {
               if (!(pincode = cb->cb_pincode_req(dev, peer))) {
                  bt_dev_pack_pincode_reply_neg(dev, peer->bd_addr);
               } else {
                  bt_dev_pack_pincode_reply(dev, peer->bd_addr, 
                                             16, pincode);
               }
            } else {
               bt_dev_pack_pincode_reply_neg(dev, bd_addr);
            }

            bt_dev_flush_hci(dev);

            return dev_evt_pincode_req;
         }

      case BTEVT_LINK_KEY_REQ:
         if (len < 6) {
            return dev_evt_garbage;
         } else {
            unsigned char bd_addr[6], *link_key;
            bt_peer_t *peer;

            memcpy(bd_addr, dev->ptr, 6); dev->ptr += 6;
            if ((peer = bt_dev_get_peer_by_bd_addr(dev, bd_addr)) &&
                (cb->cb_link_key_req)) {
               if (!(link_key = cb->cb_link_key_req(dev, peer))) {
                  bt_dev_pack_link_key_reply_neg(dev, peer->bd_addr);
               } else {
                  bt_dev_pack_link_key_reply(dev, peer->bd_addr, 
                                             link_key);
               }
            } else {
               bt_dev_pack_link_key_reply_neg(dev, bd_addr);
            }

            bt_dev_flush_hci(dev);

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

bt_dev_evt_e bt_hci_unpack_inquiry_complete(bt_dev_t *dev, int len) {
   if (len < 1) {
      return dev_evt_garbage;
   } else {
      unsigned char status = UINT8_UNPACK(dev->ptr);
      
      if (status == HCI_SUCCESS) {
         return dev_evt_inquiry_complete_succ;
      } else {
         return dev_evt_inquiry_complete_unsucc;
      }
   }
}

bt_dev_evt_e bt_hci_unpack_inquiry_results(bt_dev_t *dev, int len) {
   if (len < 1) {
      return dev_evt_garbage;
   } else {
      unsigned char num;
      
      num = UINT8_UNPACK(dev->ptr);
      if (len < (num * INQUIRY_RESULT_SIZE) + 1) {
         return dev_evt_garbage;
      } else {
         unsigned int i;
         bt_peer_t *peer;
         
         for (i=0; (i<num) && (peer = bt_dev_get_free_peer(dev)); i++) {
            bt_hci_unpack_inquiry_result(dev, peer);
         }
         return dev_evt_inquiry_results;
      }
   }
}

bt_dev_evt_e bt_hci_unpack_conn_request(bt_dev_t *dev, int len,
                                        bt_callbacks_t *cb) {
   if (len < 10) {
      return dev_evt_garbage;
   } else {
      bt_peer_t *peer;
      unsigned char bd_addr[6];
      
      memcpy(bd_addr, dev->ptr, 6);
      dev->ptr += 6;
      
      if (!(peer = bt_dev_get_peer_by_bd_addr(dev, bd_addr)))
         peer = bt_dev_get_free_peer(dev);
      
      if (peer && (cb->cb_conn_req != NULL)) {
         unsigned char link_type;
         
         memcpy(peer->cod, dev->ptr, 3);
         dev->ptr += 3;
         link_type = UINT8_UNPACK(dev->ptr);
         if (cb->cb_conn_req(dev, peer, link_type)) {
            bt_dev_pack_accept_conn(dev, peer->bd_addr, 1);
            peer->state = PEER_STATE_CONNECTING;
            dev->state = BT_DEV_STATE_CONNECTING;
         } else {
            bt_dev_pack_reject_conn(dev, peer->bd_addr, HCI_REJECT_PERSONAL);
         }
      } else {
         unsigned char bd_addr[6];
         
         memcpy(bd_addr, dev->ptr, 6); dev->ptr += 6;
         bt_dev_pack_reject_conn(dev, bd_addr, HCI_REJECT_PERSONAL);
      }
      bt_dev_flush_hci(dev);
      return dev_evt_conn_request;
   }
}

bt_dev_evt_e bt_hci_unpack_disconn_complete(bt_dev_t *dev, int len,
                                            bt_callbacks_t *cb) {
   if (len < 4) {
      return dev_evt_garbage;
   } else {
      unsigned char status;
      unsigned short handle;
      unsigned char reason;
      bt_acl_t *acl;

      status = UINT8_UNPACK(dev->ptr);
      handle = UINT16_UNPACK(dev->ptr);
      reason = UINT8_UNPACK(dev->ptr);
      
      acl = bt_dev_get_acl_by_handle(dev, handle);
      
      if (status == HCI_SUCCESS) {
         if (acl && cb->cb_disconn_succ)
            cb->cb_disconn_succ(dev, acl, reason);
         acl->state = ACL_STATE_EMPTY;
         dev->state = BT_DEV_STATE_READY;
         return dev_evt_disconn_complete_succ;
      } else {
         bt_dev_pack_disconn(dev, handle, HCI_DISCONNECT_USER);
         bt_dev_flush_hci(dev);
         dev->state = BT_DEV_STATE_DISCONNECTING;
         return dev_evt_disconn_complete_unsucc;
      }
   }
}

bt_dev_evt_e bt_hci_unpack_read_features(bt_dev_t *dev, int len) {
   if (len < 11) {
      return dev_evt_garbage;
   } else {
      unsigned char  status;
      
      status = UINT8_UNPACK(dev->ptr);
      
      if (status == HCI_SUCCESS) {
         unsigned short handle;
         unsigned char pkt_types[2];
         bt_peer_t *peer;
         bt_acl_t  *acl;
         
         handle = UINT16_UNPACK(dev->ptr);
         if (!(acl = bt_dev_get_acl_by_handle(dev, handle)))
            return dev_evt_read_features_unsucc;
         peer = acl->peer;
         
         pkt_types[0] = UINT8_UNPACK(dev->ptr);
         pkt_types[1] = UINT8_UNPACK(dev->ptr);
         if (pkt_types[0] & LMP_3SLOT)
            peer->pkt_type |= (HCI_DM3 | HCI_DH3);
         if (pkt_types[0] & LMP_5SLOT)
            peer->pkt_type |= (HCI_DM5 | HCI_DH5);
         if (pkt_types[1] & LMP_HV2)
            peer->pkt_type |= (HCI_HV2);
         if (pkt_types[1] & LMP_HV3)
            peer->pkt_type |= (HCI_HV3);
         
         return dev_evt_read_features_succ;
      } else {
         return dev_evt_read_features_unsucc;
      }
   }
}

bt_dev_evt_e bt_hci_unpack_conn_complete(bt_dev_t *dev, int len,
                                         bt_callbacks_t *cb) {
      if (len < 11) {
      return dev_evt_garbage;
   } else {
      unsigned char status, bd_addr[6], link_type, encrypt_mode;
      unsigned short handle;
      bt_peer_t      *peer;
      
      status = UINT8_UNPACK(dev->ptr);
      handle = UINT16_UNPACK(dev->ptr);
      memcpy(bd_addr, dev->ptr, 6); dev->ptr += 6;
      link_type = UINT8_UNPACK(dev->ptr);
      encrypt_mode = UINT8_UNPACK(dev->ptr);

      if (!(peer = bt_dev_get_peer_by_bd_addr(dev, bd_addr)) ||
            (link_type != 1)) {
         bt_dev_pack_disconn(dev, handle, HCI_DISCONNECT_USER);
         bt_dev_flush_hci(dev);
         dev->state = BT_DEV_STATE_DISCONNECTING;
         return dev_evt_conn_complete_unsucc;
      }
      
      if (status == HCI_SUCCESS) {
         bt_acl_t *acl = bt_dev_get_free_acl(dev);
         
         if (!acl) {
            bt_dev_pack_disconn(dev, handle, HCI_DISCONNECT_USER);
            bt_dev_flush_hci(dev);
            
            if (cb->cb_conn_unsucc != NULL)
               cb->cb_conn_unsucc(dev, peer);
            
            return dev_evt_conn_complete_unsucc;
         }
         
         acl->peer = peer;
         acl->handle = handle;
         acl->state = ACL_STATE_CONNECTED;
         acl->encrypt_mode = encrypt_mode;
         
         if (cb->cb_conn_succ != NULL)
            cb->cb_conn_succ(dev, acl);

         dev->state = BT_DEV_STATE_READY;
         
         return dev_evt_conn_complete_succ;
      } else {
         if (cb->cb_conn_unsucc != NULL)
            cb->cb_conn_unsucc(dev, peer);
         
         dev->state = BT_DEV_STATE_READY;
         return dev_evt_conn_complete_unsucc;
      }
   }
}

bt_dev_evt_e bt_hci_unpack_cmd_status(bt_dev_t *dev, int len) {
   unsigned char  status;
   unsigned short opcode, ogf, ocf;

   status = UINT8_UNPACK(dev->ptr);
   dev->ncmds  = UINT8_UNPACK(dev->ptr);
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
   unsigned short opcode, ogf, ocf;

   dev->ncmds  = UINT8_UNPACK(dev->ptr);
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

                  if (status == HCI_SUCCESS) {
                     if (dev->state == BT_DEV_STATE_INITIALIZING)
                     return dev_evt_write_scan_enable_succ;
                  } else {
                     return dev_evt_write_scan_enable_unsucc;
                  }
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

      case OGF_VENDOR:
         DEBUG_STR("OGF VENDOR");
         switch (ocf) {
#ifdef BTERICSSON
            case OCF_ERICSSON_SET_BAUDRATE:
               if (len < 1) {
                  return dev_evt_garbage;
               } else {
                  unsigned char status = UINT8_UNPACK(dev->ptr);

                  if (status == HCI_SUCCESS)
                     return dev_evt_ericsson_set_baudrate_succ;
                  else 
                     return dev_evt_ericsson_set_baudrate_unsucc;
               }
#endif /* BTERICSSON */
            default:
               DEBUG_STR("UNKNOWN ocf");
               return dev_evt_none;
         }
         break;

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

int bt_hci_main(bt_dev_t *dev, bt_callbacks_t *cb) {
   unsigned int finished = 0;

   while (!finished) {
      if (dev->ncmds > 0) {
         switch (dev->state) {
            case BT_DEV_STATE_RESET:
               dev->state = BT_DEV_STATE_INITIALIZING;
               dev->ncmds = 1;
               bt_dev_pack_clear_evt_flt(dev);
               bt_dev_flush_hci(dev);
               break;

            case BT_DEV_STATE_INITIALIZING:
               break;

            case BT_DEV_STATE_INITIALIZED:
               break;

            case BT_DEV_STATE_READY:
               {
                  unsigned int i;

                  if (cb->cb_idle)
                     cb->cb_idle(dev);

                  for (i=0; i<NUM_PEERS; i++) {
                     if (dev->peers[i].state == PEER_STATE_SHOULD_CONNECT) {
                        bt_dev_pack_create_conn(dev, dev->peers + i, 0);
                        bt_dev_flush_hci(dev);
                        break;
                     }
                  }
                  break;
               }

            case BT_DEV_STATE_INQUIRY:
               break;

            case BT_DEV_STATE_CONNECTING:
               break;

            default:
               break;
         }
      }

      switch (bt_dev_read_hci(dev, cb)) {
         case dev_evt_set_evt_flt_succ:
            if (dev->state == BT_DEV_STATE_INITIALIZING) {
               bt_dev_pack_change_local_name(dev, dev->name);
               bt_dev_flush_hci(dev);
            }
            break;

         case dev_evt_change_name_succ:
            if (dev->state == BT_DEV_STATE_INITIALIZING) {
               bt_dev_pack_write_auth_enable(dev);
               bt_dev_flush_hci(dev);
            }
            break;

         case dev_evt_write_auth_enable_succ:
            if (dev->state == BT_DEV_STATE_INITIALIZING) {
               bt_dev_pack_inq_scan_enable(dev);
               bt_dev_flush_hci(dev);
            }
            break;

         case dev_evt_write_scan_enable_succ:
            if (dev->state == BT_DEV_STATE_INITIALIZING) {
               dev->state = BT_DEV_STATE_READY;
               if (cb->cb_ready)
                  cb->cb_ready(dev);
            }
            break;

         case dev_evt_set_evt_flt_unsucc:
         case dev_evt_change_name_unsucc:
         case dev_evt_write_auth_enable_unsucc:
         case dev_evt_write_scan_enable_unsucc:
            if (dev->state == BT_DEV_STATE_INITIALIZING) {
               DEBUG_STR("Could not initialize device");
               return 0;
            }
            break;
         
         case dev_evt_inquiry_complete_succ:
            if (dev->state == BT_DEV_STATE_INQUIRY) {
               unsigned int i, num = 0;
               
               dev->state = BT_DEV_STATE_READY;
               if (cb->cb_peer_found != NULL) {
                  for (i=0; i<NUM_PEERS; i++) {
                     if (dev->peers[i].state == PEER_STATE_FOUND) {
                        if (cb->cb_peer_found(dev, dev->peers + i)) {
                           dev->peers[i].state = PEER_STATE_SHOULD_CONNECT;
                           num++;
                        } else {
                           dev->peers[i].state = PEER_STATE_EMPTY;
                        }
                     }
                  }

                  if (num == 0)
                     cb->cb_peer_found(dev, NULL);
               }
            }
            break;

         case dev_evt_inquiry_complete_unsucc:
            if (dev->state == BT_DEV_STATE_INQUIRY) {
               if (cb->cb_peer_found != NULL)
                  cb->cb_peer_found(dev, NULL);
               dev->state = BT_DEV_STATE_READY;
            }
            break;

         case dev_evt_timeout:
         case dev_evt_garbage:
         default:
            break;
      }
   }

   return 1;
}
