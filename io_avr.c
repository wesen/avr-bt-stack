#include <string.h>
#include "bt_stack.h"
#include "../src/avr-env/libavr.h"

void io_str(char *s) {
   unsigned char len = strlen(s);
   unsigned char buf[3] = {0xde, 0xad, 0};

   buf[2] = len;

   uart_send(buf, 3);
   uart_send((unsigned char *)s, len);

   return;
}

void io_int(int d) {
   char buf[30];

   itoa(d, buf, 10);
   io_str(buf);

   return;
}

void error_str(char *s) {
   io_str(s);
}

void io_bd_addr(unsigned char *bd_addr) {
   return;
}
