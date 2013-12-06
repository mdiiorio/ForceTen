#include "comm.h"
#include "struct.h"
#include <io.h>
#include <fcntl.h>
#include <dir.h>
#include <sys\stat.h>
#include <stdio.h>
#include <ctype.h>


extern struct fileindex fileidx[30];
extern struct filestruct fstruct;
extern struct userfile user;
extern struct confindex cidx[10];
extern struct bque que;
extern struct headerinfo hdr;
extern int baud,timeleft,cur,numread,curfile,curconf,confread;
extern char filepath[80];

#define MAXNUMLINES 1000
#define MAXSTRLEN 20

static int fp,filefile,linecnt;
char *sortbuf[MAXNUMLINES];

get_path(char *buff)
{
int goon=1,len;
while(goon)
	{
	nl();
	put("1Enter path:2 ");
	input(buff,30);
	len=strlen(buff);
	if((buff[len-1] == 47 || buff[len-1] == 92) && (len > 3))
		buff[len-1]=0;
	nl();
	if(access(buff,0)==-1)
		{
		nl();
		print("3%s1 does not exist, create [4Y/n1]:2 ",buff);
		if(yn())
			if(mkdir(buff)!=0)
				{
				nl();
				print("4Can't create 3%s",buff);
				nl();
				}
			else
				goon=0;
		}
	else
		goon=0;
	}
}

filesysop()
{
char s[80],k[80];
int c,i;

while(1)
	{
	nl();
	setmci(1);
	print("4[1%d left4 * 4[3Area %d4] 1File Sysop4]:2 ",chktime()/60,fileidx[cur].num);
	setmci(0);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'A':
			printdir();
			break;
		case 'W':
			addwild();
			break;
		case 'E':
			editfile();
			break;
		case 'D':
			editdirs();
			break;
		case 'Q':
			return;
		case 'U':
			delfile();
			break;
		case 'R':
			dodir();
			break;
		case 'S':
			sortarea();
			break;
		case '?':
			do_help("SYSOP");
			break;
		default:
			if(isdigit(s[0]))
				{
				i=atoi(s);
				if(i <= numread && i > 0 && user.filesl >= fileidx[i-1].level)
					{
					if(fileidx[i-1].pass[0]!=0)
						{
						nl();
						put("1Enter file base password:2 ");
						input(k,50);
						if(stricmp(fileidx[i-1].pass,k)!=0)
							{
							nl();
							pl("4Sorry, wrong password");
							}
						else
							cur=i-1;
						}
					else
						cur=i-1;
					}
				}
			break;
		}
	}
}

delfile()
{
char s[80],k[80];
int i,o,size,fk;
long len;
struct userfile tempu;

makepath();
fp=open(filepath,O_BINARY | O_RDWR);
len=filelength(fp);
close(fp);
nl();
nl();
i=1;
while(i)
	{
   put("1Enter file to delete [4Name/Number1][4?/List1]:2 ");
	input(s,50);
	o=atoi(s);
	if(s[0]==0 || o==0)
		return;
	if(s[0]=='?')
		listfiles();
	size=len/sizeof(struct filestruct);
/*	if(o > size)
		{
		pl("4Invalid file");
		return;
		}
	else
		i=0;*/
	if(read_file(s)!=-1)
		i=0;
	else
		{
		pl("4Invalid file");
		return;
		}
	}
nl();
print("1Take away %s's credit [4Y/n1]:2 ",fstruct.uploader);
if(yn())
	{
	if(stricmp(user.name,fstruct.uploader)!=0)
		{
		readuser(fstruct.uploader,&tempu);
		tempu.filepoints=-fstruct.filepoints;
		writeu(&tempu);
		}
	else
		{
		user.filepoints=-fstruct.filepoints;
		writeuser();
		}
	}
nl();
print("1Delete %s from disk [4Y/n1]:2 ",fstruct.name);
if(yn())
	{
	strcpy(s,fstruct.path);
	strcat(s,"\\");
	strcat(s,fstruct.name);
	remove(s);
	}
nl();
makepath();
fp=open(filepath,O_BINARY | O_RDWR);
strcpy(s,config.supportdir);
strcat(s,"TEMP.$$$");
fk=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
while(i+1<curfile+1)
	{
	read(fp,&fstruct,sizeof(struct filestruct));
	write(fk,&fstruct,sizeof(struct filestruct));
	i++;
	}
read(fp,&fstruct,sizeof(struct filestruct));
while(curfile+1<size)
	{
	read(fp,&fstruct,sizeof(struct filestruct));
	write(fk,&fstruct,sizeof(struct filestruct));
	curfile++;
	}
close(fp);
close(fk);
remove(filepath);
rename(s,filepath);
config.fonline--;
writeconfig();
}



editdirs()
{
char s[80],k[80];
int i;

while(1)
	{
	put_char(12);
	printdir();
	nl();
	put("1[Q]uit, [M]odify, [D]elete, [I]nsert:2 ");
	input(s,80);
	strupr(s);
	switch(s[0])
		{
		case 'M':
			nl();
			print("1Enter directory to edit:2 ");
			input(s,50);
			if(s[0]!=0 && atoi(s) <= numread)
				editdir(atoi(s));
			break;
		case 'D':
			nl();
			deldir();
			break;
		case 'I':
			nl();
			adddir();
			break;
		case 'Q':
			strcpy(s,config.supportdir);
         itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(s,"FILE.IDX");
			i=0;
			remove(s);
			filefile=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
			while(i < numread)
				{
				write(filefile,&fileidx[i],sizeof(struct fileindex));
				i++;
				}
			close(filefile);
			nl();
			nl();
			return;
		}
	}
}


writefiles(int n)
{
char s[80],k[80];
long o;
struct filestruct temp;

makepath();
fp=open(filepath,O_BINARY | O_RDWR);

o=0l + n;
o *= sizeof(struct filestruct);
lseek(fp,o,SEEK_SET);
write(fp,&fstruct,sizeof(struct filestruct));
close(fp);
}



editfile()
{
char s[80];
int c;

while(1)
	{
	nl();
	nl();
	put("1File to edit [4Name/Number1][4?/List1]:2 ");
	input(s,50);
	if(s[0]=='Q' || s[0]=='q')
		return;
	switch(s[0])
		{
		case '?':
			listfiles();
			break;
		default:
			if(read_file(s)!=-1)
				{
				editf();
				writefiles(curfile);
				}
			return;
		}
	}
}


editf()
{
char s[80],k[80],o[80];
int goon,len;

struct userfile tempu;



while(1)
	{
	goon=1;
	put_char(12);
	nl();
	print("4[214] Name: 3%s",fstruct.name);
	nl();
	print("4[224] Uploader: 3%s",fstruct.uploader);
	nl();
	if(fstruct.status==0)
		print("4[234] Status: 3New");
	else
		print("4[234] Status: 3Regular");
	nl();
	print("4[244] Cost: 3%d",fstruct.filepoints);
	nl();
	print("4[254] Path: 3%s",fstruct.path);
	nl();
	print("4[264] Description: 3%s",fstruct.desc);
	nl();
	print("4[274] Extended Description:");
	nl();
	if(fstruct.extdesc[0][0]==0)
		pl("None.");
	else
		{
		print("%s",fstruct.extdesc[0]);
		nl();
		print("%s",fstruct.extdesc[1]);
		nl();
		print("%s",fstruct.extdesc[2]);
		nl();
		}
	nl();
	print("4[2Q4] Quit");
	nl();
	nl();
	put("1[Choice]:2 ");
	input(s,50);
	switch(s[0])
		{
		case '1':
			nl();
			nl();
			sprintf(k,"%s\\%s",fstruct.path,fstruct.name);
			put("1Enter new name:2 ");
			input(s,50);
			if(s[0]!=0)
				{
				sprintf(o,"%s\\%s",fstruct.path,s);
				rename(k,o);
				strcpy(fstruct.name,s);
				}
			break;
		case '2':
			while(goon)
				{
				nl();
				print("1Uploader [3%s1]:2 ",user.name);
				input(s,50);
				if(s[0]==0)
					{
					strcpy(fstruct.uploader,user.name);
					goon=0;
					}
				else
					{
					if(ver_user(s)==1)
						{
						nl();
						print("1Take away %s's credit [4Y/n1]:2 ",fstruct.uploader);
						if(yn())
							{
							if(stricmp(user.name,fstruct.uploader)!=0)
								{
								readuser(fstruct.uploader,&tempu);
								tempu.filepoints=-fstruct.filepoints;
								writeu(&tempu);
								}
							else
								{
								user.filepoints=-fstruct.filepoints;
								writeuser();
								}
							}
						readuser(s,tempu);
						strcpy(fstruct.uploader,tempu.name);
						goon=0;
						}
					else
						print("4Can't find %s",s);
					}
				}
			break;
		case '3':
			if(fstruct.status==0)
				fstruct.status=1;
			else
				fstruct.status=0;
			break;
		case '4':
			nl();
			nl();
			print("1Enter amount of filepoints [4C/R=%d1]:2 ",fstruct.bytes/100000);
			input(s,50);
			if(s[0]==0)
				fstruct.filepoints=fstruct.bytes/100000;
			else
				fstruct.filepoints=atoi(s);
			break;
		case '5':
			nl();
			nl();
			print("1Enter path [4%s1]:2 ",fileidx[cur].path);
			input(s,50);
			if(s[0]==0)
				strcpy(fstruct.path,fileidx[cur].path);
			else
				strcpy(fstruct.path,s);
			len=strlen(fstruct.path);
			if(fstruct.path[len-1] == 47 || fstruct.path[len-1] == 92)
				fstruct.path[len-1] = 0;
			break;
		case '6':
			nl();
			nl();
			put("1Enter new description:2 ");
			input(s,40);
			if(s[0]!=0)
				strcpy(fstruct.desc,s);
			break;
		case '7':
			getextdesc();
			break;
		case 'Q':
		case 'q':
			return;
		}
	}
}


addwild()
{
char s[80],k[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT];
int i;
long by;
struct ffblk fi;
struct date date;
struct time time;

makepath();
fp=open(filepath,O_BINARY | O_RDWR);

nl();
nl();
print("1Enter path to search [4%s\\*.*1]:2 ",fileidx[cur].path);
input(s,50);
if(s[0]==0)
	{
	strcpy(s,fileidx[cur].path);
	strcat(s,"\\*.*");
	}
fnsplit(s,drive,dir,filename,ext);
if(findfirst(s,&fi,0)==-1)
	{
	nl();
	pl("4Can't find any matching files");
	close(fp);
	return;
	}



nl();
nl();
if(findfile(fi.ff_name)==0)
	{
	print("1Add %s [4Y/n/q1]:2 ",fi.ff_name);
	i=ynq();
	if(i==1)
		{
		by=fi.ff_fsize/1000;
		nl();
		nl();
		print("4Size:3 %dk  4Name:3 ",by);
		put(fi.ff_name);
		nl();
		nl();
		strcpy(fstruct.path,drive);
		strcat(fstruct.path,dir);
		getdate(&fstruct.dateul);
		gettime(&fstruct.timeul);
		fstruct.bytes=fi.ff_fsize;
		fstruct.timesdl=0;
		fstruct.status=1;
		strcpy(fstruct.name,fi.ff_name);
		addfile();
		lseek(fp,0l,SEEK_END);
		write(fp,&fstruct,sizeof(struct filestruct));
		nl();
		nl();
		}
	if(i==-1)
		{
		close(fp);
		return;
		}
	if(i==0)
		nl();
	}

while(findnext(&fi)==0)
	{
	if(findfile(fi.ff_name)==0)
		{
		print("1Add %s [4Y/n/q1]:2 ",fi.ff_name);
		i=ynq();
		if(i==1)
			{
			by=fi.ff_fsize/1000;
			nl();
			nl();
			print("4Size:3 %dk  4Name:3 ",by);
			put(fi.ff_name);
			nl();
			nl();
			strcpy(fstruct.path,drive);
			strcat(fstruct.path,dir);
			getdate(&fstruct.dateul);
			gettime(&fstruct.timeul);
			fstruct.bytes=fi.ff_fsize;
			fstruct.timesdl=0;
			fstruct.status=1;
			strcpy(fstruct.name,fi.ff_name);
			addfile();
			lseek(fp,0l,SEEK_END);
			write(fp,&fstruct,sizeof(struct filestruct));
			nl();
			nl();
			}
		if(i==-1)
			{
			close(fp);
			return;
			}
		if(i==0)
			nl();
		}
	}
close(fp);
}


dodir()
{
char s[80],p[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT];
struct ffblk fi;

nl();
nl();
print("1Enter path to search [4%s\\*.*1]:2 ",fileidx[cur].path);
input(s,50);
if(s[0]==0)
	{
	strcpy(s,fileidx[cur].path);
	strcat(s,"\\*.*");
	}

if(findfirst(s,&fi,0)==-1)
	{
	nl();
	put_char(12);
	nl();
	pl("4Can't find any matching files");
	return;
	}

sprintf(p,"DIR %s > %sDIRTEMP.$$$",s,config.gfiledir);
system(p);
sprintf(p,"%sDIRTEMP.$$$",config.gfiledir);
dispfile("DIRTEMP.$$$");
unlink(p);
nl();
nl();
}



findfile(char *s)
{
char d[80],k[80];
int i;
long by;
struct filestruct temp;

lseek(fp,0l,SEEK_SET);
while(eof(fp)==0)
	{
	read(fp,&temp,sizeof(struct filestruct));
	if(stricmp(temp.name,s)==0)
		return(1);
	}
return(0);
}

addfile()
{
char s[80];
int goon,len;
struct userfile tempu;

goon=1;
print("1Enter path [4%s1]:2 ",fstruct.path);
input(s,50);
if(s[0]==0)
	strcpy(fstruct.path,fstruct.path);
else
	strcpy(fstruct.path,s);
len=strlen(fstruct.path);
if(fstruct.path[len-1] == 47 || fstruct.path[len-1] == 92)
	fstruct.path[len-1] = 0;
while(goon)
	{
	nl();
	print("1Uploader [4%s1]:2 ",user.name);
	input(s,50);
	if(s[0]==0)
		{
		strcpy(fstruct.uploader,user.name);
		goon=0;
		}
	else
		{
		if(ver_user(s)==1)
			{
			readuser(s,&tempu);
			strcpy(fstruct.uploader,tempu.name);
			goon=0;
			}
		else
			print("4Can't find %s",s);
		}
	}
nl();
put("1Enter description:2 ");
input(fstruct.desc,40);
if(fstruct.desc[0]==0)
	strcpy(fstruct.desc,"[- No Description Given -]");
nl();
put("1Enter extended description [4y/N1]:2 ");
if(ny())
	{
	fstruct.extdesc[0][0]=fstruct.extdesc[1][0]=fstruct.extdesc[2][0]=0;
	nl();
	}
else
	getextdesc();
print("1Enter # of filepoints [4C/R=%d1]:2 ",fstruct.bytes/100000);
input(s,50);
if(s[0]==0)
	fstruct.filepoints=fstruct.bytes/100000;
else
	fstruct.filepoints=atoi(s);
if(stricmp(tempu.name,user.name))
	{
	user.k_ul+=fstruct.bytes;
	user.num_ul++;
	user.filepoints+=atoi(s);
	writeuser();
	}
else
	{
	user.k_ul+=fstruct.bytes;
	user.num_ul++;
	tempu.filepoints+=atoi(s);
	writeu(&tempu);
	}
config.fonline++;
writeconfig();
}


editdir(int c)
{
char s[80];

c--;
while(1)
	{
	put_char(12);
	print("2Directory number: %d",fileidx[c].num);
	nl();
	nl();
	print("4[214] Name:3 %s",fileidx[c].name);
	nl();
	print("4[224] Path:3 %s",fileidx[c].path);
	nl();
	print("4[234] Allow D/L's:3 ");
	if(fileidx[c].dl)
		put("Yes");
	else
		put("No");
	nl();
	print("4[244] Allow U/L's:3 ");
	if(fileidx[c].ul)
		put("Yes");
	else
		put("No");
	nl();
	print("4[254] Entry level:3 %d",fileidx[c].level);
	nl();
	print("4[264] Password:3 %s",fileidx[c].pass);
	nl();
	print("4[2Q4] Quit");
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
				strcpy(fileidx[c].name,s);
			break;
		case '2':
			get_path(fileidx[c].path);
			break;
		case '3':
			nl();
			nl();
			put("1Allow D/L's [4Y/n1]:2 ");
			if(yn())
				fileidx[c].dl=1;
			else
				fileidx[c].dl=0;
			break;
		case '4':
			nl();
			nl();
			put("1Allow U/L's [4Y/n1]:2 ");
			if(yn())
				fileidx[c].ul=1;
			else
				fileidx[c].ul=0;
			break;
		case '5':
			nl();
			nl();
			put("1Enter entry level:2 ");
			input(s,50);
			if(s[0]!=0)
				fileidx[c].level=atoi(s);
			break;
		case '6':
			nl();
			nl();
			put("1Enter new password [4C/R=None1]:2 ");
			input(s,50);
			if(s[0]!=0)
				strcpy(fileidx[c].pass,s);
			else
				fileidx[c].pass[0]=0;
			break;
		case 'Q':
		case 'q':
			return;
		}
	}
}


deldir()
{
char s[80],k[80],o[80];
int i,c,files;
long u;

i=0;
print("1Delete which directory:2 ");
input(s,50);
c=atoi(s);
c--;
if (c <= numread && s[0]!=0)
	{
	i=c;
	strcpy(o,config.supportdir);
	itoa(curconf+1,k,10);
	strcat(s,k);
	strcat(o,"FILE");
	itoa(i+1,k,10);
	strcat(o,k);
	strcat(o,".LST");
	fp=open(o,O_BINARY | O_RDWR);
	u=filelength(fp);
	files=u/sizeof(struct filestruct);
	config.fonline=-files;
	writeconfig();
	remove(o);
	while(i < numread-1)
		{
		strcpy(s,config.supportdir);
		itoa(curconf+1,k,10);
		strcat(s,k);
		strcat(s,"FILE");
		itoa(i+2,k,10);
		strcat(s,k);
		strcat(s,".LST");
		strcpy(o,config.supportdir);
		itoa(curconf+1,k,10);
		strcat(s,k);
		strcat(o,"FILE");
		itoa(i+1,k,10);
		strcat(o,k);
		strcat(o,".LST");
		rename(s,o);
		fileidx[i]=fileidx[i+1];
		fileidx[i].num=i+1;
		i++;
		}
	strcpy(s,config.supportdir);
	itoa(curconf+1,k,10);
	strcat(s,k);
	strcat(s,"FILE");
	itoa(numread,k,10);
	strcat(s,k);
	strcat(s,".LST");
	remove(s);
	numread--;
	}
cur=0;
}



adddir()
{
char s[80],k[80],o[80];
int i,c,goon;

i=c=0;
goon=1;
print("1Insert before which directory:2 ");
input(s,50);
c=atoi(s);
if(c <= numread+1 && s[0]!=0 && c!=0)
	{
	if(c != numread+1)
		{
		i=numread-1;
		c--;
		while(i != c-1)
			{
			strcpy(s,config.supportdir);
			itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(s,"FILE");
			itoa(i+2,k,10);
			strcat(s,k);
			strcat(s,".LST");
			strcpy(o,config.supportdir);
			itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(o,"FILE");
			itoa(i+1,k,10);
			strcat(o,k);
			strcat(o,".LST");
			rename(o,s);
			strcpy(fileidx[i+1].name,fileidx[i].name);
			strcpy(fileidx[i+1].path,fileidx[i].path);
			strcpy(fileidx[i+1].pass,fileidx[i].pass);
			fileidx[i+i].ul=fileidx[i].ul;
			fileidx[i+1].dl=fileidx[i].dl;
			fileidx[i+1].num=i+2;
			fileidx[i+1].level=fileidx[i].level;
			i--;
			}
		}
	else
		c=numread;
	numread++;
	while(goon)
		{
		nl();nl();
		put("1Enter new name:2 ");
		input(s,23);
		if(s[0]!=0)
			{
			strcpy(fileidx[c].name,s);
			goon=0;
			}
		}
	get_path(fileidx[c].path);
	nl();
	put("1Allow D/L's [4Y/n1]:2 ");
	if(yn())
		fileidx[c].dl=1;
	else
		fileidx[c].dl=0;
	nl();
	put("1Allow U/L's [4Y/n1]:2 ");
	if(yn())
		fileidx[c].ul=1;
	else
		fileidx[c].ul=0;
	nl();
	put("1Enter password [4C/R=None1]:2 ");
	input(s,50);
	if(s[0]==0)
		fileidx[c].pass[0]=0;
	else
		strcpy(fileidx[c].pass,s);
	goon=1;
	while(goon)
		{
		nl();
		put("1Enter entry level:2 ");
		input(s,50);
		if(s[0]!=0)
			{
			fileidx[c].level=atoi(s);
			goon=0;
			}
		}
	fileidx[c].num=c+1;
	strcpy(s,config.supportdir);
	itoa(curconf+1,k,10);
	strcat(s,k);
	strcat(s,"FILE");
	itoa(c+1,k,10);
	strcat(s,k);
	strcat(s,".LST");
	fp=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
	close(fp);
	}
}


sortarea()
{
int counter,nlines,rt;
long len;

makepath();
fp=open(filepath,O_BINARY | O_RDWR);
len=filelength(fp);
nlines=len/sizeof(struct filestruct);

if(nlines < 2)
	{
	nl();
	pl("4No files to sort.");
	close(fp);
	return;
	}

lseek(fp,0l,SEEK_SET);
nl();
print("4Sorting [3%d %s4]",fileidx[cur].num,fileidx[cur].name);
nl();
linecnt=0;
for(linecnt=0;linecnt<nlines;linecnt++)
	{
	getfile(linecnt);
	if(!(sortbuf[linecnt]=malloc(strlen(fstruct.name)+1)))
		{
		nl();
		pl("4Not enough memory to sort area.");
		close(fp);
		return;
		}
	strcpy(sortbuf[linecnt],fstruct.name);
	print("%s %s %s",sortbuf[linecnt-1],sortbuf[linecnt],fstruct.name);
	nl();
	}

close(fp);
for(counter=0;counter<nlines;counter++)
	{
	print("%s",sortbuf[counter]);
	nl();
	}

sortbuffer();

makepath();
rt=open(filepath,O_BINARY | O_RDWR);

for(counter=0;counter<linecnt;counter++)
	{
	print("%s",sortbuf[counter]);
	nl();
	getsfile(sortbuf[counter]);
	write(rt,&fstruct,sizeof(struct filestruct));
	}

close(fp);
}


sortbuffer()
{
int counter,haveexchanged;
char *temp;

do
	{
	haveexchanged=0;

	for(counter=1;counter<linecnt;counter++)
		if(strcmp(sortbuf[counter],sortbuf[counter-1]) < 0)
			{
			haveexchanged=1;
			temp=sortbuf[counter];
			sortbuf[counter]=sortbuf[counter-1];
			sortbuf[counter-1]=temp;
			}
	} while(haveexchanged);
}
