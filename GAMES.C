#include "struct.h"
#include "comm.h"
#include "keys.h"
#include <time.h>
#include <stdlib.h>

extern int timeleft;
int credits;

online_games()
{
char s[80];
put_char(12);
credits=0;

while(1)
	{
	nl();
	nl();
	title("Gambling Section");
	nl();
	pl("4[214] Enter Bank");
	pl("4[224] Craps");
	pl("4[2Q4] Quit");
	nl();
	print("4Credits:3 %d",credits);
	nl();
	nl();
	put("4[1Online Games4]:2 ");
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case '1':
			game_bank();
			break;
		case '2':
			pickcraps();
			break;
		case 'Q':
			nl();
			nl();
			if(credits > 0)
				{
				user.filepoints+=credits/50;
				print("4Your 3%d4 credits have been changed to 3%d4 filepoints",credits,credits/50);
				nl();
				nl();
				}
			credits=0;
			writeuser();
			return;
		}
	}
}



pickcraps()
{
char s[80];

while(1)
	{
	nl();
	nl();
	title("Craps Section");
	nl();
	pl("4[214] Play Craps");
	pl("4[2Q4] Quit");
	nl();
	print("4Credits:3 %d",credits);
	nl();
	nl();
	put("4[1Craps4]:2 ");
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case '1':
			craps();
			break;
		case 'Q':
			nl();
			nl();
			return;
		}
	}
}

craps()
{
char s[80];
int roll,i,roll2;

randomize();
nl();
nl();
put("1Enter bet [max. 5000]:2 ");
input(s,50);

if(s[0]==0)
	return;

i=atoi(s);
i=i/1;

if(i > credits)
	{
	nl();
	put("3You don't have that many credits");
	nl();
	return;
	}
if(i < 0)
	return;
if(i > 5000)
	return;

nl();
nl();
put("4Rolling dice..");
roll=random(12);
roll++;
print("3  %d4!",roll);
nl();
if(roll==7 || roll==11)
	{
	nl();
	nl();
	print("4You win 3%d4 credits!!",i*2);
	credits+=i;
	return;
	}
if(roll==2 || roll==3 || roll==12)
	{
	nl();
	nl();
	print("4Awww, too bad, you lost 3%d4 credits",i);
	credits-=i;
	return;
	}
nl();
print("4You only need one more 3%d4 to win!",roll);

while(1)
	{
	roll2=random(12);
	roll2++;
	nl();
	print("4Roll:3 %d4!",roll2);
	if(roll2==roll)
		{
		nl();
		nl();
		print("4You win 3%d4 credits!!",i*2);
		credits+=i;
		return;
		}
	if(roll2==7)
		{
		nl();
		nl();
		print("4Awww, too bad, you lost 3%d4 credits",i);
		credits-=i;
		return;
		}
	}
}








game_bank()
{
char s[80];
int t,i;
put_char(12);

while(1)
	{
	nl();
	nl();
	title("Game Bank");
	nl();
	pl("4[214] File Points -> Credits");
	pl("4[224] Credits     -> File Points");
	pl("4[234] Credits     -> Time");
	pl("4[2Q4] Quit");
	nl();
	print("4Credits:3 %d",credits);
	nl();
	print("4Time Left:3 %d",chktime()/60);
	nl();
	print("4File Points:3 %d",user.filepoints);
	nl();
	nl();
	put("4[1Game Bank4]:2 ");
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case '1':
			fptocred();
			break;
		case '2':
			credtofp();
			break;
		case '3':
			credtotime();
			break;
		case 'Q':
			nl();
			nl();
			return;
		}
	}
}

credtofp()
{
char s[80];
int i,t;

nl();
nl();
nl();
print("4You have 3%d4 credits",credits);
nl();
put("1Enter number of credits to exchange:2 ");
input(s,50);
if(s[0]!=0)
	{
	i=atoi(s);
	if(i > 0)
		{
		i=i/1;
		if(i > credits)
			{
			nl();
			nl();
			pl("3You don't have that many credits");
			return;
			}
		if(i > 10000)
			{
			nl();
			nl();
			pl("4That exceeds the limit on exchange");
			return;
			}
		credits-=i;
		user.filepoints+=i/50;
		}
	}
}

credtotime()
{
char s[80];
int i,t;

nl();
nl();
nl();
print("4You have 3%d4 credits",credits);
nl();
put("1Enter number of credits to exchange:2 ");
input(s,50);
if(s[0]!=0)
	{
	i=atoi(s);
	if(i > 0)
		{
		i=i/1;
		if(i > credits)
			{
			nl();
			nl();
			pl("3You don't have that many credits");
			return;
			}
		if(i > 10000)
			{
			nl();
			nl();
			pl("4That exceeds the limit on exchange");
			return;
			}
		user.timeall+=(i/100)*60;
		timeleft+=(i/100)*60;
		credits-=i;
		}
	}
}



fptocred()
{
char s[80];
int i,t;

nl();
nl();
nl();
print("4You have 3%d4 filepoints",user.filepoints);
nl();
put("1Enter number of filepoints to exchange:2 ");
input(s,50);
if(s[0]!=0)
	{
	i=atoi(s);
	if(i > 0)
		{
		i=i/1;
		if(i > user.filepoints)
			{
			nl();
			nl();
			pl("3You don't have that many filepoints");
			return;
			}
		if(i > 500)
			{
			nl();
			nl();
			pl("4That exceeds the limit on exchange");
			return;
			}
		credits+=i*50;
		user.filepoints-=i;
		}
	}
}


time_bank()
{
char s[80];
int t,i;

put_char(12);
while(1)
	{
	nl();
	nl();
	title("Time Bank");
	nl();
	pl("4[2D4]eposit time");
	pl("4[2W4]ithdrawl time");
	pl("4[2Q4]uit");
	nl();
	print("4Amount in Timebank:3 %d",user.timebank);
	nl();
	print("4Time left:3 %d",chktime()/60);
	nl();
	nl();
	put("4[1Time Bank4]:2 ");
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'D':
			deposit();
			break;
		case 'W':
			withdrawl();
			break;
		case 'Q':
			nl();
			nl();
			writeuser();
			return;
		}
	}
}


deposit()
{
char s[80];
int i,t;

nl();
nl();
nl();
put("1Enter amount to deposit:2 ");
input(s,3);
if(s[0]!=0)
	{
	i=atoi(s);
	if(i > 0)
		{
		i=i/1;
		t=chktime()/60;
		if(i > t)
			{
			nl();
			nl();
			pl("3You don't have that much time");
			return;
			}
		if(i > 500)
			{
			nl();
			nl();
			pl("4That exceeds the limit on deposit");
			return;
			}
		user.timebank+=i;
		timeleft-=i*60;
		user.timeall-=i*60;
		}
	}
}


withdrawl()
{
char s[80];
int i,t;

nl();
nl();
nl();
put("1Enter amount to withdrawl:2 ");
input(s,3);
if(s[0]!=0)
	{
	i=atoi(s);
	if(i > 0)
		{
		i=i/1;
		if(i > user.timebank)
			{
			nl();
			nl();
			pl("4You don't have that much in the bank");
			return;
			}
		user.timeall+=i*60;
		timeleft+=i*60;
		user.timebank-=i;
		}
	}
}
