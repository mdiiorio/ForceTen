
/*===============---- Twindow.C ----================*/

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <dos.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include "twindow.h"
#include "keys.h"

#define TABS 		 4
#define SCRNHT 	25
#define SCRNWIDTH 80
#define ON 			 1
#define OFF 		 0
#define ERROR 		-1


/*----- Local Proto's -------*/
redraw(WINDOW *wnd);
wframe(WINDOW *wnd);
dtitle(WINDOW *wnd);
int *waddr(WINDOW *wnd,int x,int y);
vswap(WINDOW *wnd);
vsave(WINDOW *wnd);
vrstr(WINDOW *wnd);
add_list(WINDOW *wnd);
beg_list(WINDOW *wnd);
remove_list(WINDOW *wnd);
insert_list(WINDOW *w1,WINDOW *w2);

#ifndef FASTWINDOWS
int dget(WINDOW *wnd,int x,int y);
verify_wnd(WINDOW **w1);
#endif



/*---- Array of border chars ---*/
struct {
	int nw,ne,se,sw,side,line;
} wcs[] = {
	{218,191,217,192,179,196},
	{201,187,188,200,186,205},
	{214,183,189,211,186,196},
	{213,184,190,212,179,205},
	{194,194,217,192,179,196}
};

WINDOW *listhead=NULL;
WINDOW *listtail=NULL;
int VSG;

/*---- establish a window ---*/
WINDOW *establish_window(x,y,h,w)
{
WINDOW *wnd;
/*VSG=(vmode()==7 ? 0xb000 : 0xb800);*/
VSG=0xb800;
if((wnd=(WINDOW *) malloc(sizeof(WINDOW))) == NULL)
	return NULL;
/*---- adjust for out-bound ----*/
WTITLE="";
HEIGHT=min(h,SCRNHT);
WIDTH=min(w,SCRNWIDTH);
COL=max(0,min(x,SCRNWIDTH-WIDTH));
ROW=max(0,min(y,SCRNHT-HEIGHT));
WCURS=0;
SCROLL=0;
SELECT=1;
BTYPE=0;
VISIBLE=HIDDEN=0;
PREV=NEXT=NULL;
FHEAD=FTAIL=NULL;
WBORDER=WNORMAL=PNORMAL=WTITLEC=clr(BLACK,WHITE,BRIGHT);
WACCENT=clr(WHITE,BLACK,DIM);
if((SAV=malloc(WIDTH*HEIGHT*2))==(char *)0)
	return NULL;
add_list(wnd);
#ifndef FASTWINDOWS
	clear_window(wnd);
	wframe(wnd);
#endif
	return wnd;
}

/*---- Set window's border ----*/
void set_border(WINDOW *wnd,int btype)
{
if(verify_wnd(&wnd))
	{
	BTYPE=btype;
	redraw(wnd);
	}
}

/*--- Set colors ----*/
void set_colors(WINDOW *wnd,int area,int bg,int fg,int intern)
{
if(vmode()==7)
	{
	if(bg!=WHITE && bg != BLACK)
		return;
	if(fg!=WHITE && fg != BLACK)
		return;
	}
if(verify_wnd(&wnd))
	{
	if(area==ALL)
		while(area)
			WCOLOR[--area]=clr(bg,fg,intern);
	else
		WCOLOR[area]=clr(bg,fg,intern);
	redraw(wnd);
	}
}

/*--- Set the intensity of a window ----*/
void set_intensity(WINDOW *wnd, int intern)
{
int area=ALL;
if(verify_wnd(&wnd))
	{
	while(area)
		{
		WCOLOR[--area] &=  ~BRIGHT;
		WCOLOR[area]  |= intern;
		}
	redraw(wnd);
	}
}

/*---- Set title ----*/
void set_title(WINDOW *wnd,char *title)
{
if(verify_wnd(&wnd))
	{
	WTITLE=title;
	redraw(wnd);
	}
}


/*---- redraw a window ----*/
redraw(WINDOW *wnd)
{
#ifndef FASTWINDOWS
int x,y,chat,atr;
for(y=1; y<HEIGHT-1;y++)
	for(x=1; x<WIDTH-1;x++)
		{
		chat=dget(wnd,x,y);
		atr=(((chat >> 8) & 255) == PNORMAL ? WNORMAL : WACCENT);
		displ(wnd,x,y,chat & 255, atr);
		}
wframe(wnd);
#endif
PNORMAL = WNORMAL;
}

void display_window(WINDOW *wnd)
{
if(verify_wnd(&wnd) && !VISIBLE)
	{
	VISIBLE=1;

#ifdef FASTWINDOWS

	if(HIDDEN)
		{
		HIDDEN = 0;
		vrstr(wnd);
		}
	else
		{
		vsave(wnd);
		clear_window(wnd);
		wframe(wnd);
		}
#else

	vswap(wnd);

#endif
	}
}


/*---- Close all windows ----*/
void close_all()
{
WINDOW *sav, *wnd=listtail;
while(wnd)
	{
	sav=PREV;
	delete_window(wnd);
	wnd=sav;
	}
}

/*--- Delete a window ----*/
void delete_window(WINDOW *wnd)
{
if(verify_wnd(&wnd))
	{
	hide_window(wnd);
	free(SAV);
	remove_list(wnd);
	free(wnd);
	}
}

/*---- Hide a window ----*/
void hide_window(WINDOW *wnd)
{
if(verify_wnd(&wnd) && VISIBLE)
	{
#ifndef FASTWINDOWS
	vswap(wnd);
#else
	vrstr(wnd);
#endif
	HIDDEN=1;
	VISIBLE=0;
	}
}

#ifndef FASTWINDOWS
/*---- reposition window in 3-axis plane ----*/
void repos_wnd(WINDOW *wnd,int x,int y,int z)
{
WINDOW *twnd;
int x1,y1,chat;
if(!verify_wnd(&wnd))
	return;
twnd=establish_window(x+COL,y+ROW,HEIGHT,WIDTH);
twnd->_tl=WTITLE;
twnd->btype=BTYPE;
twnd->wcolor[BORDER]=WBORDER;
twnd->wcolor[TITLE]=WTITLEC;
twnd->wcolor[ACCENT]=WACCENT;
twnd->wcolor[NORMAL]=WNORMAL;
twnd->_wsp=SCROLL;
twnd->_cr=WCURS;
if(z!=1)
	{
	remove_list(twnd);
	if(z==0)
		insert_list(twnd,wnd);
	else
		beg_list(twnd);
	}
for(y1=0;y1 < twnd->_wh; y1++)
	for(x1=0;x1 < twnd->_ww; x1++)
		{
		chat=dget(wnd,x1,y1);
		displ(twnd,x1,y1,chat & 255,(chat >> 8) & 255);
		}
twnd->_wv=1;
vswap(twnd);
hide_window(wnd);
free(SAV);
remove_list(wnd);
*wnd=*twnd;
insert_list(wnd,twnd);
remove_list(twnd);
free(twnd);
}
#endif

/*---- Clear the window ----*/
void clear_window(WINDOW *wnd)
{
register int x1,y1;
if(verify_wnd(&wnd))
	for(y1=1;y1<HEIGHT-1;y1++)
		for(x1=1;x1<WIDTH-1;x1++)
			displ(wnd,x1,y1,' ',WNORMAL);
}

/*---- Draw the window frame ---*/
wframe(WINDOW *wnd)
{
register int x1,y1;
if(!verify_wnd(&wnd))
	return;
/*------ Window Title ----------*/
displ(wnd,0,0,NW,WBORDER);
dtitle(wnd);
displ(wnd,WIDTH-1,0,NE,WBORDER);
/*------ Window sides ----------*/
for(y1=1;y1<HEIGHT-1;y1++)
	{
	displ(wnd,0,y1,SIDE,WBORDER);
	displ(wnd,WIDTH-1,y1,SIDE,WBORDER);
	}
/*------ Bottom of window ------*/
displ(wnd,0,y1,SW,WBORDER);
for(x1=1;x1<WIDTH-1;x1++)
	displ(wnd,x1,y1,LINE,WBORDER);
displ(wnd,x1,y1,SE,WBORDER);
}

/*---- Display the window title ---*/
static dtitle(WINDOW *wnd)
{
int x1=1,i,ln;
char *s=WTITLE;

if(!verify_wnd(&wnd))
	return;
if(s)
	{
	ln=strlen(s);
	if(ln>WIDTH-2)
		i=0;
	else
		i=((WIDTH-2-ln)/2);
   if(i>0)
		while(i--)
			displ(wnd,x1++,0,LINE,WBORDER);
	while(*s && x1 < WIDTH-1)
		displ(wnd,x1++,0,*s++,WTITLEC);
	}
while(x1<WIDTH-1)
	displ(wnd,x1++,0,LINE,WBORDER);
}

/*----- Window Printf ------*/
void wprintf(WINDOW *wnd,char *ln,...)
{
char dlin[100],*dl=dlin;
if(verify_wnd(&wnd))
	{
	va_list ap;
	va_start(ap,ln);
	vsprintf(dlin,ln,ap);
	va_end(ap);
	while(*dl)
		wputchar(wnd,*dl++);
	}
}

/*---- Write a char to the window -----*/
void wputchar(WINDOW *wnd,int c)
{
if(!verify_wnd(&wnd))
	return;
switch(c)
	{
	case '\n':
		if(SCROLL==HEIGHT-3)
			scroll(wnd,UP);
		else
			SCROLL++;
		WCURS=0;
		break;
	case '\t':
		do displ(wnd,(WCURS++)+3,SCROLL+1,' ',WNORMAL);
			while((WCURS%TABS)&&(WCURS+1)<WIDTH-1);
		break;
	default:
		if((WCURS+1)<WIDTH-1)
			{
			displ(wnd,WCURS+1,SCROLL+1,c,WNORMAL);
			WCURS++;
			}
		break;
	}
}

/*---- Set window cursor ----*/
void wcursor(WINDOW *wnd,int x,int y)
{
if(verify_wnd(&wnd) && x < WIDTH-1 && y < HEIGHT-1)
	{
	WCURS=x;
	SCROLL=y;
	cursor(COL+x+1,ROW+y+1);
	}
}

/*----- Pick list -----*/
int get_selection(WINDOW *wnd,int s, char *keys)
{
int c=0,ky;
if(!verify_wnd(&wnd))
	return 0;
SELECT=s;
while(c!=ESC && c != '\r')
	{
	accent(wnd);
	c=get_char();
	deaccent(wnd);
	switch(c)
		{
		case BS:
		case UP:
			if(SELECT > 1)
				SELECT--;
			else
				SELECT = SCROLL+1;
			break;
		case FWD:
		case DN:
			if(SELECT < SCROLL+1)
  				SELECT++;
			else
				SELECT = 1;
			break;
	  	case '\r':
		case  ESC:
			break;
		default:
			if(keys)
				{
				ky=0;
				while(*(keys+ky))
					{
					if(*(keys+ky)==toupper(c) || *(keys+ky)==tolower(c))
						return ky+1;
					ky++;
					}
				}
			break;

		}
 	}
return c== '\r' ? SELECT : c == ESC ? 0 : c;
}

/*---- Scroll a window -----*/
union REGS rg;

void scroll(WINDOW *wnd,int dir)
{
int row = HEIGHT-1,col,chat;
if(!verify_wnd(&wnd))
	return;
if(NEXT==NULL && HEIGHT > 3 && VISIBLE)
	{
	rg.h.ah=dir == UP ? 6 : 7;
	rg.h.al=1;
	rg.h.bh=WNORMAL;
	rg.h.cl=COL+1;
	rg.h.ch=ROW+1;
	rg.h.dl=COL + WIDTH - 2;
	rg.h.dh=ROW + HEIGHT - 2;
	int86(0x10,&rg,&rg);
	return;
	}
if(dir == UP)
	{
	for(row=2;row<HEIGHT-1;row++)
		for(col=1;col<WIDTH-1;col++)
			{
			chat=dget(wnd,col,row);
			displ(wnd,col,row-1,chat & 255,(chat >> 8) & 255);
			}
	for(col=1;col<WIDTH-1;col++)
		displ(wnd,col,row-1,' ',WNORMAL);
	}
else
	{
	for(row=HEIGHT-2;row>1;--row)
		for(col=1;col<WIDTH-1;col++)
			{
			chat=dget(wnd,col,row-1);
			displ(wnd,col,row,chat&255,(chat>>8)&255);
			}
	for(col=1;col<WIDTH-1;col++)
		displ(wnd,col,row,' ',WNORMAL);
	}
}

#ifndef FASTWINDOWS
/*----- Compute the adress of a window's display char ----*/
static int *waddr(WINDOW *wnd,int x,int y)
{
WINDOW *nxt = NEXT;
int *vp;
if(!VISIBLE)
	return(int *)(SAV+y*(WIDTH*2)+x*2);
x += COL;
y += ROW;
while(nxt)
	{
	if(nxt->_wv)
		if(x >= nxt->_wx && x <= nxt->_wx + nxt->_ww-1)
			if(y >= nxt->_wy && y <= nxt->_wy + nxt->_wh-1)
				{
				x-=nxt->_wx;
				y-=nxt->_wy;
				vp=(int *)((nxt->_ws)+y*(nxt->_ww*2)+x*2);
				return vp;
				}
	nxt=nxt->_nx;
	}
return NULL;
}

/*----- display a char to window ----*/
void displ(WINDOW *wnd,int x,int y,int ch,int at)
{
int *vp;
int vch=(ch&255)|(at<<8);
if((vp=waddr(wnd,x,y))!=NULL)
	*vp=vch;
else
	vpoke(VSG,vad(x+COL,y+ROW),vch);
}

/*----- Get a displayed char from the window -----*/
static int dget(WINDOW *wnd,int x,int y)
{
int *vp;
if((vp=waddr(wnd,x,y))!=NULL)
	return *vp;
return vpeek(VSG,vad(x+COL,y+ROW));
}

/*==============------- Low level routines --------=============*/

/*--------- swap the video buff with the save buff -----*/
static vswap(WINDOW *wnd)
{
int x,y,chat;
int *bf = (int *) SAV;
for(y=0;y<HEIGHT;y++)
	for(x=0;x<WIDTH;x++)
		{
		chat=*bf;
		*bf++=dget(wnd,x,y);
		displ(wnd,x,y,chat&255,(chat>>8)&255);
		}
}

#else

/*---- Save video mem into the save buff ----*/
static vsave(WINDOW *wnd)
{
int x,y;
int *bf=(int *)SAV;
for(y=0;y<HEIGHT;y++)
	for(x=0;x<WIDTH;x++)
   	*bf++=vpeek(VSG,vad(x+COL,y+ROW));
}

/*----- restore video mem from save buff -----*/
static vrstr(WINDOW *wnd)
{
int x,y;
int *bf=(int *)SAV;
for(y=0;y<HEIGHT;y++)
	for(x=0;x<WIDTH;x++)
   	vpoke(VSG,vad(x+COL,y+ROW),*bf++);
}

#endif

/*---- (de)accent the line where SELECT points ----*/
void acline(WINDOW *wnd,int set)
{
int x,ch;
if(!verify_wnd(&wnd))
	return;
for(x=1;x<WIDTH-1;x++)
	{
	ch=dget(wnd,x,SELECT)&255;
	displ(wnd,x,SELECT,ch,set);
	}
}

/*===============---- Linked list funcs ----================*/

/*-------------- Add a window to the end of the list --------*/
static add_list(WINDOW *wnd)
{
if(listtail)
	{
	PREV=listtail;
	listtail->_nx=wnd;
	}
listtail=wnd;
if(!listhead)
	listhead=wnd;
}

/*----- Add a window to the begining of the list ---------*/
static beg_list(WINDOW *wnd)
{
if(listhead)
	{
	NEXT = listhead;
	listhead->_pv=wnd;
	}
listhead=wnd;
if(!listtail)
	listtail=wnd;
}

/*----- Remove a window from the list ------*/
static remove_list(WINDOW *wnd)
{
if(NEXT)
	NEXT->_pv=PREV;
if(PREV)
	PREV->_nx=NEXT;
if(listhead==wnd)
	listhead=NEXT;
if(listtail==wnd)
	listtail=PREV;
NEXT=PREV=NULL;
}

/*------ insert w1 after w2 ------*/
static insert_list(WINDOW *w1,WINDOW *w2)
{
w1->_pv=w2;
w1->_nx=w2->_nx;
w2->_nx=w1;
if(w1->_nx==NULL)
	listtail=w1;
else
	w1->_nx->_pv=w1;
}

#ifndef FASTWINDOWS
/*----- verify the presence of the window ------*/
static verify_wnd(WINDOW **w1)
{
WINDOW *wnd;
wnd=listhead;
if(*w1==NULL)
	*w1=listtail;
else
	{
	while(wnd!=NULL)
		{
		if(*w1==wnd)
			break;
		wnd=NEXT;
		}
	}
return wnd!=NULL;
}
#endif


/*-------- Error Msgs. -------*/
WINDOW *ewnd=NULL;
void error_message(char *s)
{
ewnd=establish_window(50,22,3,max(10,strlen(s)+2));
set_colors(ewnd,ALL,RED,YELLOW,BRIGHT);
set_title(ewnd,"[> ERROR! <]");
display_window(ewnd);
wprintf(ewnd,s);
putchar(BELL);
}

void clear_message()
{
if(ewnd)
	delete_window(ewnd);
ewnd=NULL;
}