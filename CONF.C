#include "comm.h"
#include "struct.h"
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <stdio.h>
#include <ctype.h>


extern struct userfile user;
extern struct confindex cidx[10];
extern struct fileindex fileidx[30];
extern struct filestruct fstruct;
extern struct bque que;
extern int baud,timeleft,cur,numread,curfile,curconf,confread;
extern char filepath[80];

static int maxlist,fp;


confmenu()
{
char s[80];
int i,ok,goon;

sprintf(s,"%sCONF.IDX",config.supportdir);

i=0;
ok=goon=1;
put_char(12);

i=open(s,O_RDWR | O_BINARY);
if(filelength(i)==0)
	{
	unlink(s);
	initconffile();
	}

if(access(s,0)!=0)
	initconffile();

if(user.filesl < 200)
	{
	if(user.filesl < cidx[0].level)
		{
		nl();
		pl("4You don't have access to the first section.");
		return;
		}
	if(cidx[0].pass[0]!=0)
		{
		nl();
		put("1Enter conference password:2 ");
		input(s,50);
		if(stricmp(cidx[curconf].pass,s)!=0)
			{
			nl();
			pl("4Wrong password.");
			return;
			}
		}
	}

/*if(numconfs() < 1)
	{
	nl();
	pl("4You don't have access to the first section.");
	return;
	}*/


while(goon)
	{
	printconf();
	nl();
	if(user.filesl >= 200)
		put("4[!] for conference edit");
	nl();
	put("1Enter conference:2 ");
	input(s,80);

	if(s[0]=='!' && user.filesl >= 200)
		confedit();
	else
		goon=0;
	ok=1;
	while(i < 10 && ok)
		{
		if(cidx[atoi(s)-1].flags[i])
			if(!user.flags[i])
				ok=0;
		i++;
		}
	}

if(s[0]==0)
	{
	nl();
	return;
	}

if(atoi(s) > confread || atoi(s) == 0)
	{
	nl();
	pl("4Invalid conference.");
	return;
	}

if(!ok || user.filesl < cidx[atoi(s)-1].level)
	{
	nl();
	pl("4You don't have access to that conference.");
	return;
	}

curconf=atoi(s)-1;
if(cidx[curconf].pass[0]!=0)
	{
	nl();
	put("1Enter conference password:2 ");
	input(s,50);
	if(stricmp(cidx[curconf].pass,s)!=0)
		{
		nl();
		pl("4Wrong password.");
		return;
		}
	}
}



printconf()
{
int i,c,ok;

maxlist=i=c=0;
pl("1 #     Name                       Level      Flags");
pl("4컴컴 컴컴컴컴컴컴컴컴컴컴컴컴컴� 컴컴컴� 컴컴컴컴컴컴컴");
/*    [1 ] [ New Uploads             ] [ 255 ] [ 0123456789 ]  */
while(c < confread)
	{
	i=0;
	ok=1;
	while(i < 10 && ok)
		{
		if(cidx[c].flags[i])
			if(!user.flags[i])
				ok=0;
		i++;
		}
	if(ok && user.filesl >= cidx[c].level)
		{
		print("[2%-2d4] ",cidx[c].num);
		print("[3 %-23s 4] ",cidx[c].name);
		print("[3 %-3d 4] ",cidx[c].level);
		print("[ ");
		i=0;
		while(i < 10)
			{
			if(cidx[c].flags[i])
				{
				if(cidx[c].flags[i-1] && i!=0)
					print("%d",i);
				else
					print("2%d",i);
				}
			else
				{
				if(!cidx[c].flags[i-1] && i!=0)
					print("%d",i);
				else
					print("4%d",i);
				}
			i++;
			}
		put("4 ]");
		nl();
		}
	c++;
	}
}

initconffile()
{
char s[80],k[80];
int i,c;

confread=i=0;
strcpy(s,config.supportdir);
strcat(s,"CONF.IDX");

confread=0;
if(access(s,0)==0)
	{
	c=open(s,O_BINARY | O_RDWR);
	while(!eof(c))
		{
		read(c,&cidx[i],sizeof(struct confindex));
		i++;
		confread++;
		}
	close(c);
	return;
	}

c=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
strcpy(cidx[0].name,"General Access");
cidx[0].pass[0]=0;
cidx[0].num=1;
cidx[0].level=10;
while(i < 10)
	{
	cidx[0].flags[i]=0;
	i++;
	}
write(c,&cidx[0],sizeof(struct confindex));
close(c);
confread=1;
curconf=0;
}

confedit()
{
char s[80],k[80];
int i;

while(1)
	{
	put_char(12);
	printconf();
	nl();
	put("1[Q]uit, [M]odify, [D]elete, [I]nsert:2 ");
	input(s,80);
	strupr(s);
	switch(s[0])
		{
		case 'M':
			nl();
			print("1Enter conference to edit:2 ");
			input(s,50);
			if(s[0]!=0 && atoi(s) <= confread)
				editconf(atoi(s));
			break;
		case 'D':
			nl();
			delconf();
			break;
		case 'I':
			nl();
			addconf();
			break;
		case 'Q':
			strcpy(s,config.supportdir);
			strcat(s,"CONF.IDX");
			i=0;
			remove(s);
			fp=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
			while(i < confread)
				{
				write(fp,&cidx[i],sizeof(struct confindex));
				i++;
				}
			close(fp);
			nl();
			nl();
			put_char(12);
			return;
		}
	}
}



delconf()
{
char s[80],k[80],o[80];
int i,c,l;

i=l=0;
print("1Delete which conference:2 ");
input(s,50);
c=atoi(s);
c--;
if (c <= confread && s[0]!=0)
	{
	l=0;
	i=c;
	while(i < confread-1)
		{
		strcpy(cidx[i].name,cidx[i+1].name);
		cidx[i].level=cidx[i+1].level;
		cidx[i].num=i+1;
		l=0;
		while(l < 10)
			{
			cidx[i].flags[l]=cidx[i+1].flags[l];
			l++;
			}
		i++;
		}
	confread--;
	}
cur=0;
}



addconf()
{
char s[80],k[80],o[80];
int i,c,goon,l;

i=c=l=0;
goon=1;
print("1Insert before which conference:2 ");
input(s,50);
c=atoi(s);
if(c <= confread+1 && s[0]!=0 && c!=0)
	{
	if(c != confread+1)
		{
		i=confread-1;
		c--;
		while(i != c-1)
			{
			strcpy(cidx[i+1].name,cidx[i].name);
			cidx[i+1].num=i+2;
			cidx[i+1].level=cidx[i].level;
			while(i < 10)
				{
				cidx[i].flags[l]=cidx[i+1].flags[l];
				l++;
				}
			i--;
			}
		}
	else
		c=confread;
	confread++;
	while(goon)
		{
		nl();nl();
		put("1Enter new name:2 ");
		input(s,23);
		if(s[0]!=0)
			{
			strcpy(cidx[c].name,s);
			goon=0;
			}
		}
	goon=1;
	while(goon)
		{
		nl();
		put("1Enter entry level:2 ");
		input(s,50);
		nl();
		if(s[0]!=0)
			{
			cidx[c].level=atoi(s);
			goon=0;
			}
		}
	put("1Enter password [4C/R=None1]:2 ");
	input(s,50);
	if(s[0]!=0)
		strcpy(cidx[c].pass,s);
	else
		cidx[c].pass[0]=0;
	cidx[c].num=c+1;
	}
}


editconf(int c)
{
char s[80];
int goon,i;

c--;
while(1)
	{
	goon=1;
	put_char(12);
	print("2Conference number: %d",cidx[c].num);
	nl();
	nl();
	print("4[214] Name:3 %s",cidx[c].name);
	nl();
	print("4[224] Entry level:3 %d",cidx[c].level);
	nl();
	print("4[234] Password:3 %s",cidx[c].pass);
	nl();
	print("4[244] Flags: ");
	print("4[ ");
	i=0;
	while(i < 10)
		{
		if(cidx[c].flags[i])
			print("2%d",i);
		else
			print("4%d",i);
		i++;
		}
	put("4 ]");
	nl();
	print("4<2Q4> Quit");
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
				{
				strcpy(cidx[c].name,s);
				goon=0;
				}
			break;
		case '2':
			nl();
			nl();
			put("1Enter entry level:2 ");
			input(s,50);
			if(s[0]!=0)
				{
				cidx[c].level=atoi(s);
				goon=0;
				}
			break;
		case '3':
			nl();
			nl();
			put("1Enter new password:2 ");
			input(s,50);
			if(s[0]!=0)
				strcpy(cidx[c].pass,s);
			else
				cidx[c].pass[0]=0;
			break;
		case '4':
			goon=1;
			nl();
			while(goon)
				{
				nl();
				print("4[ ");
				i=0;
				while(i < 10)
					{
					if(cidx[c].flags[i])
						print("2%d",i);
					else
						print("4%d",i);
					i++;
					}
				put("4 ]");
				nl();
				print("1Enter flag to toggle:2 ");
				input(s,50);
				nl();
				i=atoi(s);
				if(s[0]==0)
					goon=0;
				if(i < 0 || i > 9 || !isdigit(s[0]))
					{
					nl();
					pl("4Invalid flag");
					}
				else if(s[0]!=0)
					{
					if(cidx[c].flags[i])
						cidx[c].flags[i]=0;
					else
						cidx[c].flags[i]=1;
					}
				}
			break;
		case 'Q':
		case 'q':
			return;
		}
	}
}


/*numconfs()
{
int c,i,ok,num;
c=i=maxlist=num=0;
ok=1;

while(c < confread)
	{
	while(i < 10 || !ok)
		{
		if(cidx[c].flags[i])
			if(!user.flags[i])
				ok=0;
		i++;
		}
	if(ok && user.filesl >= cidx[c].level)
		num++;
	c++;
	}
return(num);
}
	*/