#ifndef SERIAL_H__
#define SERIAL_H__

/**
 * @file serial.h
 *
 * Serial communication routines (prototypes) 
 */

/**
 * set up the serial interface 
 *
 * HCI UART specifies:
 *  - 8 bits
 *  - no parity
 *  - 1 stop bit
 *  - RTS/CTS flow control 
 *
 * @return 1 on success, 0 on error
 */
int bt_serial_init(bt_dev_t *dev);

/** 
 * close the serial interface 
 *
 * @return 1 on success, 0 on error
 */
int bt_serial_close(bt_dev_t *dev);

/** 
 * write bytes on the serial interface
 *
 * @param buf pointer to the buffer to be sent
 * @param len number of characters to send
 *
 * @return number of bytes sent, -1 on error
 * @warning should send all bytes out, bluetooth stack will abort if not
 * enough bytes were sent.
 */
int bt_serial_send(bt_dev_t *dev, unsigned char *buf, int len);

/**
 * read bytes from the serial interface
 *
 * @param buf pointer to the buffer to be read in
 * @param len number of characters to read
 *
 * @return number of bytes read, -1 on error
 *
 * @warning may block on read!
 * @warning should return the exact number or error, the stack will
 * abort if not enough bytes have been read
 */
int bt_serial_read(bt_dev_t *dev, unsigned char *buf, int len);


#endif /* SERIAL_H__ */
