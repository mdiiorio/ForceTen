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
extern struct fileindex fileidx[50];
extern struct confindex cidx[20];
extern struct bque que;


extern int baud,timeleft,cur,numread,curfile,curconf,confread;
extern char filepath[80];

long totalt;
int fp;

batchmenu()
{
char s[80];

while(1)
	{
	nl();
	setmci(1);
	print("4[1%d left4]:[1Batch4]:2 ",chktime()/60);
	setmci(0);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'L':
			listbatch();
			break;
		case 'C':
			clearbatch();
			break;
		case 'R':
			delbatfile();
			break;
		case 'D':
			downloadbatch();
			break;
		case 'U':
			batchupload();
			break;
		case 'Q':
			return;
		}
	}
}


delbatfile()
{
char s[80];
int i;

nl();
nl();

if(que.files==0)
	{
	pl("4No files in batch.");
	return;
	}

while(1)
	{
	put("1Delete which file from batch [4Name/Number1][4?/List1]:2 ");
	input(s,50);
	switch(s[0])
		{
		case 0:
			return;
		case '?':
			listbatch();
			nl();
			break;
		default:
			i=atoi(s);
			if(i < 1 || i > que.files)
				{
				nl();
				pl("4Invalid file.");
				return;
				}
			else
				{
				delbfile(i);
				return;
				}
		}
	}
}

delbfile(int i)
{
int o;

if(i == que.files)
	{
	que.files--;
	return;
	}

o=i-1;

while(o < que.files-1)
	{
	que.fstruct[o]=que.fstruct[o+1];
	o++;
	}

que.files--;
}


clearbatch()
{
nl();
nl();
put("1Really clear batch [4y/N1]:2 ");
if(ny())
	{
	nl();
	pl("4Batch not cleared.");
	}
else
	{
	nl();
	pl("4Batch cleared.");
	que.files=0;
	}
}


listbatch()
{
char s[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT],*s1;
long d;
int i,totalfp;

i=totalt=totalfp=0;

if(que.files==0)
	{
	nl();
	pl("4No files in batch.");
	return;
	}


nl();
nl();
pl("2       Name      Cost    # DL      Uploader        Time4");
pl("   컴컴컴컴컴컴컴 컴컴컴 컴컴컴 컴컴컴컴컴컴컴컴� 컴컴컴컴�");

while(i < que.files)
	{
	d=((((que.fstruct[i].bytes)+127)/128)) * (1620.0) / ((baud));
	if(i < 9)
		print("%d. ",i+1);
	else
		print("%d.",i+1);
	fnsplit(que.fstruct[i].name,drive,dir,filename,ext);
	print("4[2%-8s",filename);
	print("%-3s",ext);
	if(fstruct.filepoints==0)
		print("4] [2Free");
	if(fstruct.status!=0 && que.fstruct[i].filepoints!=0)
		{
		print("4] [3 %-3d",que.fstruct[i].filepoints);
		totalfp+=que.fstruct[i].filepoints;
		}
	print("4] [3%-3d",que.fstruct[i].timesdl);
	print("4] [3 %-15s ",que.fstruct[i].uploader);
	sprintf(s,"%ld",d/60);
	totalt+=d/60;
	print("4] [3%-4s4]",s);
	nl();
	i++;
	}
nl();
print("Total: %d file(s)     %d                 %ld mins",que.files,totalfp,totalt);
nl();
}


downloadbatch()
{
char s[80],k[80];
int fp,i;

put_char(12);
strcpy(k,config.rootdir);
strcat(k,"BATCH.$$$");
fp=open(k,O_RDWR | O_TEXT | O_CREAT,S_IWRITE | S_IREAD);

for(i=0;i < que.files;i++)
	{
	nl();
	print("4Preparing 3%s4...",que.fstruct[i].name);
	strcpy(s,que.fstruct[i].path);
	strcat(s,"\\");
	strcat(s,que.fstruct[i].name);
	strcat(s,"\n");
	write(fp,s,strlen(s));
	print("Done!");
	}

close(fp);

nl();
nl();
nl();
title("Batch Download Protocols");
nl();
pl("4<2X4> Xmodem");
pl("4<2Y4> Ymodem");
pl("4<2G4> Ymodem-G");
pl("4<2Z4> Zmodem");
pl("4<2M4> Zmodem MobyTurbo");
pl("4<2Q4> Quit");
nl();
pl("3* Note: Zmodem is auto-recovery");
nl();
put("1Protocol [Z]:2 ");
input(s,50);
strupr(s);
nl();
ansic(4);
if(s[0]=='Q')
	return;

put_char(12);
listbatch();

nl();
if(totalt > chktime()/60)
	{
	nl();
	pl("4Not enought time to download.");
	return;
	}

nl();
nl();
remove("TRANSFER.LOG");
switch(s[0])
	{
	case 'X':
		sprintf(s," port %d speed %d est len %d sx @%s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'Y':
		sprintf(s," port %d speed %d est len %d sb -y @%s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'G':
		sprintf(s," port %d speed %d est len %d sb -g @%s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 0:
	case 'Z':
		sprintf(s," port %d speed %d est len %d sz -r @%s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'M':
		sprintf(s," port %d speed %d est len %d sz -m -r @%s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	}
checkvalidbatch();
}



checkvalidbatch()
{
char s[90], bytes[10], code[10], baud[10], bps[10], numcps[10], cps[10], numerror[10], error[10], stops[10], paksize[10], regnum[10],k[80];
char drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT];
FILE *bu;
int i,totalcps, totalerror,o,totalfiles,oldfps,totalfps;
struct userfile tempu;


totalcps=totalerror=totalfiles=totalfps=0;

bu=fopen("TRANSFER.LOG","r");
if(bu==NULL)
	{
	nl();
	pl("4Can't find log file!!");
	fclose(bu);
	return;
	}

oldfps=user.filepoints;

for(i=0;i < que.files || (feof(bu)==0);i++)
	{
	fscanf(bu,"%s %s %s %s %s %s %s %s %s %s %s %s",code,bytes,baud,bps,numcps,cps,numerror,error,stops,paksize,s,regnum);
	strupr(code);
	if(code[0] != 'L' && code[0] != 'E' && code[0] != 'Q')
		{
		user.filepoints -= fstruct.filepoints;
		user.num_dl++;
		user.k_dl+=fstruct.bytes;
		fstruct.timesdl++;
		writeuser();
		fnsplit(s,drive,dir,filename,ext);
		strcpy(s,filename);
		strcat(s,ext);
		o=read_file(s);
		writefiles(o);
		readuser(fstruct.uploader,&tempu);
		tempu.filepoints=+fstruct.filepoints;
		writeu(&tempu);
		totalfiles++;
		totalcps+=atoi(numcps);
		totalerror+=atoi(numerror);
		totalfps+=fstruct.filepoints;
		}
	}
fclose(bu);

put_char(12);
title("Transfer Stats");
print("4Average CPS :3 %d",totalcps/totalfiles);
nl();
print("4Baud        :3 %s",baud);
nl();
print("4Total Errors:3 %d",totalerror);
nl();
print("4Total Files :3 %d",totalfiles);
nl();
nl();
print("4Old FPS:  3%d",oldfps);
nl();
print("4Cost   :  3%d",totalfps);
nl();
print("4        컴컴컴");
nl();
print("4New FPS:  3%d",(oldfps-totalfps));
nl();
nl();
}





/*batchupload()
{
char s[80];
int i;
time_t tstart,tend;

nl();
title("Batch Upload");
nl();
print("4You will have to provide descriptions");
print("for any files not accounted for after");
print("the upload is over.");

que.files=0;

for(i=0;i < 21;i++)
	{
	print("1Enter filename #3%d:2 ",i+1);
	input(s,12);
	strupr(s);
	if(s[0]==0)
		i=50;
	else
		{
		strcpy(que.fstruct[i].name,s);
		put("1Enter description:2 ");
		input(s,40);
		if(s[0]==0)
			strcpy(que.fstruct[i].desc,"[- No Description Given -]");
		else
			strcpy(que.fstruct[i].desc,s);
		put("1Enter extended description [4y/N1]:2 ");
		if(ny())
			{
			que.fstruct[i].extdesc[0][0]=que.fstruct[i].extdesc[1][0]=que.fstruct[i].extdesc[2][0]=0;
			nl();
			}
		else
			getextdesc();
		nl();
		que.files++;
		}
	}
title("Batch Upload Protocols");
nl();
pl("4<2X4> Xmodem");
pl("4<2Y4> Ymodem");
pl("4<2G4> Ymodem-G");
pl("4<2Z4> Zmodem");
pl("4<2M4> Zmodem MobyTurbo");
pl("4<2Q4> Quit");
nl();
pl("3* Note: Zmodem is auto-recovery");
nl();
put("1Protocol [Z]:2 ");
input(s,50);
strupr(s);
nl();
ansic(4);
if(s[0]=='Q')
	return;
nl();
nl();
mkdir("C:\TC\BBS\!@#$%");
chdir("C:\TC\BBS\!@#$%");
remove("TRANSFER.LOG");
time(&tstart);
switch(s[0])
	{
	case 'X':
		sprintf(s," port %d speed %d est len %d rx",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'Y':
		sprintf(s," port %d speed %d est len %d rb -y",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'G':
		sprintf(s," port %d speed %d est len %d rb -g",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 0:
	case 'Z':
		sprintf(s," port %d speed %d est len %d rz -r",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'M':
		sprintf(s," port %d speed %d est len %d rz -m",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	}
/*char s[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT],*s1;
char k[80];
int goon,i;
long by,tfree,bpc;
struct ffblk fi,vol;
struct date date;
struct time time;
struct dfree df;

goon=1;
i=0;

if(findfirst("C:\TC\BBS\!@#$%",&fi,0)==-1)
	{
	nl();
	pl("4No files recieved.");
	close(fp);
	return;
	}

if(findbatchfile(fi.ff_name))
	{
	strupr(fi.ff_name);
	strcpy(fstruct.name,fi.ff_name);
	strcpy(fstruct.path,fileidx[cur].path);
   strcpy(s,fstruct.path);
	strcat(s,"\\");
	strcat(s,fstruct.name);
	fp=open(s,O_BINARY | O_RDWR);
	fstruct.bytes=filelength(fp);
	close(fp);
	user.timeall+=difftime(tend,tstart);
	timeleft+=difftime(tend,tstart);
	makepath();
	fp=open(filepath,O_BINARY | O_RDWR);
	lseek(fp,0l,SEEK_END);
	write(fp,&fstruct,sizeof(struct filestruct));
	close(fp);
	user.num_ul++;
	user.k_ul+=fstruct.bytes;
	fstruct.timesdl++;
	writeuser();
	nl();
	print("3Processing 3%s4...",fstruct.name);
	sprintf(k,"COMMAND /C COMMENT.BAT %s",s);
	system(k);
	print("Done!");
	}

while(findnext(&fi)==0)
	{
	if(ifspace())
		return;
	fnsplit(fi.ff_name,drive,dir,filename,ext);
	print("%-8s",filename);
	ext[0]=32;
	print("%-4s",ext);
	print("%9ld",fi.ff_fsize);
	nl();
	i++;
	}


findbatchfile(char *s)
{
int i;

for(i=0;i < que.files;i++)
	if(stricmp(que.fstruct[i].name,s)==0)
		return(1);

return(0);
}

chargebatchul()
{
strcpy(s,fstruct.path);
strcat(s,"\\");
strcat(s,fstruct.name);
fp=open(s,O_BINARY | O_RDWR);
fstruct.bytes=filelength(fp);
close(fp);
if(fstruct.bytes==-1)
	{
	put("4File isn't there!!");
	return;
	}
user.timeall+=difftime(tend,tstart);
timeleft+=difftime(tend,tstart);
makepath();
fp=open(filepath,O_BINARY | O_RDWR);
lseek(fp,0l,SEEK_END);
write(fp,&fstruct,sizeof(struct filestruct));
close(fp);
user.num_ul++;
user.k_ul+=fstruct.bytes;
fstruct.timesdl++;
writeuser();
nl();
print("3Processing 3%s4...",fstruct.name);
sprintf(k,"COMMAND /C COMMENT.BAT %s",s);
system(k);
print("Done!");*/
}*/


batchupload()
{
char s[80],k[80];
time_t tstart,tend;
struct ffblk fi;
int n,ok,c;

put_char(12);
title("Batch Upload Protocols");
pl("4<2Y4> Ymodem");
pl("4<2G4> Ymodem-G");
pl("4<2Z4> Zmodem");
pl("4<2M4> Zmodem MobyTurbo");
pl("4<2Q4> Quit");
nl();
pl("3* Note: Zmodem is auto-recovery");
nl();
put("1Protocol [Z]:2 ");
input(s,50);
strupr(s);
nl();
ansic(4);
if(s[0]=='Q')
	return;
nl();
nl();
strcpy(k,config.rootdir);
strcat(k,"TEMPULE");
mkdir(k);
chdir(k);
remove("TRANSFER.LOG");
time(&tstart);
switch(s[0])
	{
	case 'Y':
		sprintf(s," port %d speed %d est len %d rb -y",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'G':
		sprintf(s," port %d speed %d est len %d rb -g",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 0:
	case 'Z':
		sprintf(s," port %d speed %d est len %d rz -r",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'M':
		sprintf(s," port %d speed %d est len %d rz -m",config.port,baud,baud);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	}

time(&tend);

strcpy(s,config.rootdir);
strcat(s,"TEMPULE");
strcat(s,"\\*.*");

if(findfirst(s,&fi,0)==-1)
	{
	nl();
	pl("4No files recieved.");
	close(fp);
	return;
	}

strupr(fi.ff_name);

c=cur;
for(n=0,ok=1;n<numread;n++)
	{
	cur=n;
	if(getsfile(fi.ff_name)==1)
		{
		pl("4File already exists");
		if(fileidx[cur].level > user.sl)
			pl("4File may be in a non-accessible file base");
		ok=0;
		}
	}
cur=c;

if(ok)
	{
	strcpy(fstruct.name,fi.ff_name);
	strcpy(fstruct.path,fileidx[cur].path);
	strcpy(s,fstruct.path);
	strcat(s,"\\");
	strcat(s,fstruct.name);
	fp=open(s,O_BINARY | O_RDWR);
	fstruct.bytes=filelength(fp);
	close(fp);
	strcpy(fstruct.uploader,user.name);
	nl();
	print("4Input a description for 3%s",fstruct.name);
	nl();
	nl();
	put("1:2 ");
	input(fstruct.desc,40);
	nl();
	if(fstruct.desc[0]==0)
		strcpy(fstruct.desc,"[- No Description Given -]");
	put("1Enter extended description [4y/N1]:2 ");
	if(ny())
		{
		fstruct.extdesc[0][0]=fstruct.extdesc[1][0]=fstruct.extdesc[2][0]=0;
		nl();
		}
	else
		getextdesc();
	user.timeall+=difftime(tend,tstart);
	timeleft+=difftime(tend,tstart);
	makepath();
	fp=open(filepath,O_BINARY | O_RDWR);
	lseek(fp,0l,SEEK_END);
	write(fp,&fstruct,sizeof(struct filestruct));
	close(fp);
	user.num_ul++;
	user.k_ul+=fstruct.bytes;
	fstruct.timesdl++;
	writeuser();
	nl();
	strcpy(s,config.rootdir);
	strcat(s,"TEMPULE\\");
	strcat(s,fstruct.name);
	print("4Processing 3%s4...",fstruct.name);
	sprintf(s," /C COMMENT.BAT %s",s);
	spawnlp(P_WAIT,"COMMAND.COM","COMMAND.COM",s,NULL);
	print("Done!");
	nl();
	strcpy(s,config.rootdir);
	strcat(s,"TEMPULE");
	print("4Moving to area 3%d4...",cur+1);
	sprintf(k,"COPY %s\\%s %s",s,fstruct.name,fstruct.path);
	system(k);
	sprintf(k,"DEL C:\\TC\\BBS\\TEMPULE\\%s",fstruct.name);
	system(k);
	print("Done!");
	nl();
	nl();
	}


while(findnext(&fi)==0)
	{
	strupr(fi.ff_name);
	c=cur;
	for(n=0,ok=1;n<numread;n++)
		{
		cur=n;
		if(getsfile(fi.ff_name)==1)
			{
			pl("4File already exists");
			if(fileidx[cur].level > user.sl)
				pl("4File may be in a non-accessible file base");
			ok=0;
			}
		}
	cur=c;
	if(ok)
		{
		strcpy(fstruct.name,fi.ff_name);
		strcpy(fstruct.path,fileidx[cur].path);
		strcpy(s,fstruct.path);
		strcat(s,"\\");
		strcat(s,fstruct.name);
		fp=open(s,O_BINARY | O_RDWR);
		fstruct.bytes=filelength(fp);
		close(fp);
		nl();
		print("4Input a description for 3%s",fstruct.name);
		nl();
		put("1:2 ");
		input(fstruct.desc,40);
		nl();
		if(fstruct.desc[0]==0)
			strcpy(fstruct.desc,"[- No Description Given -]");
		put("1Enter extended description [4y/N1]:2 ");
		if(ny())
			{
			fstruct.extdesc[0][0]=fstruct.extdesc[1][0]=fstruct.extdesc[2][0]=0;
			nl();
			}
		else
			getextdesc();
		user.timeall+=difftime(tend,tstart);
		timeleft+=difftime(tend,tstart);
		makepath();
		fp=open(filepath,O_BINARY | O_RDWR);
		lseek(fp,0l,SEEK_END);
		write(fp,&fstruct,sizeof(struct filestruct));
		close(fp);
		strcpy(fstruct.uploader,user.name);
		user.num_ul++;
		user.k_ul+=fstruct.bytes;
		fstruct.timesdl++;
		writeuser();
		nl();
		strcpy(s,config.rootdir);
		strcat(s,"TEMPULE\\");
		strcat(s,fstruct.name);
		print("4Processing 3%s4...",fstruct.name);
		sprintf(s," /C COMMENT.BAT %s",s);
		spawnlp(P_WAIT,"C:\COMMAND.COM","COMMAND.COM",s,NULL);
		print("Done!");
		nl();
		strcpy(s,config.rootdir);
		strcat(s,"TEMPULE");
		print("4Moving to area 3%d4...",cur+1);
		sprintf(k,"COPY %s\\%s %s",s,fstruct.name,fstruct.path);
		system(k);
		sprintf(k,"DEL C:\\TC\\BBS\\TEMPULE\\%s",fstruct.name);
		system(k);
		print("Done!");
		nl();
		nl();
		}
	chdir(config.rootdir);
	}
}

