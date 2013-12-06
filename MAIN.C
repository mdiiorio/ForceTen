#include <ctype.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <conio.h>
#include <process.h>
#include <stdio.h>
#include <time.h>
#include "keys.h"
#include "struct.h"
#include "vars.h"
#include "comm.h"
#include "wait.h"

extern int timeleft, useron;
struct configfile config;
struct userfile user;
struct filestruct fstruct;
struct fileindex fileidx[30];
struct confindex cidx[10];
struct bque que;
struct boardindex bidx[30];
struct headerinfo hdr;
char result[80];
int count,ctrlbreak,days,min;
char cy[20] = "ForTen Version";
int pos,way,yesno,exitafter;
int mouseyes,button_num;


main(int argc, char *argv[])
{
/*ctrlbreak=getcbrk();
setcbrk(0);*/

exitafter=0;

if(argc > 1)
	if(stricmp(argv[1],"/X")==0)
		exitafter=1;

readfiles();
wfc();
}

gotowfc()
{
hidemouse();
comm_close();
printf("[0m");
textcolor(LIGHTGRAY);
textbackground(BLACK);
if(exitafter)
	exit(255);
readfiles();
wfc();
}



ckmdm()
{
int n;

if(comm_avail())
	{
	result[count]=comm_getc(1);
	if(isdigit(result[count]))
		{
		count++;
		return(0);
		}
	if(result[count]==13)
		{
		result[count]=0;
		return(1);
		}
	}
}


exit_bbs(int errorlev)
{
printf("[0m");
comm_close();
/*setcbrk(ctrlbreak);*/
clear_screen();
exit(errorlev);
}


readfiles()
{
if(access("CONFIG.BBS",0)==-1)
	{
	printf("\n\nCONFIG.BBS does not exist, run INIT.EXE\n\n");
	exit_bbs(100);
	}
else
	{
	configp=open("CONFIG.BBS",O_BINARY | O_RDWR);
	read(configp,&config,sizeof(struct configfile));
	close(configp);
	}
if(access("USER.BBS",0)==-1)
    inituser();
}

writeconfig()
{
configp=open("CONFIG.BBS",O_BINARY | O_RDWR);
write(configp,&config,sizeof(struct configfile));
close(configp);
}


init_modem()
{
int n;
gotoxy(45,21); printf("[33mInit   ");
cursor(0,25);
pr1(config.string);
n=chk_modem();
gotoxy(45,21);
if(n==4)
	printf("[33mError  ");
else
	printf("[33mWaiting");
cursor(0,25);
comm_flush();
}


get_user(char *s)
{
int smallp,yes=0,goon=1;
userp=open("USER.BBS",O_RDWR | O_BINARY);
while(goon)
	{
	if(eof(userp))
		goon=0;
	read(userp,&user,sizeof(struct userfile));
	if(stricmp(user.name,s)==0)
		{
		yes=1;
		goon=0;
		}
	if(atoi(s) == user.number)
		{
		yes=1;
		goon=0;
		}
	}
if(!yes)
	{
	close(userp);
	return(0);
	}
close(userp);
return(1);
}

ver_user(char *s)
{
int smallp,yes=0,goon=1;
struct userfile tempuser;
userp=open("USER.BBS",O_RDWR | O_BINARY);
while(goon)
	{
	if(eof(userp))
		goon=0;
	read(userp,&tempuser,sizeof(struct userfile));
	if(stricmp(tempuser.name,s)==0)
		{
		yes=1;
		goon=0;
		}
	if(atoi(s) == tempuser.number)
		{
		yes=1;
		goon=0;
		}
	}
if(!yes)
	{
	close(userp);
	return(0);
	}
close(userp);
return(1);
}

readuser(char *s,struct userfile *buf)
{
int smallp,yes=0,goon=1;
userp=open("USER.BBS",O_RDWR | O_BINARY);
while(goon)
	{
	if(eof(userp))
		goon=0;
	read(userp,buf,sizeof(struct userfile));
	if(stricmp(buf->name,s)==0)
		{
		yes=1;
		goon=0;
		}
	if(atoi(s) == buf->number)
		{
		yes=1;
		goon=0;
		}
	}
if(!yes)
	{
	close(userp);
	return(0);
	}
close(userp);
return(1);
}


writeuser()
{
userp=open("USER.BBS",O_RDWR | O_BINARY);
posclr(user.number);
write(userp,&user,sizeof(struct userfile));
close(userp);
}

writeu(struct userfile *buf)
{
userp=open("USER.BBS",O_RDWR | O_BINARY);
posclr(buf->number);
write(userp,buf,sizeof(struct userfile));
close(userp);
}


posclr(int n)
{
long o;
o = 0L + (n-1);
o *= sizeof(struct userfile);
lseek(userp,o,SEEK_SET);
}

inituser()
{
int con,i;
con=open("USER.BBS",O_RDWR | O_BINARY | O_CREAT,S_IWRITE);
user.number=1;
strcpy(user.name,config.sysop);
strcpy(user.realname,"John Doe");
strcpy(user.pass,"Sysop");
strcpy(user.note,"SysOp");
strcpy(user.macro,"]> ");
strcat(user.macro,config.sysop);
strcat(user.macro," <[");
strcpy(user.phone,"(404)000-0000");
user.sl=255;
user.filesl=255;
user.gfilesl=255;
i=0;
while(i<11)
	{
	user.flags[i]=1;
	i++;
	}
user.menutype=3;
user.filepoints=10;
user.exempt=5;
user.waiting=0;
user.fullscreen=1;
user.lines=24;
user.hacks=0;
user.timeall=4200;
user.timeon=0;
user.timebank=0;
user.logons=0;
user.del=0;
user.msgpost=0;
user.emailsent=0;
user.k_dl=0;
user.k_ul=0;
user.num_dl=0;
user.num_ul=0;
user.firston.da_year=1990;
user.firston.da_mon=1;
user.firston.da_day=1;
user.laston.da_year=1990;
user.laston.da_mon=1;
user.laston.da_day=1;
user.colors[0].fore=2;
user.colors[0].back=0;
user.colors[0].hilit=1;
user.colors[0].blink=0;
user.colors[1].fore=6;
user.colors[1].back=0;
user.colors[1].hilit=1;
user.colors[1].blink=0;
user.colors[2].fore=3;
user.colors[2].back=0;
user.colors[2].hilit=1;
user.colors[2].blink=0;
user.colors[3].fore=7;
user.colors[3].back=0;
user.colors[3].hilit=0;
user.colors[3].blink=0;

write(con,&user,sizeof(struct userfile));
close(con);
}

/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³  Date: 4/12/90  ³ (35)    ® Land of Phiendz ¯           ³  Time: 10:00 pm  ³
ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
³ Serial Number: 000001             ³ Total Days Up      : 1                 ³
³ Registered to: Sadistic Phiend    ³ Total System Calls : 1                 ³
ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ Total Calls Today  : 1                 ³
³ Last Caller: Sadistic Phiend      ³ Files Online       : 0                 ³
ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ Active Messages    : 0                 ³
³ System Status                     ³ Feedback Waiting   : 0                 ³
ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ New Uploads        : 0                 ³
³                                   ³ New Messages       : 0                 ³
³ Backup in 60 minutes (11:00 pm)   ³ Total Drive Space  : 50 Megz           ³
³ CyberNet is ON                    ³ Total Space Free   : 45 Megz           ³
³ Chat Time 5:00 pm - 11:00 pm      ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
³                                   ³[F1] Exit BBS        [F6] Run Config    ³
³                                   ³[F2] Send Carrier    [F7] Run Terminal  ³
³                                   ³[F3] Read Feedback   [F8] Force Backup  ³
³                                   ³[F4] View SysLog     [F9] Force CyberNet³
³ DSZlog NOT FOUND!                 ³[F5] Toggle Chat     [F10] Local Login  ³
ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
³ CyberThrash Version: 1.00 ³ Modem Status: Initing ³ Chat: Available        ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

printwfcstats()
{
struct dfree dinfo;
unsigned long bperc, total, free;
int i;

days=min=-1;
pos=yesno=0;
way=1;

printf("[1m");
updatetd();
gotoxy(((35-strlen(config.name))/2)+20,2); printf("[35m® [37m%s [35m¯",config.name);
gotoxy(16,7); printf("[37m%s",config.lastcaller);
gotoxy(60,4); printf("[36m1");
gotoxy(60,5); printf("[36m%d",config.calls);
gotoxy(60,6); printf("[36m%d",config.callstoday);
gotoxy(60,7); printf("[36m%d",config.fonline);
gotoxy(60,8); printf("[36m%d",config.messages);
gotoxy(60,9); printf("[36m0");
gotoxy(60,10); printf("[36m%d",config.newuploads);
gotoxy(60,11); printf("[36m%d",config.newmsgs);
i=3;
bperc=total=free=0;
getdfree(i,&dinfo);
while(dinfo.df_sclus != -1)
	{
	bperc=dinfo.df_sclus * dinfo.df_bsec;
	total+=dinfo.df_total * bperc;
	free+=dinfo.df_avail * bperc;
	i++;
	getdfree(i,&dinfo);
	}
gotoxy(60,12); printf("[36m%ld [31mMegs",total/1000000);
gotoxy(60,13); printf("[36m%ld [31mMegs",free/1000000);
gotoxy(19,21); printf("[33m1[37m.[33m00");
gotoxy(45,21); printf("[33mWaiting");
gotoxy(61,21); printf("[31mAvailible");
cursor(0,25);
}


updatetd()
{
struct date datenow;
struct time ti;

getdate(&datenow);

if(days!=datenow.da_day || days==-1)
	{
	gotoxy(10,2);
	days=datenow.da_day;
	printf("[37;1m%02d/%02d/%d ",datenow.da_mon,datenow.da_day,datenow.da_year-1900);
	}
if(days==0)
	days=datenow.da_day;

gettime(&ti);

if(min!=ti.ti_min || min==-1)
	{
	gotoxy(68,2);
	ti.ti_hour++;
	if(ti.ti_hour < 13)
		{
		if(ti.ti_hour==1)
			printf("[37m1:%-.2d am ",ti.ti_min);
		else
			printf("[37m%d:%-.2d am ",ti.ti_hour-1,ti.ti_min);
		}
	else
		{
		ti.ti_hour-=13;
		if(ti.ti_hour==0)
			printf("[37m1:%-.2d pm ",ti.ti_min);
		else
			printf("[37m%d:%-.2d pm ",ti.ti_hour,ti.ti_min);
		}
	min=ti.ti_min;
	}
if(min==0)
	min=ti.ti_min;
}


get_modem_line()
{
time_t timest, timenow;
char s[80];
int g;
g=0;
time(&timest);
while(!kbhit())
	{
	updatetd();
	updatecy();
	cursor(0,25);
	if(comm_avail())
		{
		g=comm_getc(1);
		if(isdigit(g) != 0)
			{
			result[count]=g;
			count++;
			}
		if(g==13)
			{
			result[count]=0;
			return(1);
			}
		}
	}
return(0);
}

get_modem_lineno()
{
time_t tist, tinow;
char s[80];
int g,i;
g=i=0;
time(&tist);
while(1)
	{
	if(i > 10)
		{
      count=result[0]=0;
		pr1("\27");
		init_modem();
		return(0);
		}
	if(comm_avail())
		{
		g=comm_getc(1);
		if(isdigit(g) != 0)
			{
			result[count]=g;
			count++;
			}
		if(g==13)
			{
			result[count]=0;
			return(1);
			}
		}
	i++;
	}
}


updatecy()
{
if(yesno)
	{
	yesno=0;
	return;
	}
gotoxy(pos+3,21);
if(pos == 13 && way == 1)
	way=0;
if(pos == 0 && way == 0)
	way=1;
if(way == 1)
	{
	if(pos > 0)
		{
		gotoxy(pos+2,21);
		printf("[37m%c",cy[pos-1]);
		cursor(0,25);
		}
	}
else
	{
	if(pos < 13)
		{
		gotoxy(pos+4,21);
		printf("[37m%c",cy[pos+1]);
		cursor(0,25);
		}
	}
if(way == 1 && pos > 0)
	gotoxy(pos+3,21);
if(way == 0 && pos < 18)
	gotoxy(pos+3,21);
printf("[31m%c",cy[pos]);
cursor(0,25);
if(way == 1)
	pos++;
else
	pos--;
delay(100);
}


wfc()
{
char buff[41];
int c,g;
time_t tist,tinow;
baud=0;

if(checkformouse())
	mouseyes=1;
else
	mouseyes=0;

user.name[0]=0;
user.sl=0;
user.phone[0]=0;
user.note[0]=0;
setmci(0);
puttext(1,1,80,25,IMAGEDATA);
printwfcstats();
comm_open(config.port,config.baud);
comm_flush();
init_modem();
count=0;
g=0;

while(1)
	{
	g=get_modem_line();
	if(g==1)
		{
		if(atoi(result)==2)
			{
			pr1("ATA|");
			gotoxy(45,21); printf("[33mAnswer ");
			cursor(0,25);
			g=0;
			time(&tist);
			while(difftime(time(&tinow),tist) < 10)
				{
				count=result[0]=0;
				g=get_modem_line();
				if(g==1)
					{
					switch(atoi(result))
						{
						case 1:
							gotoxy(45,21); printf("[33m300    ");
							printf("[0m");
							baud=300;
							dobaud(300);
							useron=1;
							logon();
							break;
						case 5:
							gotoxy(45,21); printf("[33m1200   ");
							printf("[0m");
							baud=1200;
							dobaud(1200);
							useron=1;
							logon();
							break;
						case 10:
							gotoxy(45,21); printf("[33m2400   ");
							printf("[0m");
							baud=2400;
							dobaud(2400);
							useron=1;
							logon();
							break;
						case 13:
							gotoxy(45,21); printf("[33m9600   ");
							printf("[0m");
							baud=9600;
							dobaud(9600);
							useron=1;
							logon();
							break;
						case 15:
							gotoxy(45,21); printf("[33m1200ARQ");
							printf("[0m");
							baud=1200;
							dobaud(1200);
							useron=1;
							logon();
							break;
						case 16:
							gotoxy(45,21); printf("[33m2400ARQ");
							printf("[0m");
							baud=2400;
							dobaud(2400);
							useron=1;
							logon();
							break;
						case 19:
							gotoxy(45,21); printf("[33m4800ARQ");
							printf("[0m");
							baud=4800;
							dobaud(4800);
							useron=1;
							logon();
							break;
						case 18:
							gotoxy(45,21); printf("[33m4800   ");
							printf("[0m");
							baud=4800;
							dobaud(4800);
							useron=1;
							logon();
							break;
						case 17:
							gotoxy(45,21); printf("[33m19200  ");
							printf("[0m");
							baud=19200;
							dobaud(19200);
							useron=1;
							logon();
							break;
						}
					}
				}
			count=result[0]=0;
			pr1("\27");
			init_modem();
			}
		}
	if(g==0)
		{
		c=getch();
		if(c==0)
			{
			c=getch();
			switch(c)
				{
				case 59: 								/* F1 */
					exit_bbs(255);
					break;
				case 60:									/* F2 */
					pr1("ATA|");
					count=result[0]=0;
					g=get_modem_line();
					if(g==1)
						{
						switch(atoi(result))
							{
							case 5:
								printf("[0m");
								baud=1200;
								dobaud(1200);
								useron=1;
								logon();
								break;
							case 10:
								printf("[0m");
								baud=2400;
								dobaud(2400);
								useron=1;
								logon();
								break;
							case 13:
								printf("[0m");
								baud=9600;
								dobaud(9600);
								useron=1;
								logon();
								break;
							case 18:
								printf("[0m");
								baud=4800;
								dobaud(4800);
								useron=1;
								logon();
								break;
							case 17:
								printf("[0m");
								baud=19200;
								dobaud(19200);
								useron=1;
								logon();
								break;
							}
						}
					count=result[0]=0;
					break;
				case 61:									/* F3 */
					break;
				case 62:									/* F4 */
					break;
				case 63:									/* F5 */
					break;
				case 64:									/* F6 */
					clear_screen();
					spawnlp(P_WAIT,"INIT.EXE","INIT.EXE",NULL);
					readfiles();
					puttext(1,1,80,25,IMAGEDATA);
					printwfcstats();
					break;
				case 65:									/* F7 */
					clear_screen();
					system("COMMAND /C TERM.BAT");
					puttext(1,1,80,25,IMAGEDATA);
					printwfcstats();
					break;
				case 66: 								/* F8 */
					clear_screen();
					puttext(1,1,25,80,IMAGEDATA);
					printwfcstats();
					break;
				case 67:									/* F9 */
					break;
				case 68:									/* F10 */
					printf("[0m");
					baud=config.baud;
					useron=0;
					logon();
					break;
				}
			}
		}
	}
}
