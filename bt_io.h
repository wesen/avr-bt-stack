#ifndef IO_H__
#define IO_H__

/**
 * @file io.h
 * I/O routines (prototypes) 
 */

/** output a string */
void io_str   (char *);
/** output an integer */
void io_int   (int);
/** output an error string */
void error_str(char *);
void io_bd_addr(unsigned char *);

#ifdef DEBUG
/** output a string for debug */
#define DEBUG_STR(s) io_str((s))
/** output an error string */
#define PERR_STR(s) error_str((s))
/** output an error string */
#define PERR_INT(s) io_int((s))
/** output an integer for debug */
#define DEBUG_INT(s) io_int((int)(s))
#else
#define DEBUG_STR(s)
#define DEBUG_INT(s)
#define PERR_STR(s)
#define PERR_INT(s)
#endif /* DEBUG */

#endif /* IO_H__ */
