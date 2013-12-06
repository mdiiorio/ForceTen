/*-----------  Tmenu.C ---------------*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "keys.h"
#include "twindow.h"

extern int VSG;

WINDOW *open_menu(char *mnm,MENU *mn,int hsel);
int gethmenu(MENU *mn,WINDOW *hmenu,int hsel);
int getvmn(MENU *mn,WINDOW *hmenu,int *hsel,int vsel);
int haccent(MENU *mn,WINDOW *hmenu,int hsle,int vsel);
void dimension(char *sl[],int *ht,int *wd);
void light(MENU *mn,WINDOW *hmenu,int hsel,int d);

void menu_select(char *name,MENU *mn)
{
WINDOW *open_menu();
WINDOW *hmenu;
int sx,sy;
int hsel=1,vsel;

curr_cursor(&sx,&sy);
cursor(0,26);
hmenu=open_menu(name,mn,hsel);
while(hsel=gethmenu(mn,hmenu,hsel))
	{
	vsel=1;
	while(vsel=getvmn(mn,hmenu,&hsel,vsel))
		{
		delete_window(hmenu);
		(*(mn+hsel-1)->func[vsel-1])(hsel,vsel);
		hmenu=open_menu(name,mn,hsel);
		}
	}
delete_window(hmenu);
cursor(sx,sy);
}

static WINDOW *open_menu(char *mnm,MENU *mn,int hsel)
{
int i=0;
WINDOW *hmenu;
hmenu=establish_window(0,0,3,80);
set_title(hmenu,mnm);
set_colors(hmenu,ALL,BLUE,AQUA,BRIGHT);
set_colors(hmenu,ACCENT,BLACK,YELLOW,BRIGHT);
display_window(hmenu);
while((mn+i)->mname)
	wprintf(hmenu," %-10.10s ",(mn+i++)->mname);
light(mn,hmenu,hsel,1);
cursor(0,26);
return(hmenu);
}

static int gethmenu(MENU *mn,WINDOW *hmenu,int hsel)
{
int sel;

light(mn,hmenu,hsel,1);

while(TRUE)
	{
	switch(sel=get_char())
		{
		case FWD:
		case BS:
			hsel=haccent(mn,hmenu,hsel,sel);
			break;
		case ESC:
			return(0);
		case '\r':
		case DN:
			return(hsel);
		default:
			putchar(BELL);
			break;
		}
	}
}

static int getv(MENU *mn,WINDOW *hmenu, int *hsel,int vsel)
{
WINDOW *vmenu;
int ht=10,wd=20;
char **mp

while(1)
	{
	dimension((mn+*hsel-1)->mselcs,&ht,&wd);
	vmenu=establish_window(2+(*hsel-1)*12,2,ht,wd);
	set_colors(vmenu,ALL,BLUE,AQUA,BRIGHT);
	set_colors(vmenu,ACCENT,BLACK,YELLOW,BRIGHT);
	set_border(vmenu,4);
	wprintf(vmenu," User SL\n ");
	wprintf(vmenu," User File SL\n ");
	wprintf(vmenu," User Name\n ");
	wprintf(vmenu," User Note\n ");
	wprintf(vmenu," Time Left\n ");
	wprintf(vmenu," Filepoints\n ");
	wprintf(vmenu," Time Bank\n ");
	wprintf(vmenu," Delete\n ");
	display_window(vmenu);



static int getvmn(MENU *mn,WINDOW *hmenu, int *hsel,int vsel)
{
WINDOW *vmenu;
int ht=10,wd=20;
char **mp;



while(1)
	{
	dimension((mn+*hsel-1)->mselcs,&ht,&wd);
	vmenu=establish_window(2+(*hsel-1)*12,2,ht,wd);
	set_colors(vmenu,ALL,BLUE,AQUA,BRIGHT);
	set_colors(vmenu,ACCENT,BLACK,YELLOW,BRIGHT);
	set_border(vmenu,4);
	display_window(vmenu);
	mp=(mn+*hsel-1)->mselcs;
	while(*mp)
		wprintf(vmenu,"\n%s",*mp++);
	vsel=get_selection(vmenu,vsel,"");
	delete_window(vmenu);
	if(vsel==FWD || vsel==BS)
		{
		*hsel=haccent(mn,hmenu,*hsel,vsel);
		vsel=1;
		}
	else
		return(vsel);
	}
}

static int haccent(MENU *mn,WINDOW *hmenu,int hsel,int sel)
{
switch(sel)
	{
	case FWD:
		light(mn,hmenu,hsel,0);
		if((mn+hsel)->mname)
			hsel++;
		else
			hsel=1;
		light(mn,hmenu,hsel,1);
		break;
	case BS:
		light(mn,hmenu,hsel,0);
		if(hsel == 1)
			while((mn+hsel)->mname)
				hsel++;
		else
			--hsel;
		light(mn,hmenu,hsel,1);
		break;
	default:
		break;
	}
return(hsel);
}

static void dimension(char *sl[],int *ht,int *wd)
{
unsigned strlen(char *);

*ht=*wd=0;
while(sl[*ht])
	{
	*wd=max(*wd,strlen(sl[*ht]));
	(*ht)++;
	}
*ht += 2;
*wd += 2;
}

static void light(MENU *mn,WINDOW *hmenu,int hsel,int d)
{
if(d)
	reverse_video(hmenu);
wcursor(hmenu,(hsel-1)*12+2,0);
wprintf(hmenu,(mn+hsel-1)->mname);
normal_video(hmenu);
cursor(0,26);
}

