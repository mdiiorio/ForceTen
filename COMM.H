#ifndef TCICOMM
   extern int xonxoff;  /* auto xon/xoff support is on flag */
   extern int portin;   /* com port is open flag */
   extern int xofsnt;   /* XOFF transmitted flag */
   extern int xofrcv;   /* XOFF received flag */
#endif

#ifndef TIMEOUT         /* Define Timeout if not already */
#define TIMEOUT -1
#endif
#define USR_BRK -2

/* port number parameters for comm_open() */

#define COM1 1          /* These are not really used but were */
#define COM2 2          /* here before. They can be commented */
#define COM3 3          /* out */
#define COM4 4

/* prototypes for externally called functions */

comm_open  ( int portid, unsigned speed );
comm_close ();
comm_flush ();
comm_avail ();
comm_putc  ( unsigned char c );
comm_getc  ( unsigned seconds );
flsh_dtr   ();
carrier    ();
