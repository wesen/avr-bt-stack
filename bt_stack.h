#ifndef BT_STACK_H__
#define BT_STACK_H__

#define NUM_PEERS 3
#define NUM_ACLS 3

#include "bt_dev.h"
#include "bt_hci.h"
#include "bt_io.h"
#include "bt_serial.h"
#include "bt_l2cap.h"

#define UINT16_PACK(ptr, i) \
   { *(ptr++) = ((i) & 0xFF); \
     *(ptr++) = (((i) >> 8) & 0xFF); }

#define UINT8_PACK(ptr, i) \
     *(ptr++) = ((i) & 0xFF)

#define UINT16_UNPACK(ptr) uint16_unpack__(&ptr)

unsigned int uint16_unpack__(unsigned char **ptr);

#define UINT8_UNPACK(ptr) \
        (*(ptr++))

#endif /* BT_STACK_H__ */
