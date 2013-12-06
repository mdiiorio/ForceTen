#include <dos.h>

int sec=0;
int min=0;
int hour=0;
void interrupt (*oldtimer)();
void interrupt newtimer();
extern void interrupt (*ZeroDivVector)();
static union REGS rg;
unsigned vseg;
int running=0;
char bf[20];
unsigned v;
int ticker=0;
static struct SREGS seg;

init_timer()
{
oldtimer=getvect(0x1c);
setvect(0x1c,newtimer);
setvect(0,ZeroDivVector);
}

remove_timer()
{
setvect(0x1c,oldtimer);
}

void interrupt newtimer()
{
(*oldtimer)();
if(running==0)
	{
	running=1;
	if(ticker==0)
		{
		ticker=(((sec%5)==0)?19:18);
		sec++;
		if(sec==60)
			{
			sec=0;
			min++;
			if(min==60)
				{
				min=0;
				hour++;
				if(hour==24)
					hour=0;
				}
			}
		}
	--ticker;
	running=0;
	}
}