#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "bt_stack.h"

/**
 * @file io_stdio.c
 *
 * I/O routines (debug, error) 
 */

/** ANSI C implementation of #io_str() */
void io_str(char *s) {
   printf("DEBUG: %s\n", s);
}

/** ANSI C implementation of #io_int() */
void io_int(int d) {
   printf("DEBUG: 0x%x\n", d);
}

/** ANSI C implementation of #error_str() */
void error_str(char *s) {
   fprintf(stderr, "ERROR: %s\n", s);
}

/** ANSI C implementation of #error_str() */
void io_bd_addr(unsigned char *bd_addr) {
   printf("%x:%x:%x:%x:%x:%x\n",
         bd_addr[0], bd_addr[1], bd_addr[2],
         bd_addr[3], bd_addr[4], bd_addr[5]);
}
