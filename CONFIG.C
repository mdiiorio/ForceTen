#include "twindow.h"


main()
{
WINDOW *main;

clrscr();
main=establish_window(20,10,10,30);
set_colors(main,ACCENT,BLUE,WHITE,DIM);
set_colors(main,NORMAL,BLACK,YELLOW,DIM);
set_colors(main,BORDER,BLACK,WHITE,DIM);
display_window(main);
wprintf(main,"  General\n");
wprintf(main,"  Directories\n");
wprintf(main,"  Miscellaneous\n");



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