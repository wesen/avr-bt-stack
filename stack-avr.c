#include <string.h>
#include <eeprom.h>

#include "bt_stack.h"
#include "stack-avr.h"
#include "../src/avr-env/libavr.h"

bt_dev_t       dev;
bt_peer_t      peers[NUM_PEERS];
bt_l2cap_ch_t  l2cap_chs[NUM_L2CAP_CHS];
bt_l2cap_ch_t  *sensor_ch = NULL;
int            num_peers, num_l2caps;

bt_dev_evt_e avr_read_hci(bt_dev_t *dev) {
   bt_dev_evt_e  ret;
   unsigned char finished = 0;

   while (!finished) {
      ret = bt_dev_read_hci(dev);
      switch (ret) {
         case dev_evt_timeout:
         case dev_evt_garbage:
            break;

         default:
            finished = 1;
            break;
      }
   }

   return ret;
}

int avr_read_eeprom(void) {
   unsigned int  addr = 0, finished = 0;
   unsigned char type;

   while (!finished) {
      type = eeprom_rb(addr++);

      switch (type) {
         case AVR_END:
            finished = 1;
            break;

         default:
            DEBUG_STR("EEPROM garbage");
            finished = 1;
            break;
      }
   }

   return 1;
}

int avr_init_dev(bt_dev_t *dev) {
   int i;

   /* reset L2CAP connections */
   sensor_ch = NULL;
   for (i = 0; i < NUM_L2CAP_CHS; i++)
      l2cap_chs[i].state = state_closed;
   num_l2caps = 0;

   /* reset Bluetooth peer devices */
   num_peers = 0;

   if (!bt_serial_init(dev)) {
      PERR_STR("Error serial");
      return 0;
   }

   /* clear all event filters */
   DEBUG_STR("Resetting event filters");
   bt_dev_pack_clear_evt_flt(dev);
   bt_dev_flush_hci(dev);
   
   if (avr_read_hci(dev) != dev_evt_set_evt_flt_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   /* set the local name */
   DEBUG_STR("setting local name to "DEV_NAME);

   bt_dev_pack_change_local_name(dev, DEV_NAME);
   bt_dev_flush_hci(dev);
   
   if (avr_read_hci(dev) != dev_evt_change_name_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   /* enable authentication */
   DEBUG_STR("enabling authentication");
   bt_dev_pack_write_auth_enable(dev);
   bt_dev_flush_hci(dev);
   
   if (avr_read_hci(dev) != dev_evt_write_scan_enable_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   /* enable paging and inquiry scans */
   DEBUG_STR("enabling inquiry and page scans");
   bt_dev_pack_inq_scan_enable(dev);
   bt_dev_flush_hci(dev);
   
   if (avr_read_hci(dev) != dev_evt_write_scan_enable_succ) {
      DEBUG_STR("Error");
      return 0;
   }

   return 1;
}

int avr_inquiry_dev(bt_dev_t *dev) {
   unsigned int finished = 0;

   num_peers = 0;

   /* start inquiry */
   DEBUG_STR("starting inquiry");
   bt_dev_pack_inquiry(dev, 20, NUM_PEERS);
   bt_dev_flush_hci(dev);
   
   while (!finished) {
      switch (avr_read_hci(dev)) {
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
            finished = 1;
            num_peers = 0;
            break;

         default:
            break;
      }
   }

   return num_peers;
}

int avr_main_handle_uart(bt_dev_t *dev) {
   switch (bt_dev_read_hci(dev)) {
      case dev_evt_conn_request: 
         {
            unsigned char bd_addr[6], cod[3], link_type;
            
            DEBUG_STR("Connection request");
            memcpy(bd_addr, dev->ptr, 6);
            dev->ptr += 6;
            memcpy(cod, dev->ptr, 3);
            dev->ptr += 3;
            
            link_type = UINT8_UNPACK(dev->ptr);
            switch (link_type) {
               case 1:
                  /* ACL, accept */
                  if (handle == -1) {
                     DEBUG_STR("Accepting connection request");
                     bt_dev_pack_accept_conn(dev, bd_addr, 1);
                  } else {
                     DEBUG_STR("Rejecting connection request");
                     bt_dev_pack_reject_conn(dev, 
                           bd_addr,
                           HCI_REJECT_PERSONAL);
                  }
                  break;
                  
               default:
                  /* reject */
                  DEBUG_STR("Rejecting connection request");
                  bt_dev_pack_reject_conn(dev, 
                        bd_addr,
                        HCI_REJECT_PERSONAL);
                  break;
            }
            bt_dev_flush_hci(dev);
            break;
         }
         
      case dev_evt_conn_complete_succ:
         {
            unsigned short tmp_handle;
            unsigned char  bd_addr[6], link, encrypt;
            
            DEBUG_STR("Connection completion successful!");
            tmp_handle = UINT16_UNPACK(dev->ptr) & 0xFFF;
            memcpy(bd_addr, dev->ptr, 6); dev->ptr += 6;
            link = UINT8_UNPACK(dev->ptr);
            encrypt = UINT8_UNPACK(dev->ptr);
            
            if ((link == 1) && (handle == -1)) {
               DEBUG_STR("Acception connection");
               handle = tmp_handle;
            } else {
               DEBUG_STR("Already connected or SCO connection");
               DEBUG_INT(handle);
               bt_dev_pack_disconn(dev, 
                     tmp_handle,
                     HCI_DISCONNECT_USER);
               bt_dev_flush_hci(dev);
            }
            
            break;
         }
         
      case dev_evt_conn_complete_unsucc:
         {
            unsigned short tmp_handle;
            
            DEBUG_STR("Connection completion unsuccesful!");
            tmp_handle = UINT8_UNPACK(dev->ptr) & 0xFFF;
            
            if (tmp_handle == handle)
               handle = -1;
            break;
         }
         
      case dev_evt_disconn_complete_unsucc:
         DEBUG_STR("Close connection unsuccessful");
         break;
         
      case dev_evt_disconn_complete_succ:
         {
            unsigned short tmp_handle;
            unsigned char  reason;
            
            DEBUG_STR("Close connection");
            tmp_handle = UINT16_UNPACK(dev->ptr) & 0xfff;
            reason     = UINT8_UNPACK(dev->ptr);
                  
            if (tmp_handle == handle) {
               DEBUG_STR("Closing our connection");
               handle = -1;
            }
            break;
         }
         break;

      case dev_evt_acl:
         avr_handle_l2cap(dev);
         break;
         
      case dev_evt_timeout:
#if 0
         if (handle != -1) {
            unsigned char buf[] = "hello world!\n";
            bt_dev_pack_l2cap(&dev, handle, buf, strlen((char *)buf));
            bt_dev_flush_hci(&dev);
         }
#endif
         break;
         
      default:
         break;
   }

   return 1;
}

void avr_main_loop(bt_dev_t *dev) {
   unsigned int finished = 0;

   while (!finished) {
      if (uart_pending()) {
         if (!avr_main_handle_uart(dev))
            finished = 1;
      } else if (sensor_ch != NULL) {
         /* send sensor values */
         bt_hci_pack_acl(dev,           sensor_ch->handle, 
                         HCI_ACL_START, HCI_ACL_PP, 12+4);
         bt_l2cap_pack_hdr(dev, 12, sensor_ch->dst_cid);

         store_values(dev->ptr); dev->ptr += 12;
         bt_dev_flush_hci(dev);
      }
   }
}

int main(void) {
   unsigned short handle = -1;
   int            reset = 0;

   if (!avr_read_eeprom()) {
      PERR_STR("Could not read stored eeprom values");
   }

   uart_init();
   init_sensor();
   led_init();
   sei();

   while (1) {
      while (!avr_init_dev(&dev)) {
         bt_dev_pack_reset(&dev);
         bt_dev_flush_hci(&dev);
      }

      avr_main_loop(&dev);
   }

   return 0;
}

