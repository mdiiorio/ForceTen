#include "struct.h"
#include <dir.h>
#include <dos.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <process.h>

extern struct userfile user;
extern struct filestruct fstruct;
extern struct fileindex fileidx[30];
extern struct confindex cidx[10];
extern struct boardindex bidx[30];
extern struct headerinfo hdr;

extern int firstmsg, totalmsgs;
extern int baud,timeleft,cur,numread,curfile,curconf,confread;
extern char msgpath[80];

static int filefile,maxlist=0,fp;


gettotalmsgs()
{
int f;

makemsgpath(1);
f=open(msgpath,O_BINARY | O_RDWR);
if(f==-1)
	totalmsgs=0;
else
	{
	totalmsgs=(filelength(f)) / (sizeof(struct headerinfo));
	close(f);
	}
}


makemsgpath(int i)
{
char s[80],k[80];

strcpy(s,config.supportdir);
itoa(curconf+1,k,10);
strcat(s,k);
strcat(s,"MSG");
itoa(bidx[cur].num,k,10);
strcat(s,k);
if(i)
	strcat(s,".HDR");
else
	strcat(s,".DAT");
strcpy(msgpath,s);
}



msgmenu()
{
char prompt[80],s[80],k[80];
int i;

cur=i=0;
initmsg();
if(user.sl < bidx[0].level)
	{
	nl();
	nl();
	put("4You can't access the first section");
	return;
	}
if(bidx[0].pass[0]!=0)
	{
	put("1Enter file base password:2 ");
	input(s,50);
	nl();
	if(stricmp(bidx[0].pass,s)!=0)
		{
		pl("4Sorry, wrong password");
		return;
		}
	}


if(dispnoerr("MWELC.ANS")==-1)
	{
	put_char(12);
	put("4");
	print("ÖÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ·"); nl();
	print("º Conference:                           º"); nl();
	print("º Current Base:                         º"); nl();
	print("ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶"); nl();
	print("º Total Msgs:                           º"); nl();
	print("º Last Read :                           º"); nl();
	print("ÓÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ½"); nl();
	ccursor(2,15);
	print("2%d - %s",cidx[curconf].num,cidx[curconf].name);
	ccursor(3,17);
	print("2%d - %s",bidx[cur].num,bidx[cur].name);
	gettotalmsgs();
	ccursor(5,15);
	print("3%d",totalmsgs);
	ccursor(6,15);
	print("30");
	nl();
	nl();
	nl();
	}

gettotalmsgs();

while(1)
	{
	nl();
	setmci(1);
	print("4[1%d left4 * [3Area %d4] 1Message Menu4]:2 ",chktime()/60,bidx[cur].num);
	setmci(0);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'R':
			readpost();
			break;
		case 'P':
			post();
			break;
		case 'A':
			printboard();
			break;
		case 'G':
			logoff();
			break;
		case '?':
			do_help("MSG");
			break;
		case 'J':
			confmenu();
			msgmenu();
			break;
		case '!':
			if(user.sl > 200)
				msgsysop();
			break;
		case 'Q':
			return;
		default:
			if(isdigit(s[0]))
				{
				i=atoi(s);
				if(i <= numread && i > 0 && user.sl >= bidx[i-1].level)
					{
					if(fileidx[i-1].pass[0]!=0)
						{
						nl();
						put("1Enter file base password:2 ");
						input(k,50);
						if(stricmp(bidx[i-1].pass,k)!=0)
							{
							nl();
							pl("4Sorry, wrong password");
							}
						else
							{
							cur=i-1;
							gettotalmsgs();
							}
						}
					else
						{
						cur=i-1;
						gettotalmsgs();
						}
					}
				}
			break;
		}
	}
}



printboard()
{
int i,c,e,k;

put_char(12);
maxlist=0;
i=0;
nl();
nl();
pl("1 #     Name                       Level    Net     Max");
pl("4ÄÄÄÄ ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ");
/*    [1 ] [ New Uploads             ] [ 255 ] [ Yes ] [     ]   */
while(i < numread)
	{
	if(user.sl >= bidx[i].level)
		{
		print("[2%-2d4] ",bidx[i].num);
		print("[3 %-23s 4] ",bidx[i].name);
		print("[3 %-3d 4] ",bidx[i].level);
		if(bidx[i].net)
			put("[3 Yes 4] ");
		else
			put("[3 No  4] ");
		print("[2 %-3d 4] ",bidx[i].maxmsgs);
		nl();
		}
	i++;
	}
}



initmsg()
{
char s[80],k[80];
int i;

numread=0;
i=curconf;
i++;
strcpy(s,config.supportdir);
itoa(i,k,10);
strcat(s,k);
strcat(s,"MSG.IDX");
i=0;

filefile=open(s,O_RDWR | O_BINARY);

if(filefile!=-1 && (filelength(filefile)!=0))
	{
	while(!eof(filefile))
		{
		read(filefile,&bidx[i],sizeof(struct boardindex));
		i++;
		numread++;
		}
	close(filefile);
	return;
	}


filefile=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
strcpy(bidx[0].name,"General");
bidx[0].num=1;
bidx[0].level=10;
bidx[0].maxmsgs=255;
bidx[0].net=0;
bidx[0].totalmsgs=0;
bidx[0].options=0;
bidx[0].pass[0]=0;
write(filefile,&bidx[0],sizeof(struct boardindex));
close(filefile);
numread=1;
}

readpost()					/* umm.... Read post! */
{
int c,curmsg,dir;
char s[80], x[10];

gettotalmsgs();

nl();
nl();
if(totalmsgs==0)
	{
	pl("4No messages exist");
	return;
	}

print("1Start with which post [31-%d1]:2 ",totalmsgs);
input(s,50);
if(s[0]==0)
	return;

c=atoi(s);

if(c > totalmsgs)
	{
	nl();
	pl("4Not that many posts!");
	return;
	}

curmsg=c;
rdpost(c);
dir=1;

do
	{
	nl();
	if(dir==1)
		print("1C/R=Forward [3B,E,F,P,Q,R,+,-,?,#1]:2 ");
	else
		print("1C/R=Reverse [3B,E,F,P,Q,R,+,-,?,#1]:2 ");
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 0:
			if(dir==1)
				{
				if(curmsg >= totalmsgs)
					put("4Last message!");
				else
					{
					curmsg++;
					rdpost(curmsg);
					}
				}
			else
				{
				if(curmsg == 1)
					put("4First message!");
				else
					{
					curmsg--;
					rdpost(curmsg);
					}
				}
			break;
		case 'B':
			nl();
			pl("4Reverse read mode enabled");
			dir=-1;
			break;
		case 'E':
			break;
		case 'F':
			nl();
			pl("4Forward read mode enabled");
			dir=1;
			break;
		case 'P':
			post(0,curmsg);
			break;
		case 'Q':
			break;
		case 'R':
			post(1,curmsg);
			break;
		case '+':
			gethead(curmsg);
			if(hdr.lastrep!=0)
				{
				for(c=1;c;)
					{
					if(hdr.lastrep!=0)
						{
						rdpost(hdr.lastrep+1);
						nl();
						put("1Forward Thread Scan - Q to Quit, C/R to continue:2 ");
						input(x,10);
						strupr(x);
						if(x[0]=='Q')
							c=0;
						}
					else
						{
						print("4\bNo replies found");
						c=0;
						}
					}
				}
			else
				print("4\bNo replies found");
			nl();
			break;
		case '-':
			gethead(curmsg);
			if(hdr.prevrep!=0)
				{
				for(c=1;c;)
					{
					if(hdr.prevrep!=0)
						{
						rdpost(hdr.prevrep);
						nl();
						put("1Reverse Thread Scan - Q to Quit, C/R to continue:2 ");
						input(x,10);
						strupr(x);
						if(x[0]=='Q')
							c=0;
						}
					else
						{
						print("4\bNo replies found");
						c=0;
						}
					}
				}
			else
				print("4\bNo replies found");
			nl();
			break;
		case '?':
			nl();
			nl();
			ansic(4);
			pl("ÄÍ] Message Read Commands [ÍÄ");
			pl("  B - Reverse read");
			pl("  E - E-mail poster");
			pl("  F - Forward read");
			pl("  P - Post a message");
			pl("  Q - Quit reading messages");
			pl("  R - Reply to poster");
			pl("  - - Reverse thread read");
			pl("  + - Forward thread read");
			pl("  # - Message number");
			nl();
			break;
		default:
			c=atoi(s);
			if(c > 0 && c <= totalmsgs)
				{
				curmsg=c;
				rdpost(curmsg);
				}
			break;
		}
	} while(s[0]!='Q');
nl();
nl();
}




post(int rep, int curmsg)
{
int f,c,t;
long i;
char s[80];

makemsgpath(1);
f=open(msgpath,O_BINARY | O_RDWR);
if(f==-1)								/* If no other msgs */
	{
	if(!inputheadinfo())
		return;
	if(!editor())
		return;

	makemsgpath(1);
	f=open(msgpath,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
	c=0;
	write(f,&c,1);
	hdr.num=hdr.prevrep=hdr.lastrep=hdr.datapoint=hdr.numreps=0;
	hdr.nextpoint=1;
	write(f,&hdr,sizeof(struct headerinfo));
	close(f);
	gettotalmsgs();
	return;
	}

t=filelength(f);
t-=1;
t=t/(sizeof(struct headerinfo));

if(rep)
	{
	hdr.prevrep=curmsg;
	hdr.lastrep=hdr.numreps=0;
	}
else
	hdr.prevrep=hdr.lastrep=hdr.numreps=0;

hdr.num=t;


makemsgpath(0);										/* Get "datapoint" set */
c=open(msgpath,O_BINARY | O_RDWR);
if(c==-1)
	c=open(msgpath,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
	if(c==-1)
		{
		nl();
		pl("Error creating data file.");
		return;
		}

hdr.datapoint=filelength(c);
close(c);

if(!inputheadinfo())
	{
	close(f);
	return;
	}

if(!editor())
	{
	close(f);
	return;
	}

lseek(f,0l,SEEK_END);
write(f,&hdr,sizeof(struct headerinfo));
close(f);

makemsgpath(1);
if(rep)										/* get reply pointers set */
	{
	c=open(msgpath,O_BINARY | O_RDWR);
	i=sizeof(struct headerinfo);
	i=i * (curmsg-1);
	lseek(c,i+1,SEEK_SET);
	read(c,&hdr,sizeof(struct headerinfo));
	hdr.lastrep=t;
	hdr.numreps++;
	lseek(c,i+1,SEEK_SET);
	write(c,&hdr,sizeof(struct headerinfo));
	strcpy(s,hdr.poster);
	gethead(t+1);
	strcpy(hdr.to,s);
	i=t * (sizeof(struct headerinfo));
	lseek(c,i+1,SEEK_SET);
	write(c,&hdr,sizeof(struct headerinfo));
	close(c);
	}

gettotalmsgs();
}

inputheadinfo()				/* Input/Set header info for cur msg */
{
char s[80];
struct userfile tempu;

nl();
nl();
put("1Title:2 ");
input(hdr.title,50);
if(hdr.title[0]==0)
	{
	nl();
	pl("Post aborted");
	return(0);
	}

if(!hdr.prevrep)
	{
	nl();
	put("1To [All]:2 ");
	input(s,50);
	if(s[0]==0)
		{
		put("All");
		strcpy(hdr.to,"All");
		}
	}


sprintf(hdr.poster,"%s",user.name);
sprintf(hdr.posternote,"%s",user.note);
gettime(&hdr.posttime);
getdate(&hdr.postdate);
return(1);
}


rdpost(int i)					/* Print message (calls misc. support routines) */
{
int c,f,posts;
char s[100];

gethead(i);
put_char(12);
print("3Title: 4%s - [3%d/%d4]",hdr.title,i,totalmsgs); nl();
print("3From : 4%s 4[2%s4]",hdr.poster,hdr.posternote); nl();
print("3To   : 4%s",hdr.to); nl();
print("3Date : 4%d/%d/%d - %d:%d:%d",hdr.postdate.da_day,hdr.postdate.da_mon,(hdr.postdate.da_year)-1900,hdr.posttime.ti_hour,hdr.posttime.ti_min,hdr.posttime.ti_sec);
if(hdr.prevrep)
	{
	nl();
	print("    4! This is a reply to message #3%d",hdr.prevrep);
	}
if(hdr.lastrep)
	{
	nl();
	print("    4! This message has 3%d4 replie(s)",hdr.numreps);
	}
nl();
pl("1ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
nl();
ansic(4);
makemsgpath(0);
f=open(msgpath,O_BINARY | O_RDWR);
if(f==-1)
	{
	nl();
	pl("Data file screwed");
	return;
	}
close(f);
readdata();
nl();
}


readdata()						/* Read/Print .DAT info to screen */
{
int f,c;

makemsgpath(0);
f=open(msgpath,O_BINARY | O_RDWR);
lseek(f,hdr.datapoint,SEEK_SET);

do
	{
	read(f,&c,1);
	if((char) c=='\n')
		nl();
	else
		print("%c",c);

	} while((char) c!='\x0');

close(f);
}

gethead(int i)					/* Read .HDR info for current msg. */
{
int c,f;

makemsgpath(1);
f=open(msgpath,O_BINARY | O_RDWR);
if(f==-1)
	{
	nl();
	pl("No messages exist");
	return;
	}

read(f,&c,1);
lseek(f,((i-1) * (sizeof(struct headerinfo)))+1,SEEK_SET);
read(f,&hdr,sizeof(struct headerinfo));
close(f);
}


/*  ELITE Force Ten message editor - Will change!!  */

editor()
{
FILE *fp;
int f,c,line;
char array[50][80];

put_char(12);
print("4Title: 3%s  1=-=  4Force Ten ELITE Message editor!",hdr.title); nl();
print("3ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
nl();
ansic(4);

for(line=0;;line++)
	{
	input(array[line],79);
	nl();
	if(array[line][0]=='/')
		{
		switch(array[line][1])
			{
			case 'S':
			case 's':
				makemsgpath(0);
				fp=fopen(msgpath,"rb+");
				if(fp==NULL)
					fp=fopen(msgpath,"ab+");
				fseek(fp,0l,SEEK_END);
				for(c=0;c < line;c++)
					{
					fprintf(fp,"%s",array[c]);
					if(c!=line-1)
						fprintf(fp,"\n");
					}
				c=NULL;
				fputc(c,fp);
				fclose(fp);
				return(1);
			case 'A':
			case 'a':
				return(0);
			}
		}
	}
}





