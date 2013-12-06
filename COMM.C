#include <stdarg.h>
#include <dos.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include "struct.h"
#include <conio.h>


extern struct userfile user;
extern int baud;
extern int stat;
extern int timecheck;
extern struct configfile config;
extern time_t timeon;
extern int useron;
extern int timeleft;
extern int mouseyes,button_num;
int ctrlp,mci,okmci=0,stat=0,dirwrite=1;
union REGS r;

#define max_buf 1024

volatile int head,tail;
volatile char buffer[max_buf];

int base,async_irq,flow_control;

int irqs [4] = {4,3,0,0};
int bases[4] = {0x03f8,0x02f8,0x03e8,0x02e8};

void far interrupt async_isr ();

void far interrupt async_isr ()
/* This function is called every time a char is received on the com port.
 * The character is stored in the buffer[] array, and the head pointer is
 * updated.
 */
{
  buffer[head++] = inportb(base);
  if (head == max_buf)
    head = 0;
  outportb(0x20, 0x20);
}



void comm_putc(char ch)
/* This function outputs one character to the com port */
{
  while (!(inportb(base + 5) & 0x20))
    ;
  if (flow_control)
    while (!(inportb(base + 6) & 0x10))
      ;
  outportb(base, ch);
}


char comm_getc()
/* This function returns one character from the com port, or a zero if
 * no character is waiting
 */
{
  char c1;

  if (head != tail) {
    disable();
    c1 = buffer[tail++];
    if (tail == max_buf)
      tail = 0;
    enable();
    return(c1);
  } else
    return(0);
}



int comm_avail()
/* This returns a value telling if there is a character waiting in the com
 * buffer.
 */
{
  return(head != tail);
}



void comm_flush()
/* This function clears the com buffer */
{
  disable();
  head = tail = 0;
  enable();
}



void dobaud(unsigned int rate)
/* This function sets the com speed to that passed */
{
  float rl;

  if ((rate > 49) && (rate < 57601)) {
    rl   = 115200.0 / ((float) rate);
    rate = (int) rl;
    outportb(base + 3, inportb(base + 3) | 0x80);
    outportb(base,     (rate & 0x00FF));
    outportb(base + 1, ((rate >> 8) & 0x00FF));
    outportb(base + 3, inportb(base + 3) & 0x7F);
  }
}


void comm_open(int port_num, unsigned baud)
/* This function initializes the com buffer, setting up the interrupt,
 * and com parameters
 */
{
  int temp;

  base = bases[port_num-1];
  async_irq = irqs[port_num-1];
  setvect(8 + async_irq, async_isr);
  head = tail = 0;
  outportb(base + 3, 0x03);
  disable();
  temp = inportb(base + 5);
  temp = inportb(base);
  temp = inportb(0x21);
  temp = temp & ((1 << async_irq) ^ 0x00FF);
  outportb(0x21, temp);
  outportb(base + 1, 0x01);
  temp=inportb(base + 4);
  outportb(base + 4, temp | 0x0A);
  enable();
  dobaud(baud);
  dtr(1);
}



void comm_close()
/* This function closes out the com port, removing the interrupt routine,
 * etc.
 */
{
  int temp;

  disable();
  temp = inportb(0x21);
  temp = temp | ((1 << async_irq));
  outportb(0x21, temp | 0x10);
  outportb(base + 2, 0);
  outportb(base + 4, 1);
  enable();
}

dtr(int i)
/* This function sets the DTR pin to the status given */
{
  int i1;

  i1 = inportb(base + 4) & 0x00FE;
  outportb(base + 4, i ? i1 + 1 : i1);
}


void rts(int i)
/* This function sets the RTS pin to the status given */
{
  int i1;

  i1 = inportb(base + 4) & 0x00FD;
  outportb(base + 4, (i) ? (i1 + 2) : i1);
}


int cdet()
/* This returns the status of the carrier detect lead from the modem */
{
  return((inportb(base + 6) & 0x80) ? 1 : 0);
}



carrier()
/* This function checks to see if the user logged on to the com port has
 * hung up.  Obviously, if no user is logged on remotely, this does nothing.
 * If carrier detect is detected to be low, it is checked 100 times
 * sequentially to make sure it stays down, and is not just a quirk.
 */
{
int i, ok;

ok = 0;
if (!cdet())
	return(0);
else
	return(1);
}


checkhang()
{
int i,ok;
time_t timenow;

if(!carrier() && useron)
	 {
	 printf("\b");
    if(user.number > 0)
		  {
		  time(&timenow);
		  user.timeon=difftime(timenow,timeon) + user.timeon;
		  writeuser();
		  gotowfc();
		  }
	 else
		gotowfc();
	 }
}


/*void get_modem_line(char *s, double d)
{
  int i=0;
  char ch=0, ch1;
  double t;

  t = timer();
  do {
    ch = get1c();
    if (kbhitb()) {
      ch1 = getchd();
      if (upcase(ch1) == 'H') {
        ch = 13;
        s[0] = i = 1;
      }
    }
    if (ch >= 32)
      s[i++] = upcase(ch);
  } while ((ch != 13) && (fabs(timer() - t) < d) && (i<=30));
  s[i] = 0;
}*/



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

void drop_dtr()
{
put("+++");
sleep(1);
put("ATH0\r");
}


/*
void drop_dtr()
{
char ch;

ch = inportb(base + 4);
ch &= 0xfe;
outportb(base + 4,ch);
}
*/


inputpass(char *buff,int len,char *rs)
{
char ss[300],s[80];
struct text_info txti;

gettextinfo(&txti);
gettext(50,3,80,6,ss);
textcolor(LIGHTBLUE);
gotoxy(50,3);
cprintf("ÚÄÄÄÄÄ");
textcolor(GREEN);
cprintf("[ ");
textcolor(LIGHTGREEN);
cprintf("Password Entry");
textcolor(GREEN);
cprintf(" ]");
textcolor(LIGHTBLUE);
cprintf("ÄÄÄÄÄ¿");
gotoxy(50,4);
cprintf("³                            ³");
gotoxy(50,5);
cprintf("³                            ³");
gotoxy(50,6);
cprintf("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
gotoxy(52,4);
textcolor(LIGHTGRAY);
cprintf("Real   : ");
textcolor(YELLOW);
strcpy(s,rs);
cprintf("%s",s);
gotoxy(52,5);
textcolor(LIGHTGRAY);
cprintf("Entered: ");
textattr(txti.attribute);
gotoxy(txti.curx,txti.cury);
inputl(buff,len,1);
puttext(50,3,80,6,ss);
}

input(char *buff,int len)
{
inputl(buff,len,0);
}

inputl(char *buff,int len,int echo)
{
struct text_info txti;
int c,k,n,i,f;
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
				{
				gettextinfo(&txti);
				textcolor(YELLOW);
				gotoxy(60+i,5);
				cprintf("%c",c);
				textattr(txti.attribute);
				gotoxy(txti.curx,txti.cury);
				put_char('ä');
				}
			else
				put_char(c);
			}
		}
	switch (c)
		{
		case 27:
			if(echo)
				{
				gettextinfo(&txti);
				textcolor(YELLOW);
				gotoxy(60,5);
				cprintf("                  ");
				textattr(txti.attribute);
				gotoxy(txti.curx,txti.cury);
				}
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
/*			put_char(' ');*/
			done=1;
			break;
		case 8:		/* delete character */
			if(i > 0)
				{
				if(echo)
					{
					gettextinfo(&txti);
					textcolor(YELLOW);
					gotoxy(60+(i+1),5);
					cprintf("\b \b");
					textattr(txti.attribute);
					gotoxy(txti.curx,txti.cury);
					}
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
/*				put_char(' ');*/
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
/*			putchar(' ');*/
			done=1;
			break;
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
		put("9Yes");
		return(1);
		}
	if(c=='N')
		{
		put("9No");
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
		put("9Yes");
		return(0);
		}
	if(c=='N' || c==13)
		{
		put("9No");
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
		put("9Yes");
		return(1);
		}
	if(c=='N')
		{
		put("9No");
		return(0);
		}
	if(c=='Q')
		{
		put("9Quit");
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
	hidemouse();
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
	showmouse();
	}
}

setscroll()
{
int x,y;

hidemouse();

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
showmouse();
}


dl(int i)
{
int c;

ansic(5);
for(c=0;c<i;c++)
	print(" ");
for(c=0;c<i;c++)
	print("\b");
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
	if(c < 58 && c > 47)
		{
		ctrlp=0;
		if(c==48)
			ansic(10);
		else
			ansic(c-48);
		}
	}
else
	{
	if(c==12)
		{
		ctrlp=0;
		hidemouse();
		textbackground(BLACK);
		clear_screen();
		if(useron)
			comm_putc(12);
		showmouse();
		}
	else
		{
		ctrlp=0;
		if(useron)
			comm_putc(c);
		hidemouse();
		if(dirwrite)
			cprintf("%c",c);
		else
			putchar(c);
		showmouse();
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
	hidemouse();
	r.h.ah=6;
	r.h.al=1;
	r.h.bh=7;
	r.h.cl=0;
	r.h.ch=0;
	r.h.dl=79;
	r.h.dh=22;
	int86(0x10,&r,&r);
	gotoxy(1,23);
	showmouse();
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
	if(mouseyes)
		checkbutton();
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

cursor(int x,int y)
{
r.x.ax=0x0200;
r.x.bx=0;
r.x.dx=((y << 8) & 0xff00) + x;
int86(0x10, &r, &r);
}
