#include <stdio.h>
#include <string.h>

#include "bt_stack.h"
#include "bt-mouse.h"

bt_dev_t     dev;
bt_peer_t    peers[NUM_PEERS];
unsigned int num_peers = 0;

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
   unsigned int finished = 0;

   bt_dev_pack_create_conn(dev, peer, 0);
   bt_dev_flush_hci(dev);

   while (!finished) {
      switch (bt_dev_read_hci(dev)) {
         case dev_evt_pincode_req:
            DEBUG_STR("PIN CODE request");
            break;

         case dev_evt_link_key_req:
            DEBUG_STR("LINK KEY request");
            break;

         case dev_evt_link_key_not:
            DEBUG_STR("LINK KEY notification");
            break;

         case dev_evt_link_key_reply_succ:
            DEBUG_STR("LINK KEY REPLY successful");
            break;

         case dev_evt_link_key_reply_unsucc:
            DEBUG_STR("LINK KEY REPLY unsuccessful");
            break;

         case dev_evt_link_key_reply_neg_succ:
            DEBUG_STR("LINK KEY REPLY NEG successful");
            break;

         case dev_evt_link_key_reply_neg_unsucc:
            DEBUG_STR("LINK KEY REPLY NEG unsuccessful");
            break;

         case dev_evt_pincode_reply_succ:
            DEBUG_STR("PINCODE REPLY successful");
            break;

         case dev_evt_pincode_reply_unsucc:
            DEBUG_STR("PINCODE REPLY unsuccessful");
            break;

         case dev_evt_pincode_reply_neg_succ:
            DEBUG_STR("PINCODE REPLY NEG successful");
            break;

         case dev_evt_pincode_reply_neg_unsucc:
            DEBUG_STR("PINCODE REPLY NEG unsuccessful");
            break;

         case dev_evt_conn_complete_succ:
            DEBUG_STR("Connection complete");
            return 1;
            break;

         case dev_evt_conn_complete_unsucc:
            DEBUG_STR("Connection incomplete");
            return 0;
            break;

         case dev_evt_disconn_complete_succ:
            DEBUG_STR("Disconnection complete");
            break;

         case dev_evt_disconn_complete_unsucc:
            DEBUG_STR("Disconnection incomplete");
            break;

         default:
            finished = 1;
            DEBUG_STR("not handled");
      }
   }

   return 0;
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

   while (bt_dev_read_hci(&dev)) 
      ;

   return EXIT_SUCCESS;
}
