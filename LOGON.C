#include "struct.h"
#include "comm.h"
#include "keys.h"
#include <time.h>
#include <dos.h>
#include <conio.h>
#include <ctype.h>

extern int timeleft;
extern int timecheck;
extern int useron;
extern time_t timeon;
extern int mouseyes,button_num;


logon()
{
char s[80];
int c,ti,times,i,goon;
struct date date;
struct time tim;

timeleft=times=user.timeon=timecheck=0;
user.menutype=goon=1;

showmouse();
user.number=-1;
clear_screen();
initansicolors();
updatestat();
if(config.xsec)
	secproc();
if(config.matrix)
	matrix();

put_char(12);
ansic(4);
pl("Force Ten BBS Software");
pl("Force Ten Crew: Mr. Doo");
pl("This version: November 19, 1990");
nl();
ansic(4);
regdispfile("LOGON.MSG");
nl();
while(goon)
	{
	if(times==3)
		hangnosave();
	ansic(4);
	put("[Name or Number]: ");
	input(s,50);
	times++;
	nl();
	if(s[0]!=0)
		{
		c=ver_user(s);
		if(c==0)
			{
			if(!isdigit(s[0]))
				{
				nl();
				print("%s was not found in the user file.",s);
				nl();
				put("Apply New [Y/n]: ");
				if(yn())
					{
					newuser();
					goon=0;
					}
				nl();
				nl();
				}
			}
		else
			{
			get_user(s);
			ti=user.sl;
			time(&timeon);
			gettime(&tim);
			nl();
			if(tim.ti_hour >= 6 && tim.ti_hour < 12)
				print("Good morning, %s",user.name);
			if(tim.ti_hour >= 12 && tim.ti_hour < 17)
				print("Good afternoon, %s",user.name);
			if(tim.ti_hour >= 17 && tim.ti_hour <= 24)
				print("Good evening, %s",user.name);
			if(tim.ti_hour < 6)
				print("Kinda' late %s, isn't it?",user.name);
			nl();
			nl();
			getpassword();
			if((user.logons % 5) == 0)
				getphone();
			if(user.del)
				{
				nl();
				nl();
				put("Your account was terminated.");
				nl();
				nl();
				hangup();
				}
			if(ti >= config.cosysop)
				getcopass();
			if(ti <= 10)
				{
				nl();
				nl();
				pl("Do not log on unless given the password by the sysop.");
				pl("Next time you will be dealt with accordingly.");
				hangup();
				}
			goon=0;
			}
		}
	}
user.sl=ti;
if(user.laston.da_mon == date.da_mon)
	if(user.laston.da_day != date.da_day)
		{
		if(user.sl > 255)
			user.timeall=config.times[255] * 60;
		else
			user.timeall=config.times[user.sl] * 60;
		user.timeon=0;
		}
else
	{
	if(user.sl > 255)
		user.timeall=config.times[255] * 60;
	else
		user.timeall=config.times[user.sl] * 60;
	user.timeon=0;
	}

timeleft=user.timeall-user.timeon;
getdate(&user.laston);
getdate(&user.newscan);
user.logons++;
gettime(&user.timelaston);
if(stricmp(config.sysop,user.name)!=0)
	{
	config.calls++;
	strcpy(config.lastcaller,user.name);
	writeconfig();
	}
writeuser();
nl();
nl();
if(chktime()/60 == 0 && user.timebank > 0)
	{
	put("1Would you like to withdrawl from the timebank [Y/n]: ");
	if(yn())
		time_bank();
	}
bbs();
}



getpassword()
{
char s[80];
int times;

times=0;

while(times < 4)
	{
	put("[Password]: ");
	inputpass(s,17,user.pass);
	nl();
	if(stricmp(user.pass,s)!=0)
		times++;
	else
		return;
	}
user.hacks++;
hangup();
}

getcopass()
{
char s[80];
int times;

times=0;

while(times < 4)
	{
	put("[Privledge Password]: ");
	inputpass(s,17,config.copass);
	nl();
	if(stricmp(config.copass,s)!=0)
		times++;
	else
		return;
	}
user.hacks++;
hangup();
}

getphone()
{
char s[80],s2[80];
int times,i,ok;

times=0;

while(times < 4)
	{
	ok=1;
	put("[Phone ###-###-");
	input(s,4);
	put("]");
	nl();
	i=0;
	while(i < 5)
		{
		if(s[i]==user.phone[i+9])
			ok=1;
		else
			{
			ok=0;
			i=10;
			}
		i++;
		}
	if(!ok)
		times++;
	else
		return;
	}
user.hacks++;
hangup();
}



matrix()
{
char s[80],s2[81],prompt[81];
int i,ok,times,c,ti,b;
struct userfile tempu;

times=user.number=0;

ansic(4);
put_char(12);


while(1)
	{
	nl();
	if(times==5)
		{
		if(user.number>0)
			hangup();
		else
			hangnosave();
		}
	b=do_matrix();
	times++;
	switch(b)
		{
		case 0:
			if(config.mtype==1)
				print_matrix2();
			break;
		case 1:
			nl();
			nl();
			put("1Port 1 Access:2 ");
			inputpass(s2,17,config.systempass);
			if(stricmp(config.systempass,s2)==0)
				{
				put_char(12);
				return;
				}
			nl();
			break;
		case 2:
			nl();
			put("4Port 2 not active at the present time.");
			nl();
			nl();
			break;
		case 3:
			nl();
			nl();
			put("1[Name or Number]: 2");
			input(s2,50);
			if(s2[0]==0)
				break;
			nl();
			c=get_user(s2);
			if (c>0)
				{
				getpassword();
				getphone();
				if(user.sl >= config.cosysop)
					getcopass();
				}
			else if(c==0)
				{
				nl();
				pl("4Account non-existant.");
				user.number=-1;
				}
			if(user.sl > 10  && c > 0)
				{
				nl();
				nl();
				put("4Port 1 Access: 2");
				print("\"%s\"",config.systempass);
				nl();
				nl();
				put("1Connect to Port 1 [4Y/n1]:2 ");
				if(yn())
					{
					put_char(12);
					return;
					}
				}
			else
				{
				nl();
				nl();
				put("4You cannot access Port 1.");
				nl();
				nl();
				}
			break;
		case 4:
			newuser();
			for(i=0;i<25;i++)
				nl();
			pl("4Call back in 24 hours and check for access.");
			newhangup();
			break;
		case 5:
			break;
		case 6:
			put("Closing tty01");
			if(user.number>0)
				hangup();
			else
				hangnosave();
			break;
		}
	}
}


do_matrix()
{
char s[80];
int len;
if(config.mtype==2)
	{
	ansic(4);
	if(user.name[0]==0)
		print("Acct: None {tty01}: ");
	else
		print("Acct: %s {tty01}: ",user.name);
	input(s,80);
	switch(s[0])
		{
		case '1':
			return(1);
		case '2':
			return(2);
		case '3':
			return(3);
		case '4':
			return(4);
		case '5':
			return(5);
		case '6':
			return(6);
		case '?':
			print_matrix1();
			break;
		}
	return(-1);
	}
if(config.mtype==1)
	{
	ansic(4);
	strcpy(s,config.rootdir);
	len=strlen(s);
	if(s[len-1]==92 || s[len-1]==47)
		s[len-1]=0;
	print("%s>",s);
	input(s,50);
	if(stricmp("DIR",s)==0)
		return(0);
	if(stricmp("SYSTEM1",s)==0 || stricmp("SYSTEM1.EXE",s)==0)
		return(1);
	if(stricmp("SYSTEM2",s)==0 || stricmp("SYSTEM2.EXE",s)==0)
		return(2);
	if(stricmp("CHECK",s)==0 || stricmp("CHECK.COM",s)==0)
		return(3);
	if(stricmp("APPLY",s)==0 || stricmp("APPLY.COM",s)==0)
		return(4);
	if(stricmp("FEEDBACK",s)==0 || stricmp("FEEDBACK.EXE",s)==0)
		return(5);
	if(stricmp("LOGOFF",s)==0 || stricmp("LOGOFF.COM",s)==0)
		return(6);
	nl();
	pl("Bad command or file name");
	return(-1);
	}
}

print_matrix1()
{
char s[90];

strcpy(s,config.gfiledir);
strcat(s,"MATRIX.MSG");
if(access(s,0)!=-1)
	dispfile("MATRIX.MSG");
else
	{
	put_char(12);
	nl();
	pl("Logged on tty01");
	pl("Options as follows:");
	nl();
	pl(" 1. Connect to Port 1");
	pl(" 2. Connect to Port 2");
	pl(" 3. Check for Access");
	pl(" 4. Apply for Account");
	pl(" 5. Feedback to System Operator");
	pl(" 6. Logout");
	nl();
	}
}

print_matrix2()
{
int len;
char s[80];
strcpy(s,config.rootdir);
len=strlen(s);
if(s[len-1]==92 || s[len-1]==47)
	s[len-1]=0;
ansic(4);
nl();
nl();
pl(" Volume in drive C is Force Ten");
print(" Directory of %s",s);
nl();
nl();
pl(".            <DIR>");
pl("..           <DIR>");
pl("SYSTEM1  EXE    79020    3-20-90    7:38p");
pl("SYSTEM2  EXE    53993    3-20-90    7:46p");
pl("CHECK    COM     9303    3-20-90    7:53p");
pl("APPLY    COM    15939    3-20-90    8:03p");
pl("FEEDBACK EXE    45060    3-20-90    8:21p");
pl("LOGOFF   COM     5949    3-20-90    8:34p");
pl("        8 File(s)  15837233 bytes free");
nl();

}



initansicolors()
{
int i;

for(i=0;i<10;i++)
	csc(i,7,0,0);
}


csc(int w,int i1,int i2,int i3)
{
user.colors[w].fore=i1;
user.colors[w].back=i2;
user.colors[w].hilit=i3;
}


newuser()
{
char s[80];
int c,i,yes,num,sl;
struct date dateon;

c=1;
i=yes=0;

dispfile("NEWUSER.MSG");

nl();
nl();
put("4Looking for an open space...");

while(c!=0)
	{
	i++;
	sprintf(s,"%d",i);
	c=get_user(s);
	if(user.del)
		{
		pl("  Found");
		yes=1;
		c=0;
		}
	}

if(!yes)
	{
	pl("  Found");
	user.number=i;
	}

num=user.number;
nl();
getname();
nl();
getrealname();
nl();
getnewpass();
nl();
user.menutype=1;
getnewphone();
nl();
getemu();
initansicolors();
user.lines=24;
user.timeon=0;
user.sl=10;
user.del=0;
user.filepoints=0;
user.exempt=0;
user.waiting=0;
user.timeon=0;
user.timeall=600;
user.timebank=0;
user.logons=0;
user.msgpost=0;
user.emailsent=0;
user.k_dl=0;
user.k_ul=0;
user.num_dl=0;
user.num_ul=0;
strcpy(user.macro,"[> ");
strcat(user.macro,user.name);
strcat(user.macro," <]");
strcpy(user.note,"New User");
getdate(&dateon);
user.firston.da_year=dateon.da_year;
user.firston.da_mon=dateon.da_mon;
user.firston.da_day=dateon.da_day;
nl();
pers_mod();
put_char(12);
user.number=num;
user.sl=10;
user.filesl=10;
user.gfilesl=10;
user.timeon=0;
print("4Your User number is 3%d",user.number);
nl();
print("4Your Password is 3%s",user.pass);
nl();
pl("4Remeber your password!");
nl();
put("Hit any key to continue...");
comm_flush();
get_c();
}



change(int i)
{
char s[10];
char clrs [10][20] = {"Prompt","Input","Stat","Default","Field Input","Box Border","Box Inside","Y/N Question","Y/N Answer","Not Used"};
int fore,back,hilit,blink;
nl();
put("4[0]"); textcolor(LIGHTGRAY); cprintf(" Black"); putremote("[37;40mBlack"); nl();
put("4[1]"); textcolor(BLUE); cprintf(" Blue"); putremote("[34;40m Blue"); nl();
put("4[2]"); textcolor(GREEN); cprintf(" Green"); putremote("[32;40m Green"); nl();
put("4[3]"); textcolor(CYAN); cprintf(" Cyan"); putremote("[36;40m Cyan"); nl();
put("4[4]"); textcolor(RED); cprintf(" Red"); putremote("[31;40m Red"); nl();
put("4[5]"); textcolor(MAGENTA); cprintf(" Magenta"); putremote("[35;40m Magenta"); nl();
put("4[6]"); textcolor(14); cprintf(" Yellow"); putremote("[33;40;1m Yellow"); nl();
put("4[7]"); textcolor(LIGHTGRAY); cprintf(" White"); putremote("[37;40;0m White"); nl();

nl();
pl("4Current: ");
nl();
print("4[%d] %s  ",i,clrs[i-1]);
colorstr(i);
ansic(4);
nl();
print("1Foreground:2 ",clrs[i-1]);

input(s,50);
if(s[0] >= '0' && s[0] < '8' && s[0]!=0)
	fore=s[0]-48;
else
	fore=user.colors[i-1].fore;

nl();
print("1Background:2 ",clrs[i-1]);

input(s,50);
if(s[0] >= '0' && s[0] < '8' && s[0]!=0)
	back=s[0]-48;
else
	back=user.colors[i-1].back;

nl();
put("1Hilight [3Y/n1]: ");
if(yn())
	hilit=1;
else
	hilit=0;
nl();

user.colors[i-1].fore=fore;
user.colors[i-1].back=back;
user.colors[i-1].hilit=hilit;
user.colors[i-1].blink=0;
}


getname()
{
char s[80];
int times;

times=0;

while(times < 10)
	{
	if(times == 3)
		hangnosave();
	put("Enter your handle: ");
	input(s,50);
	nl();
	if(get_user(s) == 0 && s[0]!=0)
		{
		strcpy(user.name,s);
		times=11;
		}
	if(get_user(s)!=0)
		{
		nl();
		pl("That name is already in use");
		nl();
		times--;
		}
	times++;
	}

strcpy(user.name,s);
}

getrealname()
{
int times;

times=0;
while(times < 10)
	{
	if(times == 3)
		hangnosave();
	put("Enter your real name: ");
	input(user.realname,50);
	nl();
	if(user.realname[0]!=0)
		times=11;
	times++;
	}
}

getnewpass()
{
int times;
char s[80],s2[80];

times=0;
while(times < 10)
	{
	if(times == 3)
		hangnosave();
	put("Enter the password you intend to use: ");
	input(s,14);
	nl();
	nl();
	put("Now enter it again for verification: ");
	inputpass(s2,17,s);
	if(stricmp(s2,s)==0 && s[0]!=0 && s2[0]!=0)
		{
		strcpy(user.pass,s);
		nl();
		times=11;
		}
	else
		{
		nl();
		pl("Passwords do not match");
		nl();
		times--;
		}
	times++;
	}
}

getnewphone()
{
int times;

times=0;
while(times < 10)
	{
	if(times == 3)
		hangnosave();
	put("Now enter your phone: ");
	inputphone(user.phone);
	nl();
	if(user.phone[0]!=0)
		times=11;
	times++;
	}
}

getemu()
{
char s[80];

nl();
pl("1Pick type of emulation:");
pl("4[214] ASCII");
pl("4[224] ANSI");
pl("4[234] Enhanced ANSI 3*");
nl();
pl("3* 4Enhanced ANSI uses extensive use of scroll bars, etc.");
nl();
nl();
put("1Choice: 2");
s[0]=1;

input(s,1);
if(s[0]!=0)
	{
   if(s[0]=='1')
		{
		user.menutype=1;
		user.fullscreen=0;
		}
	if(s[0]=='2')
		{
		user.menutype=2;
		user.fullscreen=1;
		}
	if(s[0]=='3')
		{
		user.menutype=3;
		user.fullscreen=1;
		}
	}
}

secproc()
{
char s[80];
int times;
times=0;
put_char(12);
nl();
pl("TOS v32.4.R34.3 / Type \"login\" to begin session");

times=1;
while(times)
	{
	nl();
	input(s,255);
	if(stricmp(config.xpass,s)==0)
		return;
	if(stricmp("LOGIN",s)==0)
		{
		nl();
		nl();
		put("Username: ");
		input(s,255);
		nl();
		if(stricmp("GUEST",s)==0)
			{
			put("Password: ");
			input(s,255);
			if(stricmp("GUEST",s)==0)
				{
				put_char(12);
				put_char(7);
				nl(); nl(); nl();
				pl("********************************************************************");
				pl("* TOS v32.4.R34.3 / Registered to: Bell Labs Security Dept.        *");
				pl("*                                                                  *");
				pl("* Note to Systems Dev. Staff:                                      *");
				pl("*                                                                  *");
				pl("* Copies of standard DES decryption will be on-line for capture    *");
				pl("* untill the end of the week.                                      *");
				pl("*                                                ");
				put("ðèûûÍêõ±,2RŽ");
				hangnosave();
				}
			}
		else
			{
			pl("Invalid login");
			pl("Session terminated");
			times=0;
			}
		}
	}
hangnosave();
}
