#include "struct.h"
#include "comm.h"
#include "keys.h"
#include "user.h"
#include <conio.h>
#include <stdio.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <dos.h>


extern int mouseyes,button_num;
extern int useron;
extern struct userfile user;
extern struct configfile config;
extern time_t timeon;
extern timeleft;

union REGS rg;

char s[80];


do_ext(int c)
{
int x,y;

switch(c)
	{
	case 31:
		setscroll();
		break;
	case 59:							/* edit user */
		pullmenu();
		break;
	case 60:							/* view user stats */
		break;
	case 61: 						/* sysop drop to dos */
		todos();
		break;
/*	case 62:       				/* commands */
		curr_cursor(&x,&y);
		cursor(0,25);
		chooselist();
		cursor(x,y);
		break;*/
	case 63:							/* disconnect w/o line noise */
		hangup();
		break;
	case 64:							/* disconnect w/ line noise */
		line_noise();
		hangup();
		break;
	case 65:							/* finger/nuke user */
		break;
	case 66:							/* temp. sysop access */
		break;
	case 67:							/* spurt o' line noise */
		line_noise();
		break;
	case 68:						/* chat w/ user */
		chat();
		put_char(12);
		break;
	}
}

line_noise()
{
int i,c;
i=c=0;

while(c < 20)
	{
	if(c == 2 || c == 9 || c == 15 || c == 17 || c == 20)
		put_char(32);
	i=random(255);
	if(i < 32)
		c--;
	else
		put_char(i);
	c++;
	}
}



dfile(char *si,int i)
{
FILE *fp;
char fo[80],c[900];

if(i==0)
	put_char(12);
setmci(1);
strcpy(fo,config.gfiledir);
strcat(fo,si);
if(access(fo,0)!=0)
	{
	if(i<2)
		{
		nl();
		print("%s not found",fo);
		nl();
		}
	return(-1);
	}
fp=fopen(fo,"r");
while(1)
	{
	if(feof(fp)!=0)
		{
		fclose(fp);
		updatestat();
		return;
		}
	if(kbhit())
		if(getch()==32)
			{
			if(user.menutype > 1)
				{
				nl();
				put2("[0m");
				updatestat();
				}
			setmci(0);
			return;
			}
	if(comm_avail())
		if(comm_getc(1)==32)
			{
			if(user.menutype > 1)
				{
				nl();
				put2("[0m");
				updatestat();
				}
			setmci(0);
			return;
			}
	fgets(c,800,fp);
	put2(c);
	c[0]=0;
	}
fclose(fp);
if(user.menutype > 1)
	put2("[0m");
setmci(1);
updatestat();
}

regdispfile(char *s)
{
dfile(s,1);
}

dispfile(char *s)
{
dfile(s,0);
}

dispnoerr(char *s)
{
dfile(s,2);
}


pausescr()
{
comm_flush();
nl();
ansic(1);
put("[ Hit any key ]");
get_c();
put_char(12);
}


makebox(int right,int top, int height, int width)
{
char s[80];
int i,line;

ansic(4);
ccursor(top,right);
s[0]=0;
s[0]=201;
for(i=0;i < width;i++)
	s[i+1]=205;
s[i]=187;
s[i+1]=0;
put(s);
line=0;
while(line < (height)-1)
	{
	ccursor(top+line+1,right);
	s[0]=186;
	for(i=0;i < width;i++)
		s[i+1]=32;
	s[i]=186;
	s[i+1]=0;
	put(s);
	line++;
	}
ccursor(top+height,right);
s[0]=200;
for(i=0;i < width;i++)
	s[i+1]=205;
s[i]=188;
s[i+1]=0;
put(s);
}

title(char *list)
{
char s[80];
int i,line,len,width;

line=strlen(list);

if(line%2==0)
	width=30;
else
	width=29;


len=(width-line)/2;

ansic(6);
s[0]=0;
s[0]=214;                  /* 218 */
for(i=0;i < width;i++)
	s[i+1]=196;
s[i]=183;						/* 191 */
s[i+1]=0;
pl(s);
s[0]=186;						/* 179 */
for(i=0;i < len;i++)
	s[i+1]=32;
s[i]=0;
strcat(s,"7");
strcat(s,list);
strcat(s,"6");
put(s);
for(i=0;i < len;i++)
	s[i]=32;
s[i]=186;						/* 179 */
s[i+1]=0;
pl(s);
s[0]=211;						/* 192 */
for(i=0;i < width;i++)
	s[i+1]=196;
s[i]=189;     					/* 217 */
s[i+1]=0;
pl(s);
nl();
}


ccursor(int top,int right)
{
char s[80];
int i;

i=0;
if(useron)
	{
	sprintf(s,"[%d;%dH",top,right);
	while(s[i])
		{
		comm_putc(s[i]);
		i++;
		}
	}
if(top==24)
	top=22;
if(top==25)
	top=23;
gotoxy(right,top);
}

colorstr(int i)
{
char s[80];
ansic(i);
s[0]=0;


if(user.colors[i-1].hilit)
	strcat(s,"Hilighted ");
if(user.colors[i-1].blink)
	strcat(s,"Blinking ");

switch(user.colors[i-1].fore)
	{
	case 0:
		strcat(s,"Black ");
		break;
	case 1:
		strcat(s,"Blue ");
		break;
	case 2:
		strcat(s,"Green ");
		break;
	case 3:
		strcat(s,"Cyan ");
		break;
	case 4:
		strcat(s,"Red ");
		break;
	case 5:
		strcat(s,"Magenta ");
		break;
	case 6:
		if(user.colors[i-1].hilit)
			strcat(s,"Yellow ");
		else
			strcat(s,"Red ");
		break;
	case 7:
		strcat(s,"White ");
		break;
	}
strcat(s,"on ");
switch(user.colors[i-1].back)
	{
	case 0:
		strcat(s,"Black");
		break;
	case 1:
		strcat(s,"Blue");
		break;
	case 2:
		strcat(s,"Green");
		break;
	case 3:
		strcat(s,"Cyan");
		break;
	case 4:
		strcat(s,"Red");
		break;
	case 5:
		strcat(s,"Magenta");
		break;
	case 6:
		strcat(s,"Red");
		break;
	case 7:
		strcat(s,"White");
		break;
	}
put(s);
nl();
}



ansic(int i)
{
char s[80];
int bk,fn;

if(user.menutype == 1)
	return;
strcpy(s,"[");
switch(user.colors[i-1].fore)
	{
	case 0:
		strcat(s,"30");
		fn=0;
		break;
	case 1:
		strcat(s,"34");
		fn=1;
		break;
	case 2:
		strcat(s,"32");
		fn=2;
		break;
	case 3:
		strcat(s,"36");
		fn=3;
		break;
	case 4:
		strcat(s,"31");
		fn=4;
		break;
	case 5:
		strcat(s,"35");
		fn=5;
		break;
	case 6:
		strcat(s,"33");
		fn=6;
		break;
	case 7:
		strcat(s,"37");
		fn=7;
		break;
	}
strcat(s,";");
switch(user.colors[i-1].back)
	{
	case 0:
		strcat(s,"40");
		bk=0;
		break;
	case 1:
		strcat(s,"44");
		bk=1;
		break;
	case 2:
		strcat(s,"42");
		bk=2;
		break;
	case 3:
		strcat(s,"46");
		bk=3;
		break;
	case 4:
		strcat(s,"41");
		bk=4;
		break;
	case 5:
		strcat(s,"45");
		bk=5;
		break;
	case 6:
		strcat(s,"43");
		bk=6;
		break;
	case 7:
		strcat(s,"47");
		bk=7;
		break;
	}
if(user.colors[i-1].hilit)
	{
	strcat(s,";1");
	fn+=8;
	}
else
	strcat(s,";0");
if(user.colors[i-1].blink)
	{
	strcat(s,";5");
	fn+=128;
	}
strcat(s,"m");
putremote(s);
textcolor(fn);
textbackground(bk);
}


todos()
{
/*char scr[4000];*/
int x,y;

ondos();
x=wherex();
y=wherey();
/*gettext(1,1,80,25,scr);
clear_screen();
system("");
puttext(1,1,80,25,scr);*/
clear_screen();
system("");
clear_screen();
gotoxy(x,y);
offdos();
}

onwait()
{
ansic(3);
put("[ Please Wait ]");
}

offwait()
{
int i;

for(i=0;i < 15;i++)
	put("\b");
for(i=0;i < 15;i++)
	put(" ");
for(i=0;i < 15;i++)
	put("\b");
comm_flush();
ansic(2);
}

ondos()
{
ansic(3);
put("[ Sysop went to DOS ]");
}

offdos()
{
int i;

for(i=0;i < 21;i++)
	put("\b");
for(i=0;i < 21;i++)
	put(" ");
for(i=0;i < 21;i++)
	put("\b");
comm_flush();
ansic(2);
}




chat()
{
int c,kh,ch,x,y;

c=0;
ch=0;
kh=0;
put_char(12);
print("-*[ Force Ten Chat ]*-  %s Chatting with %s #%d",config.sysop,user.name,user.number);
nl();
nl();
nl();
if(!useron)
	{
	nl();
	pl(">>> No user on-line");
	nl();
	}
comm_flush();
while(1)
	{
	if(kbhit())
		{
		kh=1;
		c=getch();
		if(c==0)
			{
			c=getch();
			if(c==68)
				return;
			}
/*		if(c==F1)
			{
			curr_cursor(&x,&y);
			c=kh=0;
			pullmenu();
			cursor(x,y);
			}*/
		if(c==13)
			{
			nl();
			kh=c=0;
			}
		if(c==8)
			{
			bs();
			kh=c=0;
			}
		}
	if(comm_avail())
		{
		ch=1;
		c=comm_getc(1);
		if(c==13)
			{
			nl();
			ch=c=0;
			}
		if(c==8)
			{
			bs();
			ch=c=0;
			}
		}
	if(ch || kh)
		{
		if(kh)
			ansic(1);
		else
			ansic(2);
		ch=kh=0;
		put_char(c);
		}
	}
}


pullmenu()
{
char scr[1200],s[80];
int x,y,i,goon,c;

goon=1;
onwait();
x=wherex();
y=wherey();
hidemouse();
gettext(13,5,61,16,scr);
puttext(13,5,61,16,USERED);
showmouse();
while(goon)
	{
	gotoxy(23,12);
	cprintf(" ");
	gotoxy(23,12);
	textcolor(LIGHTCYAN);
	inputlocal(s,30);
	strupr(s);
	switch(s[0])
		{
		case 'Q':
			goon=0;
			break;
		case '1':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old Name: ");
			textcolor(LIGHTGRAY);
			cprintf("%s",user.name);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New Name: ");
			inputlocal(s,40);
			gotoxy(15,14);
			cprintf("                      ");
			gotoxy(15,15);
			cprintf("                      ");
			if(s[0]!=0 || stricmp(user.name,s)==0)
				{
				if(!ver_user(s))
					strcpy(user.name,s);
				}
			break;
		case '2':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old User SL: ");
			textcolor(LIGHTGRAY);
			cprintf("%d",user.sl);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New User SL: ");
			textcolor(LIGHTGRAY);
			inputlocal(s,20);
			gotoxy(15,14);
			cprintf("                        ");
			gotoxy(15,15);
			cprintf("                        ");
			i=atoi(s);
			if(i >= 0 || i <= 255)
				user.sl=i;
			break;
		case '3':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old File SL: ");
			textcolor(LIGHTGRAY);
			cprintf("%d",user.filesl);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New File SL: ");
			textcolor(LIGHTGRAY);
			inputlocal(s,20);
			gotoxy(15,14);
			cprintf("                        ");
			gotoxy(15,15);
			cprintf("                        ");
			i=atoi(s);
			if(i >= 0 || i <= 255)
				user.filesl=i;
			break;
		case '4':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old Note: ");
			textcolor(LIGHTGRAY);
			cprintf("\"%s\"",user.note);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New Note: ");
			textcolor(LIGHTGRAY);
			inputlocal(s,40);
			gotoxy(15,14);
			cprintf("                                      ");
			gotoxy(15,15);
			cprintf("                                      ");
			if(s[0]!=0)
				strcpy(user.note,s);
			break;
		case '5':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old Time: ");
			textcolor(LIGHTGRAY);
			cprintf("%d",print_time()/60);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New Time: ");
			textcolor(LIGHTGRAY);
			inputlocal(s,20);
			gotoxy(15,14);
			cprintf("                        ");
			gotoxy(15,15);
			cprintf("                        ");
			if(s[0]!=0)
				{
				user.timeall=atoi(s)*60;
				timeleft=atoi(s)*60;
				}
			break;
		case '6':
			i=1;
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("User Flags: ");
			while(i)
				{
				goon=0;
				while(goon < 10)
					{
					gotoxy(27+goon,14);
					if(user.flags[goon])
						{
						textcolor(LIGHTRED);
						cprintf("%d",goon);
						}
					else
						{
						textcolor(LIGHTGRAY);
						cprintf("%d",goon);
						}
					goon++;
					}
				gotoxy(15,15);
				textcolor(LIGHTGREEN);
				cprintf("Enter flag to toggle [Q/Quit]: ");
				do
					{
					c=getch();
					if(c=='Q' || c=='q')
						{
						gotoxy(15,14);
						cprintf("                         ");
						gotoxy(15,15);
						cprintf("                                 ");
						i=0;
						}
					} while(c < '0' || c > '10');
				gotoxy(37,15);
				cprintf("%c",c);
				if(user.flags[c-48])
					user.flags[c-48]=0;
				else
					user.flags[c-48]=1;
				gotoxy(37,15);
				cprintf(" ");
				}
			break;
		case '7':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old Password: ");
			textcolor(LIGHTGRAY);
			cprintf("%s",user.pass);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New Password: ");
			inputlocal(s,20);
			gotoxy(15,14);
			cprintf("                       ");
			gotoxy(15,15);
			cprintf("                       ");
			if(s[0]!=0)
				strcpy(user.pass,s);
			break;
		case '8':
			gotoxy(15,14);
			textcolor(LIGHTBLUE);
			cprintf("Old Filepoints: ");
			textcolor(LIGHTGRAY);
			cprintf("%d",user.filepoints);
			gotoxy(15,15);
			textcolor(LIGHTGREEN);
			cprintf("New Filepoints: ");
			inputlocal(s,40);
			gotoxy(15,14);
			cprintf("                     ");
			gotoxy(15,15);
			cprintf("                     ");
			if(s[0]!=0)
				user.filepoints=atoi(s);
			break;

		}
	}
writeuser();
puttext(13,5,61,16,scr);
gotoxy(x,y);
offwait();
}

ifspace()
{
if(kbhit())
	if(getch()==32)
		return(1);
if(comm_avail())
	if(comm_getc(1)==32)
		return(1);
return(0);
}


do_help(char *fn)
{
char s[80];

sprintf(s,"%s%s.ANS",config.gfiledir,fn);
if(access(s,0)!=-1 && user.menutype!=1)
	{
	sprintf(s,"%s.ANS",fn);
	dispfile(s);
	}

sprintf(s,"%s%s.ASC",config.gfiledir,fn);
if(access(s,0)!=-1 && user.menutype!=1)
	{
	sprintf(s,"%s.ASC",fn);
	dispfile(s);
	}

}

checkformouse()
{
rg.x.ax=0x0000;

int86(0x33,&rg,&rg);

if(rg.x.ax==0)
	return(0);

switch(rg.x.bx)
	{
	case -1:
		button_num=2;
		break;
	case 3:
		button_num=3;
		break;
	default:
		button_num=2;
		break;
	}

return(1);
}

checkbutton()
{
int bs,i;

rg.x.ax=0x0003;
int86(0x33,&rg,&rg);

bs=rg.x.bx;

if(bs==0)
	return;

switch(bs)
	{
	case 1:
		pullmenu();
		break;
	case 2:
		printf("R");
		break;
	case 4:
		chat();
		break;
	}
}

hidemouse()
{
if(!mouseyes)
	return;

rg.x.ax=0x0002;
int86(0x33,&rg,&rg);
}

showmouse()
{
if(!mouseyes)
	return;

rg.x.ax=0x0001;
int86(0x33,&rg,&rg);
}



