#include <stdio.h>

main()
{
char a[80], b[80], c[80], d[80], e[80], f[80], g[80], h[80], i[80], j[80], k[80], l[80];
FILE *fp;

fp=fopen("TRANSFER.LOG","r");

while(feof(fp)==0)
	{
	fscanf(fp,"%s %s %s %s %s %s %s %s %s %s %s %s",a,b,c,d,e,f,g,h,i,j,k,l);
	printf("\n%s\n%d\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",a,atoi(b),c,d,e,f,g,h,i,j,k,l);
	}
fclose(fp);
}