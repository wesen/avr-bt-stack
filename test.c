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

void unix_cb_ready(bt_dev_t *dev) {
   DEBUG_STR("Device ready");
   DEBUG_STR(dev->name);
   dev->state = BT_DEV_STATE_INQUIRY;
   bt_dev_pack_inquiry(dev, 5, NUM_PEERS);
   bt_dev_flush_hci(dev);
}

int unix_cb_peer_found(bt_dev_t *dev, bt_peer_t *peer) {
   if (!peer) {
      DEBUG_STR("No Peer found");
      dev->state = BT_DEV_STATE_INQUIRY;
      bt_dev_pack_inquiry(dev, 5, NUM_PEERS);
      bt_dev_flush_hci(dev);
      return 0;
   } else {
      DEBUG_STR("Peer found");
      io_bd_addr(peer->bd_addr);
      
      return 1; /* CONNECT */
   }
}

void unix_cb_conn_succ(bt_dev_t *dev, bt_acl_t *acl) {
   DEBUG_STR("Connection successful");
   io_bd_addr(acl->peer->bd_addr);
}

void unix_cb_conn_unsucc(bt_dev_t *dev, bt_peer_t *peer) {
   DEBUG_STR("Connection not successful");
   io_bd_addr(peer->bd_addr);
}

void unix_cb_disconn_succ(bt_dev_t *dev, bt_acl_t *acl, unsigned char reason) {
   DEBUG_STR("Disconnection successful");
   DEBUG_INT(reason);
   io_bd_addr(acl->peer->bd_addr);
}

unsigned char *unix_cb_pincode_req(bt_dev_t *dev, bt_peer_t *peer) {
   return pincode;
}

void unix_cb_conn_data(bt_dev_t *dev, bt_acl_t *data) {
   DEBUG_STR("Received data");
}

int main(int argc, char *argv[]) {
   bt_callbacks_t cb = {
      unix_cb_ready,
      unix_cb_peer_found,
      NULL,
      unix_cb_conn_succ,
      unix_cb_conn_unsucc,
      unix_cb_disconn_succ,
      unix_cb_conn_data,
      unix_cb_pincode_req,
      NULL,
      NULL
   };

   if (argc == 2) {
      strncpy(dev.tty, argv[1], 128);
      dev.tty[127] = '\0';
   }

   bt_dev_init(&dev);
   memset(dev.name, 0, 32);
   strncpy(dev.name, DEV_NAME, 31);

   if (!bt_hci_main(&dev, &cb)) {
      PERR_STR("ERROR in MAIN LOOP");
   }

   return EXIT_SUCCESS;
}
