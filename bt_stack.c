#include "bt_stack.h"

unsigned int uint16_unpack__(unsigned char **ptr) {
   int i = *((*ptr)++);
   return (i |= *((*ptr)++) << 8);
}

