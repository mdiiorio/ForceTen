#include <time.h>
#include <stdio.h>

main()
{
time_t tstart,tnow;
int beeponce=0,c;
int tused=0,tbuf=0;

time(&tstart);

while(c!='Q')
	{
	beeponce=0;
	tbuf=difftime(time(&tnow),tstart);
	tused=tused+tbuf;
	if(60-tused<=0)
		{
		printf("\n\nTime Used up, call tommorow...");
		exit(255);
		}
	printf("\n[%i][Main Menu]:",60-tused);
	time(&tstart);
	while(!kbhit())
		{
		tbuf=difftime(time(&tnow),tstart);
		if(tbuf==10)
			{
			if(!beeponce)
				{
				printf("\a");
				beeponce=1;
				}
			}
		if(tbuf==20)
			{
			printf("Too bad...");
			exit(255);
			}
		}
	c=getch();
	c=upcase(c);
	putchar(c);
	}
}

upcase(int c)
{
if((c > '`') && (c < '{'))
	c=c-32;
return(c);
}