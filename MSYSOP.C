#include "comm.h"
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
extern struct bque que;
extern struct headerinfo hdr;

extern long firstmsg;
extern int baud,timeleft,cur,numread,curfile,curconf,confread;
extern char msgpath[80];

static int fp,filefile,linecnt;


msgsysop()
{
char s[80],k[80];
int c,i;

while(1)
	{
	nl();
	setmci(1);
	print("4[1%d left4 * 4[3Area %d4] 1Message Sysop4]:2 ",chktime()/60,bidx[cur].num);
	setmci(0);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'D':
			editboards();
			break;
		case 'Q':
			return;
		default:
			if(isdigit(s[0]))
				{
				i=atoi(s);
				if(i <= numread && i > 0 && user.sl >= bidx[i-1].level)
					{
					if(bidx[i-1].pass[0]!=0)
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
							cur=i-1;
						}
					else
						cur=i-1;
					}
				}
			break;
		}
	}
}

		
		
editboards()
{
char s[80],k[80];
int i;

while(1)
	{
	put_char(12);
	printboard();
	nl();
	put("1[Q]uit, [M]odify, [D]elete, [I]nsert:2 ");
	input(s,80);
	strupr(s);
	switch(s[0])
		{
		case 'M':
			nl();
			print("1Enter directory to edit:2 ");
			input(s,50);
			if(s[0]!=0 && atoi(s) <= numread)
				editboard(atoi(s));
			break;
		case 'D':
			nl();
			delboard();
			break;
		case 'I':
			nl();
			addboard();
			break;
		case 'Q':
			strcpy(s,config.supportdir);
         itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(s,"MSG.IDX");
			i=0;
			remove(s);
			filefile=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
			while(i < numread)
				{
				write(filefile,&bidx[i],sizeof(struct boardindex));
				i++;
				}
			close(filefile);
			nl();
			nl();
			return;
		}
	}
}

				
editboard(int c)
{
char s[80];

c--;
while(1)
	{
	put_char(12);
	print("2Board number: %d",bidx[c].num);
	nl();
	nl();
	print("4[214] Name:3 %s",bidx[c].name);
	nl();
	print("4[224] Networked:3 ");
	if(bidx[c].net)
		put("Yes");
	else
		put("No");
	nl();
	print("4[234] Max Messages:3 %d",bidx[c].maxmsgs);
	nl();
	print("4[244] Entry level:3 %d",bidx[c].level);
	nl();
	print("4[254] Password:3 %s",bidx[c].pass);
	nl();
	print("4[2Q4] Quit");
	nl();
	nl();
	put("1[Choice]:2 ");
	input(s,50);
	switch(s[0])
		{
		case '1':
			nl();
			nl();
			put("1Enter new name:2 ");
			input(s,23);
			if(s[0]!=0)
				strcpy(bidx[c].name,s);
			break;
		case '2':
			nl();
			nl();
			put("1Networked [4y/N1]:2 ");
			if(ny())
				bidx[c].net=0;
			else
				bidx[c].net=1;
			break;
		case '3':
			nl();
			nl();
			put("1Enter max messages:2 ");
			input(s,10);
			if(s[0]!=0)
				bidx[c].maxmsgs=atoi(s);
			break;
		case '4':
			nl();
			nl();
			put("1Enter entry level:2 ");
			input(s,50);
			if(s[0]!=0)
				bidx[c].level=atoi(s);
			break;
		case '5':
			nl();
			nl();
			put("1Enter new password [4C/R=None1]:2 ");
			input(s,50);
			if(s[0]!=0)
				strcpy(bidx[c].pass,s);
			else
				bidx[c].pass[0]=0;
			break;
		case 'Q':
		case 'q':
			return;
		}
	}
}


delboard()
{
char s[80],k[80],o[80];
int i,c,msgs;
long u;

i=0;
print("1Delete which board:2 ");
input(s,50);
c=atoi(s);
c--;
if (c <= numread && s[0]!=0)
	{
	i=c;
	strcpy(o,config.supportdir);
	itoa(curconf+1,k,10);
	strcat(s,k);
	strcat(o,"MSG");
	itoa(i+1,k,10);
	strcat(o,k);
	strcat(o,".HDR");
	fp=open(o,O_BINARY | O_RDWR);
	u=filelength(fp);
	msgs=u/sizeof(struct filestruct);
	config.messages=-msgs;
	writeconfig();
	remove(o);
	while(i < numread-1)
		{
		strcpy(s,config.supportdir);
		itoa(curconf+1,k,10);
		strcat(s,k);
		strcat(s,"MSG");
		itoa(i+2,k,10);
		strcat(s,k);
		strcat(s,".HDR");
		strcpy(o,config.supportdir);
		itoa(curconf+1,k,10);
		strcat(s,k);
		strcat(o,"MSG");
		itoa(i+1,k,10);
		strcat(o,k);
		strcat(o,".HDR");
		rename(s,o);
		bidx[i]=bidx[i+1];
		bidx[i].num=i+1;
		i++;
		}
	strcpy(s,config.supportdir);
	itoa(curconf+1,k,10);
	strcat(s,k);
	strcat(s,"MSG");
	itoa(numread,k,10);
	strcat(s,k);
	strcat(s,".HDR");
	remove(s);
	numread--;
	}
cur=0;
}



addboard()
{
char s[80],k[80],o[80];
int i,c,goon;

i=c=0;
goon=1;
print("1Insert before which board:2 ");
input(s,50);
c=atoi(s);
if(c <= numread+1 && s[0]!=0 && c!=0)
	{
	if(c != numread+1)
		{
		i=numread-1;
		c--;
		while(i != c-1)
			{
			strcpy(s,config.supportdir);
			itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(s,"MSG");
			itoa(i+2,k,10);
			strcat(s,k);
			strcat(s,".HDR");
			strcpy(o,config.supportdir);
			itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(o,"FILE");
			itoa(i+1,k,10);
			strcat(o,k);
			strcat(o,".HDR");
			rename(o,s);
			bidx[i+1]=bidx[i];
			bidx[i+1].num=i+2;
			i--;
			}
		}
	else
		c=numread;
	numread++;
	while(goon)
		{
		nl();nl();
		put("1Enter new name:2 ");
		input(s,23);
		if(s[0]!=0)
			{
			strcpy(bidx[c].name,s);
			goon=0;
			}
		}
	nl();
	put("1Networked [4y/N1]:2 ");
	if(ny())
		bidx[c].net=0;
	else
		bidx[c].net=1;
	nl();
	put("1Enter password [4C/R=None1]:2 ");
	input(s,50);
	if(s[0]==0)
		bidx[c].pass[0]=0;
	else
		strcpy(bidx[c].pass,s);
	goon=1;
	while(goon)
		{
		nl();
		put("1Enter entry level:2 ");
		input(s,50);
		if(s[0]!=0)
			{
			bidx[c].level=atoi(s);
			goon=0;
			}
		}
	goon=1;
	while(goon)
		{
		nl();
		put("1Enter max messages:2 ");
		input(s,50);
		if(s[0]!=0)
			{
			bidx[c].maxmsgs=atoi(s);
			goon=0;
			}
		}
	bidx[c].num=c+1;
	}
}