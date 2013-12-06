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
extern int curconf,confread;

sysopmenu()
{
char s[80];

while(1)
	{
	nl();
	print("4[1%d left4 * 1Sysop Menu4]:2 ",chktime()/60);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'U':
			useredit();
			break;
		case 'Q':
			return;
		}
	}
}

useredit()
{
char s[80];

nl();
nl();

while(1)
	{
	nl();
	put("1User to edit [4?/List1]:2 ");
	input(s,50);
	strupr(s);

	if(s[0]==0)
		{
		nl();
		return;
		}

	else if(s[0]=='?')
		{
		user_list();
		nl();
		}

	else if(ver_user(s)==1)
		{
		editu(s);
		nl();
		}

	else if(ver_user(s)!=1)
		{
		nl();
		pl("4User doesn't exist!");
		}
	}
}


editu(char *n)
{
char s[80];
struct userfile tempu;

readuser(n,&tempu);

while(1)
	{
	nl();
	print("4[1%d left4 * [3%s4] 1User Edit4]:2 ",chktime()/60,tempu.name);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'D':
			nl();
			put("1Delete the looser [4y/N1]:2 ");
			if(!ny())
				tempu.del=1;
			nl();
			break;

		case 'Q':
			writeu(&tempu);
			return;
		}
	}
}




