/* Serial communication routines */

#include "bt_stack.h"

/**
 * @file serial_unix.c
 * UNIX serial communication routines
 */

#if 0
/** called on exit to restore tty settings from #save_termios */
void serial_atexit(bt_device_t *dev) {
   if (dev->ttyfd >= 0) /* reset terminal mode */
      tcsetattr(ttyfd, TCSAFLUSH, &dev->save_termios);
}
#endif

/** 
 * UNIX implementation of #serial_init() 
 *
 * opens the tty defined by #TTY, saves current tty settings into
 * #save_termios, switches it to noncanonical mode, 8N1, and sets
 * serial_atexit() to be called on exit in order to restore the saved
 * settings.
 *
 * @see Stevens, APUE 
 */
int bt_serial_init(bt_dev_t *dev) {
   struct termios term;

   if (((dev->ttyfd = open(dev->tty, O_RDWR | O_NOCTTY)) < 0)) {
      PERR_STR("could not open TTY");
      return 0;
   }

   /* save terminal settings */
   if (!isatty(dev->ttyfd) ||
       (tcgetattr(dev->ttyfd, &dev->save_termios) < 0)) {
      PERR_STR("TTY is not a tty");
      close(dev->ttyfd);
      return 0;
   }

   term = dev->save_termios;

   /* no echo, noncanonical mode, no processing, no signals */
   term.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
   /* no SIGINT, no CR-to-NL, no parity, no strip, no flow control */
   term.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
   /* no size bits, no parity */
   term.c_cflag &= ~(CSIZE | PARENB);
   /* 8 bits */
  term.c_cflag &= ~(HUPCL|PARENB|CSIZE);
  term.c_cflag |= (CS8|CLOCAL);
   /* no processing */
   term.c_oflag &= ~(OPOST);

   term.c_cc[VMIN] = 1;   /* bytewise input */
   term.c_cc[VTIME] = 0;  /* no timeout */

#ifdef BTERICSSON
   /* ericsson devices want 57600 bps on startup */
   if ((cfsetispeed(&term, B57600) < 0) ||
       (cfsetospeed(&term, B57600) <0 )) {
      PERR_STR("Could not set 57600 bps baudrate");
      tcsetattr(dev->ttyfd, TCSAFLUSH, &dev->save_termios);
      close(dev->ttyfd);
      return 0;
   }
#endif /* BTERICSSON */

#ifdef BTXIRCOM
   /* xircom devices want 115200 bps on startup */
   if ((cfsetispeed(&term, B115200) < 0) ||
       (cfsetospeed(&term, B115200) <0 )) {
      PERR_STR("Could not set 115200 bps baudrate");
      tcsetattr(dev->ttyfd, TCSAFLUSH, &dev->save_termios);
      close(dev->ttyfd);
      return 0;
   }
#endif /* BXIRCOM */

   if (tcsetattr(dev->ttyfd, TCSAFLUSH, &term) < 0) {
      PERR_STR("Could not switch TTY to noncanonical mode");
      close(dev->ttyfd);
      return 0;
   }

   /* atexit(serial_atexit); XXX fix */

   return 1;
}

/**
 * UNIX implementation of #serial_close()
 *
 * restores the saved tty settings from #save_termios, and closes
 * #ttyfd.
 */
int bt_serial_close(bt_dev_t *dev) {
   /* restore settings */
   if (tcsetattr(dev->ttyfd, TCSAFLUSH, &dev->save_termios) < 0) {
      PERR_STR("Could not restore TTY settings");
      close(dev->ttyfd);
      return 0;
   }

   close(dev->ttyfd);

   return 1;
}

/** UNIX implementation of #serial_send() */
int bt_serial_send(bt_dev_t *dev, unsigned char *buf, int len) {
   int tlen = 0, wlen;
   int i;

   printf("Send to serial: ");
   for (i = 0; i < len; i++) {
      printf("%x, ", buf[i]);
   }
   printf("\n");

   while (tlen < len) {
      if ((wlen = write(dev->ttyfd, (void *)(buf + tlen), 1)) < 0)
         return -1;
/*       printf("sent to serial: %x\n", buf[tlen]); */
      tlen += wlen;
   }

   return tlen;
}

/** UNIX implementation of #serial_read() */
int bt_serial_read(bt_dev_t *dev, unsigned char *buf, int len) {
   int tlen = 0, rlen;

   printf("Read from serial: ");
   while (tlen < len) {
      if ((rlen = read(dev->ttyfd, (void *)(buf + tlen), 1)) <= 0)
         return -1;
      printf("%x, ", buf[tlen]);
      tlen += rlen;
   }
    printf("\n");

   return tlen;
}

