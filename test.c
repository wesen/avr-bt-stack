#include <stdio.h>
#include <string.h>

#include "bt_stack.h"
#include "bt-mouse.h"

bt_dev_t     dev;
bt_peer_t    peers[NUM_PEERS];
bt_acl_t     acl;

unsigned char pincode[16] = {
   0xde, 0xad, 0xc0, 0xde, 0xf0, 0xa8, 0xbe, 0xef,
   0xde, 0xad, 0xc0, 0xde, 0xf0, 0xa8, 0xba, 0xbe };
unsigned int num_peers = 0;

int unix_handle_connection(bt_dev_t *dev, bt_peer_t *peer);

int unix_init_dev(bt_dev_t *dev) {
   /* initialize bluetooth device */
   if (!bt_serial_init(dev)) {
      PERR_STR("Could not initialise serial interface");
      return EXIT_FAILURE;
   }

   /* clear all event filters */
   DEBUG_STR("Resetting event filters");
   bt_dev_pack_clear_evt_flt(dev);
   bt_dev_flush_hci(dev);
   
   if (bt_dev_read_hci(dev) != dev_evt_set_evt_flt_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   /* set the local name */
   DEBUG_STR("setting local name to "DEV_NAME);

   bt_dev_pack_change_local_name(dev, DEV_NAME);
   bt_dev_flush_hci(dev);
   
   if (bt_dev_read_hci(dev) != dev_evt_change_name_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   /* enable authentication */
   DEBUG_STR("enabling authentication");
   bt_dev_pack_write_auth_enable(dev);
   bt_dev_flush_hci(dev);
   
   if (bt_dev_read_hci(dev) != dev_evt_write_auth_enable_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   /* enable paging and inquiry scans */
   DEBUG_STR("enabling inquiry and page scans");
   bt_dev_pack_inq_scan_enable(dev);
   bt_dev_flush_hci(dev);
   
   if (bt_dev_read_hci(dev) != dev_evt_write_scan_enable_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   return 1;
}

int unix_inquiry_dev(bt_dev_t *dev) {
   unsigned int finished = 0;

   num_peers = 0;

   /* start inquiry */
   DEBUG_STR("starting inquiry");
   bt_dev_pack_inquiry(dev, 20, NUM_PEERS);
   bt_dev_flush_hci(dev);
   
   while (!finished) {
      switch (bt_dev_read_hci(dev)) {
         case dev_evt_inquiry_results: 
            {
               unsigned char num = *(dev->ptr - 1);
               unsigned int i;

               for (i = 0; 
                   (i < num) && (num_peers < NUM_PEERS);
                   i++) {
                  memcpy(peers[num_peers].bd_addr, dev->ptr, 6);
                  dev->ptr += 6;
                  peers[num_peers].pscan_rep_mode = UINT8_UNPACK(dev->ptr);
                  peers[num_peers].pscan_per_mode = UINT8_UNPACK(dev->ptr);
                  peers[num_peers].pscan_mode = UINT8_UNPACK(dev->ptr);
                  memcpy(peers[num_peers].cod, dev->ptr, 3);
                  dev->ptr += 3;
                  peers[num_peers].clock_offset = UINT16_UNPACK(dev->ptr);
                  num_peers++;
               }

               break;
            }

         case dev_evt_inquiry_complete_succ:
            finished = 1;
            break;

         case dev_evt_inquiry_complete_unsucc:
            num_peers = 0;
            finished = 1;
            break;

         default:
            break;
      }
   }

   return num_peers;
}

int unix_connect_acl(bt_dev_t *dev, bt_peer_t *peer) {
   bt_dev_pack_create_conn(dev, peer, 0);
   bt_dev_flush_hci(dev);

   return unix_handle_connection(dev, peer);
}

int unix_wait_for_connection(bt_dev_t *dev) {
   unsigned int finished = 0;

   while (!finished) {
      switch (bt_dev_read_hci(dev)) {
         case dev_evt_conn_request:
            {
               unsigned char link_type;
               
               memcpy(peers[0].bd_addr, dev->ptr, 6); dev->ptr += 6;
               memcpy(peers[0].cod, dev->ptr, 3); dev->ptr += 3;
               link_type = UINT8_UNPACK(dev->ptr);

               if (link_type == 1) {
                  bt_dev_pack_accept_conn(dev, peers[0].bd_addr, 1);
                  bt_dev_flush_hci(dev);

                  if (unix_handle_connection(dev, peers))
                     finished = 1;
               } else {
                  bt_dev_pack_reject_conn(dev, peers[0].bd_addr,
                                          HCI_REJECT_PERSONAL);
                  bt_dev_flush_hci(dev);
               }

               break;
            }

         case dev_evt_timeout:
         case dev_evt_garbage:
            break;

         default:
            DEBUG_STR("Not handled");
            break;
      }
   }

   return 1;
}

int unix_handle_connection(bt_dev_t *dev, bt_peer_t *peer) {
   unsigned int finished = 0;

   while (!finished) {
      switch (bt_dev_read_hci(dev)) {
         case dev_evt_conn_complete_succ:
            DEBUG_STR("Connection successful");
            return 1;

         case dev_evt_conn_complete_unsucc:
            DEBUG_STR("Connection not successful");
            return 0;

         case dev_evt_pincode_req:
            DEBUG_STR("PIN CODE request");
            if (!memcmp(dev->ptr, peers[0].bd_addr, 6)) {
               bt_dev_pack_pincode_reply(dev, peers[0].bd_addr, 
                                              16, pincode);
               bt_dev_flush_hci(dev);
            }
            break;

         case dev_evt_link_key_req:
            DEBUG_STR("LINK KEY request");
            if (!memcmp(dev->ptr, peers[0].bd_addr, 6)) {
               bt_dev_pack_link_key_reply_neg(dev, peers[0].bd_addr);
               bt_dev_flush_hci(dev);
            }
            break;

         case dev_evt_link_key_not:
            DEBUG_STR("LINK KEY notification");
            break;

         case dev_evt_link_key_reply_succ:
            DEBUG_STR("LINK KEY REPLY successful");
            break;

         case dev_evt_link_key_reply_unsucc:
            DEBUG_STR("LINK KEY REPLY unsuccessful");
            return 0;

         case dev_evt_link_key_reply_neg_succ:
            DEBUG_STR("LINK KEY REPLY NEG successful");
            break;

         case dev_evt_link_key_reply_neg_unsucc:
            DEBUG_STR("LINK KEY REPLY NEG unsuccessful");
            return 0;

         case dev_evt_pincode_reply_succ:
            DEBUG_STR("PINCODE REPLY successful");
            break;

         case dev_evt_pincode_reply_unsucc:
            DEBUG_STR("PINCODE REPLY unsuccessful");
            return 0;

         case dev_evt_pincode_reply_neg_succ:
            DEBUG_STR("PINCODE REPLY NEG successful");
            break;

         case dev_evt_pincode_reply_neg_unsucc:
            DEBUG_STR("PINCODE REPLY NEG unsuccessful");
            return 0;

         case dev_evt_none:
         case dev_evt_garbage:
            break;

         default:
            DEBUG_STR("not handled");
            break;
      }
   }

   return 1;
}

int unix_handle_acl(bt_dev_t *dev, bt_acl_t *acl) {
   unsigned short hf, handle, pb, bc, len;

   hf = UINT16_UNPACK(dev->ptr);
   len = UINT16_UNPACK(dev->ptr);
   handle = ACL_HANDLE(hf);
   pb = ACL_FLAGS_PB(hf);
   bc = ACL_FLAGS_BC(hf);

   if (hf == acl->handle) {
      switch (pb) {
         case HCI_ACL_CONT:
            DEBUG_STR("L2CAP Continuation");
            break;

         case HCI_ACL_START:
            DEBUG_STR("L2CAP start packet");
            switch (bt_l2cap_unpack(dev)) {
               case l2cap_evt_garbage:
                  DEBUG_STR("L2CAP garbage");
                  break;

               case l2cap_evt_connless_data:
                  DEBUG_STR("Connection less data");
                  break;

               case l2cap_evt_conn_data:
                  DEBUG_STR("Connection data");
                  break;
                  
               case l2cap_evt_echo_req:
                  DEBUG_STR("L2CAP echo request");
                  break;

               case l2cap_evt_echo_rsp:
                  DEBUG_STR("L2CAP echo response");
                  break;

               case l2cap_evt_conf_req:
                  DEBUG_STR("L2CAP configuration request");
                  break;

               case l2cap_evt_conf_rsp:
                  DEBUG_STR("L2CAP configuration response");
                  break;

               case l2cap_evt_conn_req:
                  DEBUG_STR("L2CAP connection request");
                  break;

               case l2cap_evt_conn_rsp:
                  DEBUG_STR("L2CAP connection response");
                  break;

               default:
                  DEBUG_STR("Not handled");
                  break;
            }
            break;

         default:
            DEBUG_STR("LMP not handled");
            break;
      }
   }

   return 1;
}

int unix_main_loop(bt_dev_t *dev, bt_acl_t *acl) {
   unsigned int finished = 0;

   while (!finished) {
      switch (bt_dev_read_hci(dev)) {
         case dev_evt_acl:
            unix_handle_acl(dev, acl);
            break;

         case dev_evt_disconn_complete_succ:
            break;

         case dev_evt_disconn_complete_unsucc:
            break;

         case dev_evt_conn_request:
            {
               unsigned char bd_addr[6];
               
               memcpy(bd_addr, dev->ptr, 6); dev->ptr += 6;

               bt_dev_pack_reject_conn(dev, bd_addr,
                                       HCI_REJECT_PERSONAL);
               bt_dev_flush_hci(dev);

               break;
            }

         case dev_evt_timeout:
         case dev_evt_garbage:
            break;

         default:
            DEBUG_STR("Not handled");
            break;
      }
   }

   return 1;
}

int main(int argc, char *argv[]) {
   if (argc == 2) {
      strncpy(dev.tty, argv[1], 128);
      dev.tty[127] = '\0';
   }

   if (!unix_init_dev(&dev)) {
      PERR_STR("Could not initialize device");
      return EXIT_FAILURE;
   }

   while (!unix_wait_for_connection(&dev))
      ;

   acl.peer = peers;
   acl.handle = UINT8_UNPACK(dev.ptr);
   memcpy(acl.peer->bd_addr, dev.ptr, 6); dev.ptr += 6;
   acl.encrypt_mode = UINT8_UNPACK(dev.ptr);

   unix_main_loop(&dev, &acl);

   return EXIT_SUCCESS;
}
