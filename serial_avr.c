#include "bt_stack.h"
#include "../src/avr-env/libavr.h"

int bt_serial_init(bt_dev_t *dev) {
   /* wait for module to reset itself */
   DEBUG_STR("waiting for module reset");
   uart_receive(dev->buf, 256);
   DEBUG_STR("module resetted");

   return 1;
}

int bt_serial_close(bt_dev_t *dev) {
   return 1;
}

int bt_serial_send(bt_dev_t *dev, unsigned char *buf, int len) {
   uart_send(buf, len);
   return len;
}

int bt_serial_read(bt_dev_t *dev, unsigned char *buf, int len) {
   return uart_receive(buf, len);
}
