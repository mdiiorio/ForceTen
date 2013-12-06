#include "struct.h"
#include "comm.h"
#include "keys.h"
#include <time.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

extern int timeleft;
extern int timecheck;
extern time_t timeon;
extern struct userfile user;
extern struct configfile config;
extern struct filestruct fstruct;
extern struct bque que;
extern struct headerinfo hdr;
extern int curconf,confread;

char automsg[5][80];

struct _bbslst
	{
	char phone[15],
		  name[46],
		  baud[5],
		  software[14],
		  extdesc[80];
	} bbslst;


bbs()
{
if(user.menutype==1)
	dispfile("WELCOME.ASC");
else
	dispfile("WELCOME.ANS");
pausescr();
print_stats();
ccursor(24,1);
pausescr();
nl();nl();nl();nl();nl();nl();
initconffile();
topmenu();
}


topmenu()
{
char prompt[80],s[80];
int ti;

while(1)
	{
	timecheck=1;
	nl();
	print("4[1%d left4 * [3Conf %d4] 1Main Menu4]: 2",chktime()/60,curconf+1);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'Y':
			put_char(12);
			print_stats();
			ccursor(24,1);
			pausescr();
			break;
		case 'L':
			user_list();
			break;
		case '$':
			time_bank();
			break;
		case 'B':
			bbslist();
			break;
		case 'O':
			online_games();
			break;
		case 'G':
			logoff();
			break;
		case 'M':
			msgmenu();
			break;
		case 'F':
			que.files=0;
			filemenu();
			break;
		case 'J':
			confmenu();
			break;
		case 'U':
			pers_mod();
			break;
		case '!':
			sysopmenu();
			break;
		case 'C':
			chatcall();
			break;
		case '?':
			if(user.menutype==1)
				dispfile("MAIN.ASC");
			else
				dispfile("MAIN.ANS");
			break;
		}
	}
}


logoff()
{
time_t timenow;
time(&timenow);
nl();nl();
print("1Really logoff %s [8y/N1]: 2",config.name);
if(!ny())
	{
	nl();
	nl();
	print("4Later, 0%s",user.name);
	hangup();
	}
nl();
nl();
nl();
}


chatcall()
{
char s[80];
int i,c;

nl();
nl();
put("1Enter reason to chat:2 ");
input(s,50);
if(s[0]==0)
	{
	nl();
	pl("4Aborted");
	return;
	}

nl();
nl();

print("4!! User wants to chat, reason: 3%s",s);
nl();
nl();
for(c=0;c<8;c++)
	{
	put("1** 2! 1**");
	for(i=0;i<7;i++)
		{
		sound(1500);
		if(kbhit())
			{
			nosound();
			if(chatkbhit())
				return;
			}
		delay(50);
		sound(2000);
		if(kbhit())
			{
			nosound();
			if(chatkbhit())
				return;
			}
		delay(50);
		nosound();
		}
	for(i=0;i<1800;i++)
		if(kbhit())
			if(chatkbhit())
				return;
	}
nosound();
}


chatkbhit()
{
int n;

n=getch();
if(n==32)
	{
	ansic(4);
	chat();
	put_char(12);
	return(1);
	}
if(n==27)
	{
	nl();
	put("4The SysOp does not wish to chat, leave E-Mail if you want.");
	nl();
	return(1);
	}
return(0);
}


user_list()
{
char s[80];
int i,goon;
struct userfile tempu;

s[0]=0;
goon=i=1;
put_char(12);
nl();
pl("1 #      Name                  Level       Note          *=Deleted");
pl("4컴�  컴컴컴컴컴컴컴컴컴컴컴� 컴컴컴� 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴");
/*    100  [ Sadistic Phiend     ] [ 255 ]-[ A k-rad, cool, k-awesome total 0d!! ]*/
readuser("1",&tempu);
while(goon)
	{
	i++;
	print("4%-3d ",i-1);
	if(tempu.del)
		put("3*");
	else
		put(" ");
	print("4[ 3%-19s 4] ",tempu.name);
	print("4[ 2%-3d 4] ",tempu.sl);
	print("4[ 2%-36s 4]",tempu.note);
	nl();
	sprintf(s,"%d",i);
	if(readuser(s,&tempu)==0)
		goon=0;
	}
}


/*
Prompt,Input,Stat,Default,Field Input,Box Border,Box Inside,Y/N Question
Y/N Answer,Not Used
*/

chg_colors()
{
char s[80];
int i;

while(1)
	{
	nl();
	nl();
	put("1Colors: [0-9][Q/Quit][L/List]: 2");
	input(s,50);
	strupr(s);
	nl();
	if(s[0]=='Q')
		return;
	else if(s[0]=='L')
		list_colors();
	else
		{
		i=atoi(s);
		if(i >= 0 && i <= 9)
			change(i+1);
		}
	}
}



list_colors()
{
nl();
print("4[204] Prompt      : ");
colorstr(1);
print("4[214] Input       : ");
colorstr(2);
print("4[224] Information : ");
colorstr(3);
print("4[234] Default     : ");
colorstr(4);
print("4[244] Field Input : ");
colorstr(5);
print("4[254] Box Border  : ");
colorstr(6);
print("4[264] Box Inside  : ");
colorstr(7);
print("4[274] Y/N Question: ");
colorstr(8);
print("4[284] Y/N Answer  : ");
colorstr(9);
print("4[294] Not Used    : ");
colorstr(10);
}

pers_mod()
{
int times;
char s[80];

times=1;
while(times)
	{
	put_char(12);
	nl();
	print("4Stats for 3%s:",user.name);
	nl();
	nl();
	ansic(4);
	print("[214] Phone              :3 %s",user.phone);
	nl();
	ansic(4);
	print("[224] Change Password");
	nl();
	if(user.menutype==1)
		strcpy(s,"ASCII");
	if(user.menutype==2)
		strcpy(s,"ANSI");
	if(user.menutype==3)
		strcpy(s,"Enhanced ANSI");
	print("[234] Emulation          :3 %s",s);
	nl();
	ansic(4);
	print("[244] Full Screen Editor :3 ");
	if(user.fullscreen)
		put("Yes");
	else
		put("No");
	nl();
	ansic(4);
	print("[254] Pause on screen    :3 ");
	if(user.pause)
		put("Yes");
	else
		put("No");
	nl();
	ansic(4);
	print("[264] Lines on screen    :3 %d",user.lines);
	nl();
	ansic(4);
	print("[274] Macro              :3 %s",user.macro);
	nl();
	ansic(4);
	if(user.menutype > 1)
		{
		print("[284] Change Colors");
		ansic(4);
		}
	nl();
	nl();
	print("[2Q4] Configuration Done");
	nl();
	nl();
	put("1[Choice]:2 ");
	input(s,50);
	nl();
	nl();
	strupr(s);
	switch(s[0])
		{
		case '1':
			nl();
			put("1Enter you new phone:2 ");
			inputphone(user.phone);
			break;
		case '2':
			nl();
			inpass();
			break;
		case '3':
			nl();
			getemu();
			break;
		case '4':
			nl();
			put("1Use full screen editor [8Y/n1]: ");
			if(yn())
				if(user.menutype > 1)
					user.fullscreen=1;
				else
					put("4You must have ANSI active!!");
			else
				user.fullscreen=0;
			break;
		case '5':
			nl();
			put("1Pause on screen [8Y/n1]: ");
			if(yn())
				user.pause=1;
			else
				user.pause=0;
			break;
		case '6':
			nl();
			put("1How many lines [CR/25]: 2");
			input(s,80);
			if(s[0]==0)
				user.lines=25;
			else
				user.lines=atoi(s);
			break;
		case '7':
			nl();
			put("1Enter your new macro: 2");
			input(s,50);
			if(s[0]!=0)
				strcpy(user.macro,s);
			break;
		case '8':
			if(user.menutype > 1)
				chg_colors();
			break;
		case 'Q':
			times=0;
			break;
		}
	}
}



inpass()
{
char s[80],s2[80];
nl();
put("1Enter your new password: 2");
input(s,50);
if(s[0]==0)
	return;
nl();
nl();
put("1Re-enter for verification: 2");
inputpass(s2,50,s);
if(s2[0]==0)
	return;
if(stricmp(s,s2)==0)
	{
	nl();nl();
	pl("4Password changed.");
	strcpy(user.pass,s);
	}
else
	{
	nl();nl();
	pl("4Password not changed.");
	}

nl();
put("4Hit [Enter] to continue...");
get_c();
}


print_stats()
{
long a,b,c;

if(user.menutype > 1)
	{
	if(ifspace())
		return;
	ansic(4);
	makebox(6,7,7,24);
	ccursor(7,11);
	put("4[ 2Main Status 4]");
	ansic(4);
	ccursor(8,8); put("Level:");
	ccursor(9,8); put("Times on:");
	ccursor(10,8); put("Hacks:");
	ccursor(11,8); put("Timebank:");
	ccursor(12,8); put("Exempt:");
	ccursor(13,8); put("Last on:");
	ansic(3);
	ccursor(8,18); print("%d",user.sl);
	ccursor(9,18); print("%d",user.logons);
	ccursor(10,18); print("%d",user.hacks);
	ccursor(11,18); print("%d",user.timebank);
	ccursor(12,18); print("%d",user.exempt);
	ccursor(13,18); print("%02d/%02d/%d",user.laston.da_mon,user.laston.da_day,user.laston.da_year-1900);
	if(ifspace())
		return;
	ansic(4);
	makebox(29,3,7,24);
	ccursor(3,34);
	put("4[ 2File Status 4]");
	ansic(4);
	ccursor(4,31); put("Level:");
	ccursor(5,31); put("K UL:");
	ccursor(6,31); put("K DL:");
	ccursor(7,31); put("# UL:");
	ccursor(8,31); put("# DL:");
	ccursor(9,31); put("File Pts:");
	ansic(3);
	ccursor(4,41); print("%d",user.filesl);
	ccursor(5,41); print("%d",user.k_ul/1000);
	ccursor(6,41); print("%d",user.k_dl/1000);
	ccursor(7,41); print("%d",user.num_ul);
	ccursor(8,41); print("%d",user.num_dl);
	ccursor(9,41); print("%d",user.filepoints);
	if(ifspace())
		return;
	ansic(4);
	makebox(50,9,4,21);
	ccursor(9,54);
	put("4[ 2Msg. Status 4]");
	ansic(4);
	ccursor(10,52); put("Posts:");
	ccursor(11,52); put("Mail Sent:");
	ccursor(12,52); put("Mail Waiting:");
	ansic(3);
	ccursor(10,66); print("%d",user.msgpost);
	ccursor(11,66); print("%d",user.emailsent);
	ccursor(12,66); print("%d",user.waiting);
	if(user.waiting)
		put_char(7);
	if(ifspace())
		return;
	ansic(4);
	makebox(12,15,3,39);
	ccursor(15,23);
	put("4[ 2System Status 4]");
	ansic(4);
	ccursor(16,15); put("Last Caller:");
	ccursor(17,15); put("Total Calls:");
	ansic(3);
	ccursor(16,28); print("%s",config.lastcaller);
	ccursor(17,28); print("%d",config.calls);
	ansic(4);
/*	makebox(6,15,2,39);
	ccursor(15,11);
	put("4[ 2This week's lottery winner 4]");
	ansic(3);
	ccursor(16,((39-strlen("Mr. Doo"))/2)+6);
	print("%s","Mr. Doo");*/
	if(ifspace())
		return;
	makebox(2,2,3,24);
	ccursor(2,9);
	put("4[ 2Credits 4]");
	ccursor(3,4);
	put("1Force Ten v1.00");
	ccursor(4,4);
	put("1Written by Mr. Doo");
	ccursor(22,1);
	}
else
	{
	nl();nl();nl();nl();nl();nl();nl();
	print("----- Stats for %s -----",user.name);
	nl();
	nl();
	print("User SL     : %d",user.sl);
	nl();
	print("Times on    : %d",user.logons);
	nl();
	print("Hacks       : %d",user.hacks);
	nl();
	print("Mail Waiting: %d",user.waiting);
	nl();nl();
	}
}


bbslist()
{
char s[80],temp[80];
int fp,i;
struct _bbslst tempbbs;

strcpy(temp,config.supportdir);
strcat(temp,"BBS.LST");

put_char(12);
while(1)
	{
	title("BBS List");
	pl("4[214] See BBS List");
	pl("4[224] Add a BBS");
	pl("4[2Q4] Quit");
	nl();
	put("4[1BBS List4]: 2");
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case '1':
			nl();
			nl();
			nl();
			pl("4[3404-000-00004][1Temples of Syrinx                             4][324004][2ForceTen4]");
			pl("  4[3Home for Force Ten!                                                   4]");
			fp=open(temp,O_BINARY | O_RDWR);
			lseek(fp,0l,SEEK_SET);
			while(!eof(fp))
				{
				read(fp,&bbslst,sizeof(struct _bbslst));
				print("4[3%-12s4][1%-46s4][3%-4s4][2%-8s4]",bbslst.phone,bbslst.name,bbslst.baud,bbslst.software);
				nl();
				print("  4[3%-70s4]",bbslst.extdesc);
				nl();
				}
			nl();
			close(fp);
			break;
		case '2':
			nl();
			nl();
			for(i=0;i<12;i++)
				bbslst.phone[i]=0;
			for(i=0;i<46;i++)
				bbslst.name[i]=0;
			for(i=0;i<5;i++)
				bbslst.baud[i]=0;
			for(i=0;i<14;i++)
				bbslst.software[i]=0;
			for(i=0;i<80;i++)
				bbslst.extdesc[i]=0;

			if(access(temp,0)!=0)
				fp=open(temp,O_TEXT | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
			else
				fp=open(temp,O_TEXT | O_RDWR);
			pl("4Enter the phone in the form ");
			pl(" ###-###-####");
			put(":");
			dl(12);
			input(bbslst.phone,12);
			if(bbslst.phone[0]==0)
				{
				close(fp);
				return;
				}
			nl();
			while(eof(fp)==0)
				{
				read(fp,&tempbbs,sizeof(struct _bbslst));
				if(stricmp(tempbbs.phone,bbslst.phone)==0)
					{
					pl("4It's already in there!!");
					close(fp);
					return;
					}
				}
			pl("4Enter the BBS name");
			put(":");
			dl(45);
			input(bbslst.name,45);
			nl();
			pl("4Enter the baud rate (i.e. 2400, 9600, 14.4)");
			put(":");
			dl(4);
			input(bbslst.baud,4);
			nl();
			pl("4Enter the software (i.e. ForceTen, Emulex/2, TCS)");
			put(":");
			dl(8);
			input(bbslst.software,8);
			nl();
			pl("4Enter a extended description");
			put(":");
			dl(70);
			input(bbslst.extdesc,70);
			nl();
			nl();
			pl("4What you entered:");
			print("4[3%s4][1%-46s4][3%-4s4][2%-8s4]",bbslst.phone,bbslst.name,bbslst.baud,bbslst.software);
			nl();
			print("  4[3%-70s4]",bbslst.extdesc);
			nl();
			nl();
			put("1Is the above correct [8Y/n1]:2 ");
			if(yn())
				{
				lseek(fp,0l,SEEK_END);
				write(fp,&bbslst,sizeof(struct _bbslst));
				close(fp);
				}
			else
				{
				nl();
				put("4Not saved.");
				nl();
				}
			nl();
			nl();
			break;
		case 'Q':
			return;
		}
	}
}


