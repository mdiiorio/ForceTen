#include <dos.h>
static union REGS rg;

/*===========---- Position Cursor ----==============*/

void cursor(int x,int y)
{
rg.h.ah=0x02;
rg.x.ax=0x200;
rg.x.bx=0;
rg.x.dx=((y << 8) & 0xff00) + x;
int86(0x10,&rg,&rg);
}

/*===========---- Return Cursor ----================*/

void curr_cursor(int *x,int *y)
{
rg.h.ah=0x03;
rg.x.ax=0x300;
rg.x.bx=0;
int86(0x10,&rg,&rg);
*x=rg.h.dl;
*y=rg.h.dh;
}

/*===========---- Set Cursor ----===================*/

void set_cursor_type(int t)
{
rg.h.ah=0x01;
rg.x.ax=0x100;
rg.x.bx=0;
rg.x.cx=t;
int86(0x10,&rg,&rg);
}

/*===========---- Clear Screen ----=================*/

char attrib=7;

void clear_screen()
{
cursor(0,0);
rg.h.al=' ';
rg.h.ah=9;
rg.x.bx=attrib;
rg.x.cx=2000;
int86(0x10,&rg,&rg);
}

void clear_screen2()
{
cursor(0,0);
rg.h.al=' ';
rg.h.ah=9;
rg.x.bx=attrib;
rg.x.cx=1840;
int86(0x10,&rg,&rg);
}

/*===========---- Return Video ----=================*/

int vmode()
{
rg.h.ah=0x0f;
int86(0x10,&rg,&rg);
return rg.h.al;
}

/*===========---- Test for Scoll Lock ----==========*/

int scroll_lock()
{
rg.x.ax=0x200;
int86(0x16,&rg,&rg);
return rg.h.al & 0x10;
}

/*===========---- Get a Char from Keyboard ----=====*/

void (*helpfunc)();
int helpkey=0;
int helping=0;

int get_char()
{
int c;
while(1)
	{
	rg.h.ah=1;
	int86(0x16,&rg,&rg);
	if(rg.x.flags & 0x40)
		{
		int86(0x28,&rg,&rg);
		continue;
		}
	rg.h.ah=0;
	int86(0x16,&rg,&rg);
	if(rg.h.al==0)
		c=rg.h.ah | 128;
	else
		c=rg.h.al;
	if(c==helpkey)
		{
		if(!helping && helpfunc)
			{
			helping=1;
			(*helpfunc)();
			helping=0;
			continue;
			}
		}
		break;
	}
	return c;
}


/*
/*===========---- Write a char to Video RAM ----====*/

void vpoke(unsigned vseg,unsigned adr,unsigned chr)
{
if(vseg==45056)
    poke(vseg,adr,chr);
else
    {
	_DI=adr;
	_ES=vseg;
	asm	cld;
	_BX=chr;
	_DX=986;
/*--------------- Wait for retrace to start --------*/
	do
		asm 	in 	al,dx;
	while(_AL & 1);
/*--------------- Wait for retrace to stop ---------*/
	do
		asm 	in		al,dx;
	while (!(_AL & 1));
	_AL=_BL;
	asm 	stosb;					   	  /* Store Char */
/*--------------- Wait for retrace to start --------*/
	do
		asm	in		al,dx;
	while(_AL & 1);
/*--------------- Wait for retrace to stop ---------*/
	do
		asm	in 	al,dx;
	while(!(_AL & 1));
	_AL=_BH;
	asm 	stosb;
	}
}
*/

/*
/*===========---- Read a char from Video RAM ----===*/

int vpeek(unsigned vseg,unsigned adr)
{
int ch,at;
if(vseg==45056)
	return peek(vseg,adr);
asm	push	ds;
_DX=986;
_DS=vseg;
_SI=adr;
asm	cld;
/*-------------- Wait for retrace to start ---------*/
do
	asm	in		al,dx;
while(_AL & 1);
/*-------------- Wait for retrace to stop ----------*/
do
	asm	in		al,dx;
while(!(_AL & 1));
asm	lodsb;                   		    /* Get Char */
_BL=_AL;
/*-------------- Wait for retrace to start ---------*/
do
	asm	in		al,dx;
while(_AL & 1);
/*-------------- Wait for retrace to stop ----------*/
do
	asm	in		al,dx;
while(!(_AL & 1));
asm	lodsb;
_BH=_AL;
_AX=_BX;
asm	pop	ds;
return _AX;
}
*/
