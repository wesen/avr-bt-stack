BTFLAGS ?= -DBTXIRCOM -DSTACK_UNIX
CFLAGS = -Wall -pedantic -g -Os $(BTFLAGS)
CFLAGS += -DDEBUG
#CFLAGS += -DDMALLOC
LDFLAGS =
#LIBS = -ldmalloc

all: bt-mouse

COMMON_STACK_OBJS = bt_dev.o bt_stack.o bt_hci.o
STDIO_STACK_OBJS = io_stdio.o
UNIX_STACK_OBJS = serial_unix.o

bt_stack.h: bt_dev.h bt_hci.h bt_io.h bt_serial.h

%.o: bt_stack.h

# STACK
stack-unix: $(COMMON_STACK_OBJS) $(STDIO_STACK_OBJS) \
            $(UNIX_STACK_OBJS) stack-unix.o
	$(CC) $(CFLAGS) $(BTFLAGS) $(LDFLAGS) \
         -o stack-unix $(COMMON_STACK_OBJS) \
                       $(UNIX_STACK_OBJS) \
                       $(STDIO_STACK_OBJS) \
                       stack-unix.o $(LIBS)

bt-mouse: $(COMMON_STACK_OBJS) $(STDIO_STACK_OBJS) \
            $(UNIX_STACK_OBJS) bt-mouse.o
	$(CC) $(CFLAGS) $(BTFLAGS) $(LDFLAGS) \
         -o bt-mouse $(COMMON_STACK_OBJS) \
                       $(UNIX_STACK_OBJS) \
                       $(STDIO_STACK_OBJS) \
                       bt-mouse.o $(LIBS)


clean:
	- rm -f *.o *~ *core *.elf *.hex *.map *.srec *.lst *.avrobj \
                stack-unix test stack-ericsson stack-xircom stack-avr \
                bt-mouse
