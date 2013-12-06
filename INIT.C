#include <dos.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <conio.h>
#include "twindow.h"
#include "keys.h"
#include "struct.h"

static union REGS rg;
struct configfile config;
struct userfile user;

main()
{
if(access("CONFIG.BBS",0)==-1)
	initfiles();
conf();
}


initfiles()
{
int con,i,c;

con=open("CONFIG.BBS",O_RDWR | O_BINARY | O_CREAT,S_IWRITE);
strcpy(config.name,"Infinite Void");
strcpy(config.sysop,"Mr. Doo");
strcpy(config.rootdir,"C:\\TC\\BBS\\");
strcpy(config.boarddir,"C:\\TC\\BBS\\BOARDS\\");
strcpy(config.gfiledir,"C:\\TC\\BBS\\GFILES\\");
strcpy(config.supportdir,"C:\\TC\\BBS\\SUPPORT\\");
strcpy(config.string,"ATS0=1V0X3|");
strcpy(config.syspass,"Foogle Ball");
strcpy(config.copass,"Foogle Ball");
strcpy(config.xpass,"Systempro");
config.xsec=0;
config.matrix=1;
config.port=1;
config.baud=2400;
config.mtype=2;
config.cosysop=200;
config.daysup=0;
config.callstoday=0;
config.fonline=0;
config.messages=0;
config.newuploads=0;
config.newmsgs=0;
strcpy(config.systempass,"Banshee");
for(i=0;i < 255;i++)
	config.times[i]=70;
config.calls=0;
config.kul=0;
config.kdl=0;
strcpy(config.lastcaller,"Mr. Doo");
write(con,&config,sizeof(struct configfile));
close(con);
}



conf()
{
char s[80];
int c,fp,i;

fp=open("CONFIG.BBS",O_BINARY | O_RDWR);
read(fp,&config,sizeof(struct configfile));
close(fp);

while(1)
	{
	clrscr();
	printf("[0m");
	printf("[1m");
	printf("[31m              ***[33m> [36mDoo/2 Config [33m<[31m***");
	printf("\n\n");
	printf("[33m 1. BBS Name:           [37m%s\n",config.name);
	printf("[33m 2. Sysop Name:         [37m%s\n",config.sysop);
	printf("[33m 3. Root Directory:     [37m%s\n",config.rootdir);
	printf("[33m 4. Board Directory:    [37m%s\n",config.boarddir);
	printf("[33m 5. G-File Directory:   [37m%s\n",config.gfiledir);
	printf("[33m 6. Support Directory:  [37m%s\n",config.supportdir);
	printf("[33m 7. Init String:        [37m%s\n",config.string);
	printf("[33m 8. Com. Port:          [37m%d\n",config.port);
	printf("[33m 9. Maximum Baud Rate:  [37m%d\n",config.baud);
	if(config.matrix)
		printf("[33m 10. Matrix:            [37mYes\n");
	else
		printf("[33m 10. Matrix:            [37mNo\n");
	printf("[33m 11. Matrix Type:       [37m%d\n",config.mtype);
	printf("[33m 12. Co-Sysop Level:    [37m%d\n",config.cosysop);
	printf("[33m 13. Matrix Password:   [37m%s\n",config.systempass);
	printf("[33m 14. Sysop Password:    [37m%s\n",config.syspass);
	printf("[33m 15. Co-Sysop Password: [37m%s\n",config.copass);
	if(config.xsec)
		printf("[33m 16. Extra Security:    [37mYes\n");
	else
		printf("[33m 16. Extra Security:    [37mNo\n");
	printf("[33m 18. Extra Sec Pass:    [37m%s\n",config.xpass);
	printf("[33m  Q. Quit\n");
	printf("\n\n");
	printf("Selection: ");
	input(s,10,0);
	if(s[0]=='Q' || s[0]=='q')
		{
		printf("[0m");
		fp=open("CONFIG.BBS",O_BINARY | O_RDWR);
		write(fp,&config,sizeof(struct configfile));
		close(fp);
		exit();
		}
	c=atoi(s);
	switch(c)
		{
		case 1:
			printf("\n\nEnter new BBS name.\n");
			printf(":");
			lin(30);
			input(s,30,0);
			if(s[0]!=0)
				strcpy(config.name,s);
			break;
		case 2:
			printf("\n\nEnter new Sysop name.\n");
			printf(":");
			lin(30);
			input(s,30,0);
			if(s[0]!=0)
				strcpy(config.sysop,s);
			break;
		case 3:
			printf("\n\nEnter new Root Directory.\n");
			printf(":");
			lin(30);
			input(s,30,1);
			if(s[0]!=0)
				strcpy(config.rootdir,s);
			break;
		case 4:
			printf("\n\nEnter new Board Directory.\n");
			printf(":");
			lin(30);
			input(s,30,1);
			if(s[0]!=0)
				strcpy(config.boarddir,s);
			break;
		case 5:
			printf("\n\nEnter new G-File Directory.\n");
			printf(":");
			lin(30);
			input(s,30);
			if(s[0]!=0)
				strcpy(config.gfiledir,s);
			break;
		case 6:
			printf("\n\nEnter new Support Directory.\n");
			printf(":");
			lin(30);
			input(s,30);
			if(s[0]!=0)
				strcpy(config.supportdir,s);
			break;
		case 7:
			printf("\n\nEnter new Init string.  | for CR, ~ for pause\n");
			printf(":");
			lin(30);
			input(s,30);
			if(s[0]!=0)
				strcpy(config.string,s);
			break;
		case 8:
			i=1;
			printf("\n\nComm Ports availible: 1,2.\n");
			printf("\nPick comm port: ");
			while(i)
				{
				c=get_char();
				if(c == '1' || c == '2')
					i=0;
				}
			putchar(c);
			config.port=c-48;
			break;
		case 9:
			i=1;
			printf("\n\n1. 1200  2. 2400  3. 4800  4. 9600  5. 19200\n");
			printf("\nEnter Baud rate: ");
			while(i)
				{
				c=get_char();
				if(c >= 49 && c <= 53)
					i=0;
				}
			putchar(c);
			c=c-48;
			switch(c)
				{
				case 1:
					config.baud=1200;
					break;
				case 2:
					config.baud=2400;
					break;
				case 3:
					config.baud=4800;
					break;
				case 4:
					config.baud=9600;
					break;
				case 5:
					config.baud=19200;
					break;
				}
			break;
		case 10:
			i=1;
			printf("\n\nHave a Matrix? ");
			while(i)
				{
				c=get_char();
				c=toupper(c);
				if(c=='Y' || c=='N')
					i=0;
				}
			if(c == 'Y')
				{
				printf("Yes");
				config.matrix=1;
				}
			else
				{
				printf("No");
				config.matrix=0;
				}
			break;
		case 11:
			i=1;
			printf("\n\n1. DOS/TCS Type  2. Regular\n");
			printf("\nMatrix type: ");
			while(i)
				{
				c=get_char();
				if(c=='1' || c == '2')
					i=0;
				}
			putchar(c);
			config.mtype=c-48;
			break;
		case 12:
			c=1;
			printf("\n\nEnter Co-Sysop level (0-32767): ");
			while(c)
				{
				lin(4);
				input(s,5,0);
				if(s[0]!=0)
					{
					config.cosysop=atoi(s);
					c=0;
					}
				}
			break;
		case 13:
			printf("\n\nEnter new Matrix password.\n");
			printf(":");
			lin(30);
			input(s,30,0);
			if(s[0]!=0)
				strcpy(config.systempass,s);
			break;
		case 14:
			printf("\n\nEnter new Sysop password.\n");
			printf(":");
			lin(30);
			input(s,30,0);
			if(s[0]!=0)
				strcpy(config.syspass,s);
			break;
		case 15:
			printf("\n\nEnter new Co-Sysop password.\n");
			printf(":");
			lin(30);
			input(s,30,0);
			if(s[0]!=0)
				strcpy(config.copass,s);
			break;
		case 16:
			i=1;
			printf("\n\nHave Extra Security? ");
			while(i)
				{
				c=get_char();
				c=toupper(c);
				if(c=='Y' || c=='N')
					i=0;
				}
			if(c == 'Y')
				{
				printf("Yes");
				config.xsec=1;
				}
			else
				{
				printf("No");
				config.xsec=0;
				}
			break;
		case 17:
			printf("\n\nEnter new Extra Security password.\n");
			printf(":");
			lin(30);
			input(s,30,0);
			if(s[0]!=0)
				strcpy(config.xpass,s);
			break;
		}
	}
}


lin(int len)
{
int c;

printf("[37;44m");
for(c=0;c <= len;c++)
	putchar(' ');
for(c=0;c <= len;c++)
	putchar('\b');
}

input(char *buff,int len,int caps)
{
int c,k;
int n,i,f;
int done=0;
i=0;
while(!done)
	{
	c=get_char();            /* get a key, */
	if(c==-1)
		{
		buff[0]=0;
		return;
		}
	if(c == 32)
		{
		if(i == 0)
			c=0;
		}
	if(c > 31 && c < 127)
		{
		if(i < len)
			{
			if(caps)
				c=toupper(c);
			buff[i++]=c;
			putchar(c);
			}
		}
	switch (c)
		{
		case 27:
			for(k=i;k!=0;k--)
				putchar('\b');
			for(k=i;k!=0;k--)
				putchar(' ');
			for(k=i;k!=0;k--)
				putchar('\b');
			i=0;
			break;
		case 13:		/* end of input */
	      buff[i]=0;
			putchar(' ');
			done=1;
		case 8:		/* delete character */
			if(i > 0)
				{
				i--;
				putchar('\b');
				putchar(' ');
				putchar('\b');
				}
			break;
		}
	}
}


get_char()
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
	if(rg.h.al == 0)
		c = rg.h.ah | 128;
	else
		c = rg.h.al;
	break;
	}
return c;
}
