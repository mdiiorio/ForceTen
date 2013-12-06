#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include "struct.h"

struct configfile config;

main()
{
int configp,i,c,k;

clrscr();
configp=open("CONFIG.BBS",O_RDWR | O_BINARY);

read(configp,&config,sizeof(struct configfile));
i=0;
for(c=0;c < 51;c++)
	{
	printf("\n");
	k=0;
	while(k < 5)
		{
		printf(" %d: %d ",i,config.times[i]);
		k++;
		i++;
		}
	}

close(configp);
}