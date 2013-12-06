#include <stdarg.h>
#include <dos.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include "struct.h"

extern struct userfile user;
extern int baud;
extern int stat;
extern int timecheck;
extern struct configfile config;
extern time_t timeon;
extern int useron;
extern int timeleft;
int ctrlp,mci,okmci=0,stat=0,dirwrite=1;
union REGS r;


/*    From CompuServe's BPROGB Borland Int'l forum
 *
 *    #:  11879 S4/Turbo C
 *        02-Jun-88  18:54:50
 *    Sb: #11817-VT100
 *    Fm: Jerry Joplin 70441,2627
 *    To: Pete Gontier 72261,1754 (X)
 *
 *    Pete, here are the communication ISR's I've been using.   Hope these
 *    don't overflow the message data base for BPROGB.  :-)
 *    I think all is included to set up COM1/COM2 for receiver interrupts.
 *
 *    Also, these are straight ports from routines in MSKERMIT 2.30 and CKERMIT
 *    for UNIX, which are public domain but copyrighted (c) by Columbia
 *    University.
 */

/*    Modification, to support other compiler(s):
 *       Pete Gontier, July 20th, 1988
 *    This code is no longer portable to the UNIX systems mentioned.
 *    The function names were in accordance with the TT teletype conventions,
 *    and I couldn't stand for it. So sue me.
 */

/*    Modification, to support comm ports 3 & 4
 *       Al Sharpe, June 7, 1989
 *    Minor changes added to handle DTR on port close.
 *
 *    New function added :
 *      flsh_dtr( )   -  drops DTR for 200 milliseconds
 *
 *    Old function modified :
 *      comm_getc( unsigned seconds )   -  (1) gets a char from input buffer
 *                                             with timeout
 *                                         (2) if a key is pressed a Timeout
 *                                             is returned
 */

/*     Last modification by Al Sharpe, April 28, 1990.
 *
 *     - corrected comm_close() to drop RTS and DTR, not just DTR.
 *     - correct flsh_dtr( ) to drop only DTR and leave RTS active.
 *     - flsh_dtr( ) duration extented to 1/2 second.
 *     - correct all bugs in function main( ) so test program will work.
 *     - removed global CARRIER as it was redundant.
 *     - conio.h added for cprintf( ) function.
 */


#include <dos.h>
#include <bios.h>
#include <conio.h>

#define TCICOMM    /* avoid extern declaration in... */
#include "comm.h"  /* comm prototypes */

/** macros for absolute data ************************************************/

#define MDMDAT1 0x03F8            /* Address of modem port 1 data */
#define MDMSTS1 0x03FD            /* Address of modem port 1 status  */
#define MDMCOM1 0x03FB            /* Address of modem port 1 command */
#define MDMDAT2 0x02F8            /* Address of modem port 2 data */
#define MDMSTS2 0x02FD            /* Address of modem port 2 status */
#define MDMCOM2 0x02FB            /* Address of modem port 2 command */
#define MDMDAT3 0x03E8            /* Address of modem port 3 data */
#define MDMSTS3 0x03ED            /* Address of modem port 3 status  */
#define MDMCOM3 0x03EB            /* Address of modem port 3 command */
#define MDMDAT4 0x02E8            /* Address of modem port 4 data */
#define MDMSTS4 0x02ED            /* Address of modem port 4 status */
#define MDMCOM4 0x02EB            /* Address of modem port 4 command */
#define MDMINTV 0x000C            /* Com 1 & 3 interrupt vector */
#define MDINTV2 0x000B            /* Com 2 & 4 interrupt vector */
#define MDMINTO 0x0EF             /* Mask to enable IRQ3 for port 1 & 3 */
#define MDINTO2 0x0F7             /* Mask to enable IRQ4 for port 2 & 4 */
#define MDMINTC 0x010             /* Mask to Disable IRQ4 for port 1 & 3 */
#define MDINTC2 0x008             /* Mask to Disable IRQ3 for port 2 & 4 */
#define INTCONT 0x0021            /* 8259 interrupt controller ICW2-3 */
#define INTCON1 0x0020            /* Address of 8259 ICW1 */
#define CBS     2048              /* Communications port buffer size */
#define XOFF    0x13              /* XON/XOFF */
#define XON     0x11
#define CARRIER_ON  0x80          /* Return value for carrier detect */


/** globals not externally accessible ***************************************/

static int   MODEM_STAT;          /* 8250 modem status register */
static int   dat8250;             /* 8250 data register */
static int   stat8250;            /* 8250 line-status register */
static int   com8250;             /* 8250 line-control register */
static char  en8259;              /* 8259 IRQ enable mask */
static char  dis8259;             /* 8259 IRQ disable mask */
static unsigned int intv;         /* interrupt number to usurp */

static char  buffer[CBS];         /* Communications circular buffer */
static char *inptr;               /* input address of circular buffer */
static char *outptr;              /* output address of circular buffer */
static int   c_in_buf = 0;        /* count of characters received */
static int   xoffpt;              /* amount of buffer that forces XOFF */
static int   xonpt;               /* amount of buffer that unXOFFs */


static void interrupt (far *oldvec)();
                                  /* vector of previous comm interrupt */

int xonxoff = 0;                          /* auto xon/xoff support flag */
int xofsnt  = 0;                          /* XOFF transmitted flag */
int xofrcv  = 0;                          /* XOFF received flag */

void interrupt serint (void)
{											         /* ISR to receive character */
   *inptr++ = inportb ( dat8250 );        /* Store character in buffer */
   c_in_buf++;                            /* and increment count */
   if ( xonxoff ) {                       /* if xon/xoff auto-support is on */
      if ( c_in_buf > xoffpt && ! xofsnt ) {  /* then if buf nearly full */
         comm_putc ( XOFF );              /* send an XOFF */
         xofsnt = 1;                      /* and say so */
      }
   }
   disable ( );                           /* ints off for ptr change */
   if ( inptr == &buffer[CBS] )           /* Set buffer input pointer */
      inptr = buffer;
   enable ( );
   outportb ( 0x20, 0x20 );               /* Generic EOI to 8259 */
}


comm_close () {             /* restore previous settings of 8259 */

    outportb ( com8250 + 1, 0x08 );      /* Drop OUT2 */
	 outportb ( com8250 + 1, 0x00 );      /* Drop DTR and RTS */
    outportb ( INTCONT, dis8259 | inportb ( INTCONT ) );
       /* Disable com interrupt at 8259 */
	  setvect ( intv, oldvec );      /* Reset original interrupt vector */
}

dobaud ( unsigned baudrate ) {     /* parses baud integer to mask,
                                         * re-inits port accordingly */
   unsigned char portval;
   unsigned char blo, bhi;
   switch  ( baudrate ) {          /* Baud rate LSB's and MSB's */
       case 50:     bhi = 0x9;  blo = 0x00;  break;
       case 75:     bhi = 0x6;  blo = 0x00;  break;
       case 110:    bhi = 0x4;  blo = 0x17;  break;
       case 150:    bhi = 0x3;  blo = 0x00;  break;
       case 300:    bhi = 0x1;  blo = 0x80;  break;
       case 600:    bhi = 0x0;  blo = 0xC0;  break;
       case 1200:   bhi = 0x0;  blo = 0x60;  break;
       case 1800:   bhi = 0x0;  blo = 0x40;  break;
       case 2000:   bhi = 0x0;  blo = 0x3A;  break;
       case 2400:   bhi = 0x0;  blo = 0x30;  break;
       case 4800:   bhi = 0x0;  blo = 0x18;  break;
       case 9600:   bhi = 0x0;  blo = 0x0C;  break;
       case 19200:  bhi = 0x0;  blo = 0x06;  break;
       default:
           return;
   }
   portval = inportb ( com8250 );         /* read Line-Control Reg val */
   outportb ( com8250, portval | 0x80 );  /* set high bit for baud init */
   outportb ( dat8250, blo );             /* Send LSB for baud rate */
   outportb ( dat8250 + 1, bhi );         /* Send MSB for baud rate */
   outportb ( com8250, portval );         /* Reset initial value at LCR */
}


/* installs comm interrupts */

comm_open ( int portid, unsigned speed )
{
   int be = biosequip ( );      /* to get # installed serial ports */
   be <<= 4;                    /* shift-wrap high bits off */
   be >>= 13;                   /* shift down to low bits */
   if ( be >= portid || portid >= 3) {
      if ( portid == 1 ) {
          dat8250  = MDMDAT1;
          stat8250 = MDMSTS1;
          com8250  = MDMCOM1;
          dis8259  = MDMINTC;
          en8259   = MDMINTO;
          intv = MDMINTV;
          }
      else if ( portid == 2 ) {
          dat8250  = MDMDAT2;
          stat8250 = MDMSTS2;
          com8250  = MDMCOM2;
          dis8259  = MDINTC2;
          en8259   = MDINTO2;
          intv = MDINTV2;
          }
      else if ( portid == 3 ) {		/* Ports 3 & 4 cannot be checked */
      dat8250  = MDMDAT3;           /* with biosquip( ) */
          stat8250 = MDMSTS3;
          com8250  = MDMCOM3;
          dis8259  = MDMINTC;
          en8259   = MDMINTO;
          intv = MDMINTV;
          }
      else if ( portid == 4 ) {
          dat8250  = MDMDAT4;
          stat8250 = MDMSTS4;
          com8250  = MDMCOM4;
          dis8259  = MDINTC2;
          en8259   = MDINTO2;
          intv = MDINTV2;
          }
      else
         return ( 0 );

      MODEM_STAT = dat8250 + 6;           /* Define Modem Status Register */

      dobaud ( speed );                   /* set baud */
      inptr = outptr = buffer;            /* set circular buffer values */
      c_in_buf = 0;
      oldvec = getvect ( intv );          /* Save old int vector */
		setvect  ( intv, serint );          /* Set up SERINT as com ISR */
      outportb ( com8250,     0x3 );      /* 8 bits no parity */
      outportb ( com8250 + 1, 0xb );      /* Assert OUT2, RTS, and DTR */
      inportb  ( dat8250 );
      outportb ( dat8250 + 1, 0x1 );      /* Receiver-Data-Ready int */
      outportb ( INTCONT, en8259 & inportb ( INTCONT ) );
                                          /* Enable 8259 interrupts */
      xoffpt = CBS - 128;                 /* chars in buff to send XOFF */
      xonpt  = CBS - xoffpt;              /* chars in buff to send XON */
     }
   else
      be = 0;
   return ( be );
}


comm_avail ( )       /* returns # characters available in buffer */
{
    return ( c_in_buf );
}


comm_putc ( unsigned char c )      /* sends char out port */
{
    while ( ( inportb ( stat8250 ) & 0x20 ) == 0 );
					/* Wait til transmitter is ready */
    outportb ( dat8250, c );            /* then send it */
}


comm_getc ( unsigned seconds )         /* gets char from buffer */
{
    int c;
    long get_tm, end_tm;

    register char * ptr;
    if ( c_in_buf < xonpt && xofsnt ) {   /* Check if we need to send */
       xofsnt = 0;                        /* an XON to the host after */
       comm_putc ( XON );                 /* we had to send an XOFF */
       }

    get_tm = biostime(0,0);               /* If character not ready */
    end_tm = get_tm + (18 * seconds);     /* then wait til one is   */
                                          /* or return TIMEOUT */
    do {
        if(kbhit()) {                     /* If key pressed, Timeout */
            getch();                      /* is returned */
            comm_putc('\20');             /* send 'space' to modem to */
        return(USR_BRK);                  /* cancel off-hook */
            }
        get_tm = biostime(0,0);           /* check Timeout counter */
        }
    while(get_tm <= end_tm && c_in_buf == 0) ;

    if(get_tm >= end_tm) return(TIMEOUT);  /* port timed out */

    ptr = outptr;
    c = *ptr++;                     /* Get next character in circular buff */
    if ( ptr == &buffer[CBS] )      /* Check for end of circular buffer */
        ptr = buffer;               /* start from bottom of buff */
    disable ( );                    /* no interrupts during pointer manips */
    outptr = ptr;                   /* set character output pointer */
    c_in_buf--;                     /* and decrement the character count */
    enable ( );                     /* then allow interrupts to continue */
    return ( c );                   /* Return the character */
}

comm_flush ( )                 /* flushes all chars out of buffer */
{
    if ( xofsnt ) {                 /* Check if XON needs to be sent */
       xofsnt = 0;
       comm_putc ( XON );
    }
    disable ( );                    /* no interrupts during pointer manips */
    inptr = outptr = buffer;        /* reset buffer pointers */
    c_in_buf = 0;                   /* and indicate no chars received */
    enable ( );
}

flsh_dtr ( )                   /* Drop DTR for 1/2 second */
{
    outportb( com8250 + 1, 0xa);    /* Un-set DTR (set RTS, OUT2) */
    delay(500);                     /* Wait for 1/2 second */
    outportb( com8250 + 1, 0xb);    /* Set DTR, RTS, OUT2 */

}


carrier()          /* TEST FOR CARRIER DETECT */
{

/*	delay(200);                        Short pause */
	if((inportb( MODEM_STAT ) & CARRIER_ON ) == CARRIER_ON)
        return(1);                  /* Check MSR for DCD */
	return(0);

}

pr1(char *s)
{
int i;

for(i=0;s[i] > 0; i++)
	if(s[i] == '|')
		comm_putc(13);
	else if(s[i] == '~')
		sleep(0.50);
	else
		comm_putc(s[i]);
}

pl(char *s)
{
put(s);
nl();
}

put2(char *s)
{
dirwrite=0;
while(*s)
	put_char(*s++);
dirwrite=1;
}

nl()
{
put("\r\n");
}

bs()
{
put("\b \b");
}



putremote(char *s)
{
if(useron)
	while(*s)
		comm_putc(*s++);
}

put(char *s)
{
while(*s)
	put_char(*s++);
}

putlocal(char *s)
{
while(*s)
	put_char(*s++);
}

hang_up()
{
put("+++");
sleep(1);
put("ATH0\r");
}

drop_dtr()
{
flsh_dtr();
comm_close(config.port,config.baud);
}



inputpass(char *buff,int len)
{
inputl(buff,len,1);
}

input(char *buff,int len)
{
inputl(buff,len,0);
}

inputl(char *buff,int len,int echo)
{
int c,k;
int n,i,f;
int done=0;
i=0;
while(!done)
	{
	c=get_c();            /* get a key, */
	if(c==-1)
		{
		buff[0]=0;
		return;
		}
	if(c == 32)
		{
		if(i == 0)
			c=0;
		}
	if(c > 31 && c < 127)
		{
		if(i < len)
			{
			buff[i++]=c;
			if(echo)
				put_char('ä');
			else
				put_char(c);
			}
		}
	switch (c)
		{
		case 27:
			for(k=i;k!=0;k--)
				put("\b");
			for(k=i;k!=0;k--)
				put(" ");
			for(k=i;k!=0;k--)
				put("\b");
			i=0;
			break;
		case 13:		/* end of input */
	      buff[i]=0;
			put_char(' ');
			done=1;
		case 8:		/* delete character */
			if(i > 0)
				{
				i--;
				bs();
				}
			break;
		}
	}
updatestat();
}

inputphone(char *buff)
{
int c,k,len,n,i,f,done;
char s[80];

s[0]=done=buff[0]=0;
len=10;
put("1(   )   -    2");
for(i=0;i < 12;i++)
	put("\b");
i=0;
while(!done)
	{
	c=get_c();            /* get a key, */
	if(c==-1)
		{
		s[0]=0;
		return;
		}
	if(c > 31 && c < 127)
		{
		if(i < len && c != 32 && isdigit(c))
			{
			s[i++]=c;
			put_char(c);
			if(i == 3)
				put("1)2");
			if(i == 6)
				put("1-2");

			}
		}
	switch (c)
		{
		case 13:		/* end of input */
			if(i==10)
				{
				s[i]=0;
				put_char(' ');
				done=1;
				}
			break;
		case 8:		/* delete character */
			if(i > 0)
				{
				if(i == 3 || i == 6)
					{
					put("\b");
					put("\b");
					put(" ");
					put("\b");
					i--;
					}
				else
					{
					i--;
					bs();
					}
				}
			break;
		}
	}
buff[0]='(';
buff[1]=s[0];
buff[2]=s[1];
buff[3]=s[2];
buff[4]=')';
buff[5]=s[3];
buff[6]=s[4];
buff[7]=s[5];
buff[8]='-';
buff[9]=s[6];
buff[10]=s[7];
buff[11]=s[8];
buff[12]=s[9];
buff[13]=0;
}



print(char *ln, ...)
{
char dlin[100], *dl=dlin;

va_list ap;
va_start(ap,ln);
vsprintf(dlin,ln,ap);
va_end(ap);
while(*dl)
	put_char(*dl++);
}


inputlocal(char *buff,int len)
{
int c;
int n,i,f;
int done=0;
i=0;
while(!done)
	{
	c=getch();            /* get a key, */
	if(c > 31 && c < 127)
		{
		if(i < len)
			{
			buff[i++]=c;
			cprintf("%c",c);
			}
		}
	switch (c)
		{
		case 13:		/* end of input */
	      buff[i]=0;
			putchar(' ');
			done=1;
		case 8:		/* delete character */
			if(i > 0)
				{
				i--;
				putchar('\b');
				putchar(' ');
				putchar('\b');
				}
			break;
		}
	}
}

yn()
{
int c;

while(1)
	{
	c=get_c();
	c=upcase(c);
	if(c=='Y' || c==13)
		{
		put("2Yes");
		return(1);
		}
	if(c=='N')
		{
		put("2No");
		return(0);
		}
	}
}

ny()
{
int c;

while(1)
	{
	c=get_c();
	c=upcase(c);
	if(c=='Y')
		{
		put("2Yes");
		return(0);
		}
	if(c=='N' || c==13)
		{
		put("2No");
		return(1);
		}
	}
}


ynq()
{
int c;

while(1)
	{
	c=get_c();
	c=upcase(c);
	if(c=='Y' || c==13)
		{
		put("2Yes");
		return(1);
		}
	if(c=='N')
		{
		put("2No");
		return(0);
		}
	if(c=='Q')
		{
		put("2Quit");
		return(-1);
		}
	}
}

updatestat()
{
struct text_info tinf;
char s[80];

if(stat)
	{
	gettextinfo(&tinf);
	textcolor(BLACK);
	textbackground(WHITE);
	gotoxy(1,24);
	sprintf(s,"[¯%s®] [%d left] [%d SL] [%s] [%s]",user.name,chktime()/60,user.sl,user.phone,"No");
	cprintf("%-79s",s);
	gotoxy(1,25);
	sprintf(s,"[%s]",user.note);
	cprintf("%-79s",s);
	gotoxy(tinf.curx,tinf.cury);
	textattr(tinf.attribute);
	}
}

setscroll()
{
int x,y;
if(stat)
	{
	x=wherex();
	y=wherey();
	gotoxy(1,24);
	printf("%-79s"," ");
	gotoxy(1,25);
	printf("%-79s"," ");
	stat=0;
	gotoxy(x,y);
	}
else
	{
	stat=1;
	x=wherex();
	if(wherey() > 23)
		{
		r.h.ah=6;
		if(wherey()==24)
			r.h.al=1;
		if(wherey()==25)
			r.h.al=2;
		r.h.bh=7;
		r.h.cl=0;
		r.h.ch=0;
		r.h.dl=79;
		if(wherey()==24)
			r.h.dh=23;
		if(wherey()==25)
			r.h.dh=24;
		int86(0x10,&r,&r);
		gotoxy(x,23);
		}
	updatestat();
	}
}


clear_screen()
{
gotoxy(1,1);
r.h.al=' ';
r.h.ah=9;
r.x.bx=7;
if(stat==0)
	r.x.cx=2000;
if(stat==1)
	r.x.cx=1840;
int86(0x10,&r,&r);
}

setmci(int i)
{
okmci=i;
}

put_char(int c)
{
if(c=='@' && okmci)
	mci=1;

else if(mci)
	{
	mci=0;
	domci(c);
	}

else if(c==3)
	ctrlp=1;

else if(ctrlp)
	{
	if(c < 53 && c > 48)
		{
		ctrlp=0;
		ansic(c-48);
		}
	}
else
	{
	if(c==12)
		{
		ctrlp=0;
		textbackground(BLACK);
		clear_screen();
		if(useron)
			comm_putc(12);
		}
	else
		{
		ctrlp=0;
		if(useron)
			comm_putc(c);
		if(dirwrite)
			cprintf("%c",c);
		else
			putchar(c);
		}
	}
if(wherey()==24)
	scrollup();
}


domci(int c)
{
switch(c)
	{
	case 'H':
		if(okmci)
			print("%s",user.name);
		break;
	case '$':
		if(okmci)
			print("%s",user.pass);
		break;
	case 'T':
		if(okmci)
			print("%d",chktime()/60);
		break;
	case 'I':
		if(okmci)
			get_c();
		break;
	case 'W':
		if(okmci)
			sleep(1);
		break;
	case 'B':
		if(okmci)
			put_char(7);
		break;
	case 'U':
		if(okmci)
			if(useron)
				comm_putc(7);
		break;
	case 'S':
		if(okmci)
			putchar(7);
		break;
	case 'P':
		if(okmci)
			print("%s",user.phone);
		break;
	case 'R':
		if(okmci)
			print("%s",user.realname);
		break;
	case 'N':
		if(okmci)
			print("%s",user.note);
		break;
	case 'M':
		if(okmci)
			nl();
		break;
	}
}


scrollup()
{

if(stat)
	{
	r.h.ah=6;
	r.h.al=1;
	r.h.bh=7;
	r.h.cl=0;
	r.h.ch=0;
	r.h.dl=79;
	r.h.dh=22;
	int86(0x10,&r,&r);
	gotoxy(1,23);
	}
}

chk_modem()
{
int i=0,n=0;
char buff[41];
delay(200);
while(n < 40)
	{
	if(comm_avail() > 0)
		{
		buff[i]= comm_getc(1);
		if (isdigit(buff[i]))
			++i;
		}
	buff[i]=0;
	n++;
	}
return(atoi(buff));
}



get_c()
{
time_t tstart,tnow;
int beeponce=0,c;
int tbuf=0,goon=1,kh=0,ch=0;

beeponce=0;
if(user.sl>0)
	chktime();
time(&tstart);

while(goon)
	{
	checkhang();
	if(kbhit()!=0)
		{
		kh=1;
		goon=0;
		}
	if(comm_avail()!=0)
		{
		ch=1;
		goon=0;
		}
	tbuf=difftime(time(&tnow),tstart);
	if(tbuf>90)
		{
		if(!beeponce)
			{
			put_char(7);
			put_char(7);
			beeponce=1;
			}
		}
	if(tbuf>180)
		{
		pl("Too bad...");
		hangup();
		}
	}
if(kh)
	{
	c=getch();
	if(c != 0)
		return(c);
	else
		{
		c=getch();
		do_ext(c);
		if(c == 68)
			return(-1);
		}
	}
if(ch)
	{
	c=comm_getc(1);
	if(c < 254)
		return(c);
	}
}

upcase(int c)
{
if((c > '`') && (c < '{'))
	c=c-32;
return(c);
}



print_time()
{
int c;
time_t timenow;

time(&timenow);

c=timeleft - difftime(timenow,timeon);
c+=60;
return(c);
}

chktime()
{
int c;
time_t timenow;

time(&timenow);

c=timeleft - difftime(timenow,timeon);
c+=60;
if(c<=0 && timecheck)
	{
	put("Your time is up. Call back tommorow.");
	hangup();
	}
return(c);
}

hangup()
{
time_t timenow;

time(&timenow);
if(useron)
	drop_dtr();
user.timeon=difftime(timenow,timeon) + user.timeon;
writeuser();
gotowfc();
}

newhangup()
{
time_t timenow;

time(&timenow);
if(useron)
	drop_dtr();
writeuser();
gotowfc();
}

hangnosave()
{
if(useron)
	drop_dtr();
gotowfc();
}

checkhang()
{
int i,ok;

if(!carrier() && useron)
	{
	if(user.number > 0)
		hangup();
	else
		hangnosave();
	}
}

cursor(int x,int y)
{
r.x.ax=0x0200;
r.x.bx=0;
r.x.dx=((y << 8) & 0xff00) + x;
int86(0x10, &r, &r);
}
