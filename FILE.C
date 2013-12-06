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
extern struct fileindex fileidx[30];
extern struct confindex cidx[10];
extern struct bque que;
extern struct headerinfo hdr;


static int filefile,maxlist=0,fp;

extern int baud,timeleft,cur,numread,curfile,curconf,confread;
extern char filepath[80];

makepath()
{
char s[80],k[80];

strcpy(s,config.supportdir);
itoa(curconf+1,k,10);
strcat(s,k);
strcat(s,"FILE");
itoa(fileidx[cur].num,k,10);
strcat(s,k);
strcat(s,".LST");
strcpy(filepath,s);
}



filemenu()
{
char prompt[80],s[80],k[80];
int i;

cur=i=0;
initfile();
if(user.filesl < fileidx[0].level)
	{
	nl();
	nl();
	put("4You can't access the first section");
	return;
	}
if(fileidx[0].pass[0]!=0)
	{
	put("1Enter file base password:2 ");
	input(s,50);
	nl();
	if(stricmp(fileidx[0].pass,s)!=0)
		{
		pl("4Sorry, wrong password");
		return;
		}
	}


if(dispnoerr("FWELC.ANS")==-1)
	{
	put_char(12);
	put("4");
	print("ÖÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ·"); nl();
	print("º Conference:                         º"); nl();
	print("ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶"); nl();
	print("º UL K:            ³ DL K:            º"); nl();
	print("º UL #:            ³ DL #:            º"); nl();
	print("ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶"); nl();
	print("º File points:                        º"); nl();
	print("º File level:                         º"); nl();
	print("ÓÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ½"); nl();
	ccursor(2,15);
	print("3%d - %s",cidx[curconf].num,cidx[curconf].name);
	ccursor(4,9);
	print("2%d",user.k_ul/1000);
	ccursor(5,9);
	print("2%d",user.num_ul);
	ccursor(4,28);
	print("2%d",user.k_dl/1000);
	ccursor(5,28);
	print("2%d",user.num_dl);
	ccursor(7,16);
	print("2%d",user.filepoints);
	ccursor(8,15);
	print("2%d",user.filesl);
	nl();
	nl();
	nl();
	}


while(1)
	{
	nl();
	setmci(1);
	print("4[1%d left4 * [3Area %d4] 1File Menu4]:2 ",chktime()/60,fileidx[cur].num,fileidx[cur].name);
	setmci(0);
	input(s,50);
	strupr(s);
	switch(s[0])
		{
		case 'J':
			i=que.files;
			confmenu();
			que.files=i;
			filemenu();
			break;
		case 'N':
			newscan(1);
			break;
		case 'B':
			batchmenu();
			break;
		case 'Z':
			newscan(0);
			break;
		case '!':
			if(user.filesl > 200)
				filesysop();
			break;
		case '+':
			addbatch();
			break;
		case 'D':
			i=cur;
			download();
			cur=i;
			break;
		case 'U':
			i=cur;
			upload();
			cur=i;
			break;
		case 'X':
			extendedinfo();
			break;
		case 'S':
			i=cur;
			searchfiles();
			cur=i;
			break;
		case 'L':
			listfiles();
			break;
		case 'A':
			printdir();
			break;
		case 'V':
			viewzip();
			break;
		case 'Q':
			strcpy(s,config.supportdir);
			itoa(curconf+1,k,10);
			strcat(s,k);
			strcat(s,"FILE.IDX");
			i=0;
			filefile=open(s,O_BINARY | O_RDWR);
			while(i < numread)
				{
				write(filefile,&fileidx[i],sizeof(struct fileindex));
				i++;
				}
			close(filefile);
			topmenu();
			break;
		case 'G':
			logoff();
			break;
		case '?':
			do_help("FILE");
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

addbatch()
{
char s[80],k[80];
int i,totalfp;

nl();
nl();
put("1Enter file to add to batch [4Name/Number1]:2 ");
input(s,50);
if(read_file(s)==-1)
	{
	nl();
	return;
	}
else
	{
	strcpy(k,fstruct.path);
	strcat(k,"\\");
	strcat(k,fstruct.name);
	if(access(k,0)!=0)
		{
		nl();
		pl("4File is not on-line.");
		return;
		}
	if(fstruct.status==0)
		{
		nl();
		pl("4You may not download un-validated files.");
		return;
		}
	totalfp=0;
	for(i=0;i < que.files;i++)
		totalfp+=que.fstruct[i].filepoints;
	totalfp+=fstruct.filepoints;
	if(totalfp > user.filepoints)
		{
		nl();
		pl("4You do not have enough filepoints for this file.");
		return;
		}
	for(i=0;i < que.files;i++)
		{
		if(stricmp(que.fstruct[i].name,fstruct.name)==0)
			{
			nl();
			pl("4File already in batch.");
			return;
			}
		}
	if(que.files==20)
		{
		nl();
		pl("4Only 20 files allowed in a batch.");
		return;
		}
	que.fstruct[que.files]=fstruct;
	que.files++;
	nl();
	nl();
	print("3%s4 add to batch as file #3%d.",fstruct.name,que.files);
	nl();
	}
}


extendedinfo()
{
char s[80];

nl();
nl();
nl();
while(1)
	{
	put("1Which file [4Name/Number1][4?/List1]:2 ");
	input(s,50);
	switch(s[0])
		{
		case '?':
			listfiles();
			nl();
			nl();
			break;
		default:
			if(read_file(s)!=-1)
				xinfo();
			return;
		}
	}
}


xinfo()
{
char s[80];

put_char(12);
pl("4ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
print("³ Name: 3%-17s 4³",fstruct.name);
nl();
strcpy(s,fstruct.path);
strcat(s,"\\");
strcat(s,fstruct.name);
if(access(s,0)!=0)
	print("³ Size: 3Off-Line          4³");
else
	print("³ Size: 3%4dk             4³",fstruct.bytes/1000);
nl();
print("³ Times DL: 3%-13d 4³",fstruct.timesdl);
nl();
if(fstruct.filepoints==0)
	print("³ Cost: 3Free              4³");
else
	print("³ Cost: 3%-17d 4³",fstruct.filepoints);
nl();
if(fstruct.status==0)
	print("³ Status: 3New             4³");
else
	print("³ Status: 3Regular         4³");
nl();
print("³ U/Ler: 3%-16s 4³",fstruct.uploader);
nl();
if(fstruct.timeul.ti_hour<=12)
	print("³ Time UL: 3%02d:%02dam        4³",fstruct.timeul.ti_hour,fstruct.timeul.ti_min);
else
	print("³ Time UL: 3%02d:%02dpm        4³",fstruct.timeul.ti_hour-12,fstruct.timeul.ti_min);
nl();
print("³ Date UL: 3%02d/%02d/%d       4³",fstruct.dateul.da_mon,fstruct.dateul.da_day,fstruct.dateul.da_year-1900);
nl();
		pl("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
pl("ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
print("³ Desc: 3%-52s 4³",fstruct.desc);
nl();
pl("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
pl("ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
pl("³ 2Extended Description  4 ³");
pl("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");

if(fstruct.extdesc[0][0]!=0)
	{
	ansic(3);
	print("%s",fstruct.extdesc[0]);
	nl();
	print("%s",fstruct.extdesc[1]);
	nl();
	print("%s",fstruct.extdesc[2]);
	nl();
	}
else
	pl("3None.");nl();nl();
ansic(4);
}


viewzip()
{
char s[80];

nl();
nl();
title("Archive View");

while(1)
	{
	put("1Which file [4Name/Number1][4?/List1]:2 ");
	input(s,50);
	switch(s[0])
		{
		case '?':
			listfiles();
			nl();
			nl();
			break;
		default:
			if(read_file(s)!=-1)
				readzip();
			return;
		}
	}
}

readzip()
{
char s[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT],*s1;

fnsplit(fstruct.name,drive,dir,filename,ext);
if(stricmp(ext,".ZIP")!=0 && stricmp(ext,".ARC")!=0 && stricmp(ext,".LZH")!=0 && stricmp(ext,".PAK")!=0)
	{
	nl();
	put("1Cannot determine archive type, would you like to choose [4Y/n1]:2 ");
	if(!yn())
		return;
	nl();
	nl();
	pl("4Please choose an archive type:");
	nl();
	pl("[1] PkZip");
	pl("[2] PKArc");
	pl("[3] LHArc");
	pl("[4] Pak");
	nl();
	put("1[Choice]:2 ");
	input(s,50);
	switch(s[0])
		{
		case '1':
			dozip(1);
			break;
		case '2':
			dozip(2);
			break;
		case '3':
			dozip(3);
			break;
		case '4':
			dozip(4);
			break;
		}
	}
else
	{
	nl();
	nl();
	pl("4Auto-Determination of archive type:");
	put("-> 3");

	if(stricmp(ext,".ZIP")==0)
		{
		pl("PkZip");
		dozip(1);
		}
	if(stricmp(ext,".ARC")==0)
		{
		pl("PkArc");
		dozip(2);
		}
	if(stricmp(ext,".LZH")==0)
		{
		pl("LHArc");
		dozip(3);
		}
	if(stricmp(ext,".PAK")==0)
		{
		pl("Pak");
		dozip(4);
		}
	}
}


dozip(int i)
{
char s[80];

sprintf(s,"%s\\%s",fstruct.path,fstruct.name);

if(access(s,0)!=0)
	{
	nl();
	nl();
	pl("4This file is not on-line");
	return;
	}

switch(i)
	{
	case 1:
		sprintf(s,"PKUNZIP -v %s\\%s > %sARCLIST.$$$",fstruct.path,fstruct.name,config.gfiledir);
		break;
	case 2:
		sprintf(s,"PKUNPAK -vm %s\\%s > %sARCLIST.$$$",fstruct.path,fstruct.name,config.gfiledir);
		break;
	case 3:
		sprintf(s,"LHARC l %s\\%s > %sARCLIST.$$$",fstruct.path,fstruct.name,config.gfiledir);
		break;
	case 4:
		sprintf(s,"PAK V %s\\%s > %sARCLIST.$$$",fstruct.path,fstruct.name,config.gfiledir);
		break;
	}

put("4");
system(s);
dispfile("ARCLIST.$$$");
sprintf(s,"%sARCLIST.$$$",config.gfiledir);
unlink(s);
}







searchfiles()
{
char s[80],k[80],name[80],de[80],d[80],fe[80];
int i,n,file,desc,all,both,goon,len;

i=n=0;
goon=1;
nl();
nl();
pl("4Enter string to search for -");
put("1:2");
input(s,50);
if(s[0]==0)
	{
	nl();
	return;
	}
strupr(s);
nl();
nl();
pl("4Scanning methods:");
pl("4[214] Both filename and description");
pl("4[224] Only filename");
pl("4[234] Only description");
nl();
put("1Selection [1]:2 ");
input(k,50);
if(k[0]==0 || k[0]=='1')
	{
	both=1;
	file=desc=0;
	}
if(k[0]=='2')
	{
	file=1;
	desc=both=0;
	}
if(k[0]=='3')
	{
	desc=1;
	file=both=0;
	}
nl();
nl();
put("1Scan all dirs [4Y/n1]:2 ");
if(yn())
	all=1;
else
	all=0;
nl();
nl();
print("4Scanning for 3%s",s);
nl();
nl();
if(!all)
	{
	makepath();
	fp=open(filepath,O_BINARY | O_RDWR);
	len=filelength(fp);
	len=len/sizeof(struct filestruct);
	close(fp);
	if(user.sl >= fileidx[n].level)
		{
		print("4Scanning [3%d %s4]",fileidx[cur].num,fileidx[cur].name);
		nl();
		nl();
		goon=1;
		while(goon)
			{
			getfile(i);
			strcpy(name,fstruct.name);
			strcpy(de,fstruct.desc);
			strupr(name);
			strupr(de);
			if(both)
				{
				if(strstr(name,s)!=NULL || strstr(de,s)!=NULL)
					printinfo(i+1);
				}
			else if(desc)
				{
				if(strstr(de,s)!=NULL)
					printinfo(i+1);
				}
			else if(file)
				{
				if(strstr(name,s)!=NULL)
					printinfo(i+1);
				}
			if(ifspace())
				return;
			if(i == len-1 || len==0)
				goon=0;
			i++;
			}
		}
	return;
	}
while(n < numread)
	{
	cur=n;
	makepath();
	fp=open(filepath,O_BINARY | O_RDWR);
	len=filelength(fp);
	len=len/sizeof(struct filestruct);
	close(fp);
	nl();
	if(user.sl >= fileidx[n].level)
		{
		if(fileidx[n].pass[0]!=0)
			{
			put("1Enter file base password:2 ");
			input(fe,80);
			nl();
			if(stricmp(fileidx[n].pass,fe)!=0)
				{
				pl("4Sorry, wrong password");
				n++;
				}
			}
		}
	if(user.sl >= fileidx[n].level)
		{
		print("4Scanning [3%d %s4]",fileidx[cur].num,fileidx[cur].name);
		i=0;
		nl();
		nl();
		goon=1;
		while(goon)
			{
			getfile(i);
			strcpy(name,fstruct.name);
			strcpy(de,fstruct.desc);
			strupr(name);
			strupr(de);
			if(both)
				{
				if(strstr(name,s)!=NULL || strstr(de,s)!=NULL)
					printinfo(i+1);
				}
			else if(desc)
				{
				if(strstr(de,s)!=NULL)
					printinfo(i+1);
				}
			else if(file)
				{
				if(strstr(name,s)!=NULL)
					printinfo(i+1);
				}
			if(ifspace())
				return;
			if(i == len-1 || len==0)
				goon=0;
			i++;
			}
		}
	n++;
	}
}


printdir()
{
int i,c,e,k;

put_char(12);
maxlist=0;
i=0;
nl();
nl();
pl("1 #     Name                       Level    U/L     D/L");
pl("4ÄÄÄÄ ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ");
/*    [1 ] [ New Uploads             ] [ 255 ] [ Yes ] [ No  ]   */
while(i < numread)
	{
	if(user.filesl >= fileidx[i].level)
		{
		print("[2%-2d4] ",fileidx[i].num);
		print("[3 %-23s 4] ",fileidx[i].name);
		print("[3 %-3d 4] ",fileidx[i].level);
		if(fileidx[i].ul)
			put("[3 Yes 4] ");
		else
			put("[3 No  4] ");
		if(fileidx[i].dl)
			put("[3 Yes 4]");
		else
			put("[3 No  4]");
		nl();
		}
	i++;
	}
}



initfile()
{
char s[80],k[80];
int i;

numread=0;
i=curconf;
i++;
strcpy(s,config.supportdir);
itoa(i,k,10);
strcat(s,k);
strcat(s,"FILE.IDX");
i=0;

filefile=open(s,O_RDWR | O_BINARY);

if(filefile!=-1 && (filelength(filefile)!=0))
	{
	while(!eof(filefile))
		{
		read(filefile,&fileidx[i],sizeof(struct fileindex));
		i++;
		numread++;
		}
	close(filefile);
	return;
	}


filefile=open(s,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
strcpy(fileidx[0].name,"General");
strcpy(fileidx[0].path,"C:\\");
fileidx[0].ul=1;
fileidx[0].dl=1;
fileidx[0].num=1;
fileidx[0].level=10;
write(filefile,&fileidx[0],sizeof(struct fileindex));
close(filefile);
numread=1;
makepath();
fp=open(filepath,O_BINARY | O_RDWR | O_CREAT,S_IWRITE | S_IREAD);
close(fp);
}



getextdesc()
{
char s[80];
int cur;

cur=0;
nl();
nl();
put("1Enter extended description below (3 lines). \".\" to end");
nl();
ansic(2);
while(cur < 3)
	{
	input(fstruct.extdesc[cur],75);
	nl();
	if(fstruct.extdesc[cur][0]=='.')
		{
		if(cur==0)
			fstruct.extdesc[0][0]=fstruct.extdesc[1][0]=fstruct.extdesc[2][0]=0;
		if(cur==1)
			fstruct.extdesc[1][0]=fstruct.extdesc[2][0]=0;
		if(cur==2)
			fstruct.extdesc[2][0]=0;
		return;
		}
	cur++;
	}
}

 
newscan(int n)
{
char s[80],k[80];
int i;

nl();
nl();
print("1New-Scan since [4%02d/%02d/%2d1]:2 ",user.newscan.da_mon,user.newscan.da_day,user.newscan.da_year-1900);
input(s,50);
if(s[0]==0)
	{
	if(i)
		for(i=0;i < numread;i++)
			getnew(n);
	else
		getnew(cur);
	}
else
	{
	if(s[2]!='/' || s[5]!='/')
		{
		nl();
		pl("4Invalid Date");
		return;
		}
	if(!isdigit(s[0]) || !isdigit(s[1]) || !isdigit(s[3]) || !isdigit(s[4]) || !isdigit(s[6]) || !isdigit(s[7]))
		{
		nl();
		pl("4Invalid Date");
		return;
		}
	k[0]=s[0];
	k[1]=s[1];
	k[2]=0;
	user.newscan.da_mon=atoi(k);
	k[0]=s[3];
	k[1]=s[4];
	k[2]=0;
	user.newscan.da_day=atoi(k);
	k[0]=s[6];
	k[2]=s[7];
	k[3]=0;
	user.newscan.da_year=atoi(k)-1900;
	if(i)
		for(i=0;i < numread;i++)
			getnew(n);
	else
		getnew(cur);
	}
}


getnew(int i)
{
int j;
cur=i;
j=0;
nl();
nl();
print("4New-Scanning [3%d %s4]",fileidx[i].num,fileidx[i].name);
nl();
nl();
while(getfile(j) != -1)
	{
	if(user.newscan.da_year < fstruct.dateul.da_year)
		printinfo(j+1);
	else if(user.newscan.da_mon < fstruct.dateul.da_mon)
		printinfo(j+1);
	else if(user.newscan.da_day < fstruct.dateul.da_day)
		printinfo(j+1);
	j++;
	}
}

 

listfiles()
{
char s[80],k[80];
int i,start,end,goon,nonstop;
long by,len;

i=0;
makepath();
fp=open(filepath,O_BINARY | O_RDWR);
len=filelength(fp);
if(len==0)
	{
	close(fp);
	nl();
	pl("4No files in this directory.");
	return;
	}
nl();
nl();
title("List Files");
print("4There are 3%d4 files.",len/sizeof(struct filestruct));
nl();
put("1[4Q/Quit1][4CR=All1][4Range1]:2 ");
input(s,50);
if(s[0]=='Q' || s[0]=='q')
	{
	close(fp);
	return;
	}
else if(s[0]==0)
	{
	start=0;
	end=len/sizeof(struct filestruct);
	}
else if(s[1]=='-')
	{
	s[1]=0;
	start=atoi(s)-1;
	if(isdigit(s[2]))
		{
		i=2;
		while(isdigit(s[i]))
			{
			k[i-2]=s[i];
			i++;
			}
		k[i-2]=0;
		if(atoi(k) < start)
			{
			nl();
			pl("4Invalid Range.");
			}
		else
			end=atoi(k);
		}
	else
		end=len/sizeof(struct filestruct);
	}
else if(s[2]=='-')
	{
	s[2]=0;
	start=atoi(s)-1;
	if(isdigit(s[3]))
		{
		i=3;
		while(isdigit(s[i]))
			{
			k[i-3]=s[i];
			i++;
			}
		k[i-3]=0;
		if(atoi(k) < start)
			{
			nl();
			pl("4Invalid Range");
			}
		else
			end=atoi(k);
		}
	else
		end=len/sizeof(struct filestruct);
	}
else if(s[3]=='-')
	{
	s[3]=0;
	start=atoi(s)-1;
	if(isdigit(s[4]))
		{
		i=4;
		while(isdigit(s[i]))
			{
			k[i-4]=s[i];
			i++;
			}
		k[i-4]=0;
		if(atoi(k) < start)
			{
			nl();
			pl("4Invalid Range.");
			}
		else
			end=atoi(k);
		}
	else
		end=len/sizeof(struct filestruct);
	}
else if(s[0]=='-')
	{
	k[0]=s[1];
	k[1]=s[2];
	start=0;
	end=atoi(k)-1;
	}
else
	{
	start=atoi(s)-1;
	end=atoi(s);
	}
if(start >= len/sizeof(struct filestruct))
	{
	nl();
	nl();
	pl("4Invalid Range");
	nl();
	}
put_char(12);
print("4ÄÄÍÍ3] 4[1%d4] - 2%s 3[4ÍÍÄÄ",fileidx[cur].num,fileidx[cur].name);
nl();
nl();
pl("4ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
pl("4³    3Filename.Ext   Cost    Size    Description      (* = Ext. Description)4  ³");
pl("4ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");

/*        [DOO2100E.ZIP] [ 2  ] [ 324k] [*/

i=nonstop=0;
goon=1;
while(start < end)
	{
	getfile(start);
	printinfo(start+1);
	if(ifspace())
		{
		close(fp);
		return;
		}
	if(i==user.lines-5 && !nonstop)
		{
		while(goon)
			{
			nl();
			put("1[C/R] Continue, N]on-Stop, D]ownload, Q]uit, +] Add File, X]tended Info:2 ");
			input(k,5);
			strupr(k);
			if(k[0]=='D')
				download();
			else if(k[0]=='Q')
				{
				close(fp);
				return;
				}
			else if(k[0]=='+')
				addbatch();
			else if(k[0]=='X')
				extendedinfo();
			else if(k[0]=='N')
				nonstop=1;
			else
				goon=0;
			}
		put_char(12);
		print("4ÄÄÍÍ3] 4[1%d4] - 2%s 3[4ÍÍÄÄ",fileidx[cur].num,fileidx[cur].name);
		nl();
		nl();
		pl("4ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
		pl("4³    3Filename.Ext   Cost    Size    Description      (* = Ext. Description)4  ³");
		pl("4ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
		goon=1;
		i=0;
		}
	start++;
	i++;
	}
close(fp);
}



printinfo(int n)
{
long by;
char s[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT],*s1;
int c,e,i;

print("3%d.",n);
if(n < 10)
	put("  ");
else if(n < 100)
	put(" ");
fnsplit(fstruct.name,drive,dir,filename,ext);
print("4[3%-8s",filename);
print("%-3s",ext);
if(fstruct.status==0)
	print("4] [2New ");
else if(fstruct.filepoints==0)
	print("4] [2Free");
else if(fstruct.status!=0 && fstruct.filepoints!=0)
	print("4] [3 %-3d",fstruct.filepoints);
by=fstruct.bytes/1000;
strcpy(s,fstruct.path);
strcat(s,"\\");
strcat(s,fstruct.name);
if(access(s,0)!=0)
	print("4] [2 Ask ");
else
	print("4] [3%4dk",by);
print("4] [");
if(fstruct.extdesc[0][0]==0)
	put("  ");
else
	put("3* 4");
print("%-40s4]",fstruct.desc);
nl();
}


read_file(char *s)
{
int ok,num;

makepath();
fp=open(filepath,O_BINARY | O_RDWR);
num=filelength(fp)/sizeof(struct filestruct);
close(fp);

if(s[0]==0)
	return(-1);

if(getsfile(s)==-1)
	ok=0;
else
	ok=1;

if(ok==0)
	{
	if(atoi(s) > num || atoi(s) <= 0)
		ok=0;
	else
		{
		getfile(atoi(s)-1);
		curfile=atoi(s)-1;
		ok=1;
		}
	}

if(ok==0)
	{
	nl();
	pl("4Invalid file.");
	return(-1);
	}
return(curfile);
}

getfile(int n)
{
char s[80],k[80];
long o,len;

makepath();
fp=open(filepath,O_BINARY | O_RDWR);
len=filelength(fp);
if(n >= len/sizeof(struct filestruct))
	{
	close(fp);
	return(-1);
	}

o=0l + n;
o *= sizeof(struct filestruct);
if(lseek(fp,o,SEEK_SET)==-1)
	return(-1);
if(read(fp,&fstruct,sizeof(struct filestruct))==-1)
	return(-1);
close(fp);
}


getsfile(char *r)
{
char s[80],k[80];

curfile=0;
makepath();
fp=open(filepath,O_BINARY | O_RDWR);
while(eof(fp)==0)
	{
	read(fp,&fstruct,sizeof(struct filestruct));
	if(stricmp(fstruct.name,r)==0)
		{
		close(fp);
		return(1);
		}
	curfile++;
	}
close(fp);
return(-1);
}



download()
{
char s[80],k[80];
int num,ok,goon;
long buf;
struct userfile tempu;

goon=1;
makepath();
fp=open(filepath,O_BINARY | O_RDWR);
num=filelength(fp)/sizeof(struct filestruct);
close(fp);


if(!fileidx[cur].dl)
	{
	nl();
	pl("4Not allowed to download from this area.");
	return;
	}
if(num==0)
	{
	nl();
	pl("4No files in this directory.");
	return;
	}
nl();
nl();
while(goon)
	{
	put("1File to download [4Name/Number1][4?/List1]:2 ");
	input(s,50);
	if(s[0]=='?')
		{
		listfiles();
		nl();
		}
	else
		goon=0;
	}
if(s[0]==0)
	return;

/*if(getsfile(s)==-1)
	ok=0;
else
	ok=1;

if(ok==0)
	{
	if(atoi(s) > num || atoi(s) <= 0)
		ok=0;
	else
		{
		getfile(atoi(s));
		curfile=atoi(s)-1;
		ok=1;
		}
	}

if(ok==0)
	{
	nl();
	put("4Invalid file.");
	return;
	}*/

read_file(s);

strcpy(k,fstruct.path);
strcat(k,"\\");
strcat(k,fstruct.name);
if(access(k,0)!=0)
	{
	nl();
	pl("4File is not on-line.");
	return;
	}

if(user.filepoints < fstruct.filepoints)
	{
	nl();
	pl("4Not enough filepoints to download.");
	return;
	}

if(fstruct.status==0)
	{
	nl();
	pl("4Not allowed to download New files.");
	return;
	}


put_char(12);
nl();
title("Download Protocols");
nl();
pl("4<2X4> Xmodem");
pl("4<2Y4> Ymodem");
pl("4<2G4> Ymodem-G");
pl("4<2Z4> Zmodem");
pl("4<2M4> Zmodem MobyTurbo");
pl("4<2T4> Tmodem");
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
buf=((((fstruct.bytes)+127)/128)) * (1620.0) / ((baud));

if(chktime() < buf)
	{
	pl("4Not enough time for download.");
	return;
	}
put_char(12);
printfilestats();
nl();
nl();
strcpy(k,fstruct.path);
strcat(k,"\\");
strcat(k,fstruct.name);
remove("TRANSFER.LOG");
switch(s[0])
	{
	case 'X':
		sprintf(s," port %d speed %d est len %d sx %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'Y':
		sprintf(s," port %d speed %d est len %d sb -y %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'G':
		sprintf(s," port %d speed %d est len %d sb -g %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 0:
	case 'Z':
		sprintf(s," port %d speed %d est len %d sz -r %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'M':
		sprintf(s," port %d speed %d est len %d sz -m -r %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'T':
		sprintf(s," -B %d -P %d -F %s",baud,config.port,k);
		spawnlp(P_WAIT,"TMODEM.EXE","TMODEM.EXE",s,NULL);
		break;
	}

if(checkvalid())
	{
	user.filepoints -= fstruct.filepoints;
	user.num_dl++;
	user.k_dl+=fstruct.bytes;
	fstruct.timesdl++;
	writeuser();
	writefiles(curfile);
	readuser(fstruct.uploader,&tempu);
	user.filepoints=+fstruct.filepoints;
	writeu(&tempu);
	}
}


checkvalid()
{
char s[90], bytes[10], code[10], baud[10], bps[10], numcps[10], cps[10], numerror[10], error[10], stops[10], paksize[10], regnum[10];
FILE *bu;


bu=fopen("TRANSFER.LOG","r");
if(bu==NULL)
	{
	nl();
	pl("4Can't find log file!!");
	return(0);
	}
fscanf(bu,"%s %s %s %s %s %s %s %s %s %s %s %s",code,bytes,baud,bps,numcps,cps,numerror,error,stops,paksize,s,regnum);

strupr(code);

put_char(12);
pl("4Transfer status:");
print("4Code:3   %c",code[0]);
nl();
print("4CPS:3    %s",numcps);
nl();
print("4Baud:3   %s",baud);
nl();
print("4Errors:3 %s",numerror);
nl();
if(code[0] != 'L' && code[0] != 'E' && code[0] != 'Q')
pl   ("4Result:3 Succesful");
else
	pl("4Result:3 Non-Successful");
if(code[0] != 'L' && code[0] != 'E' && code[0] != 'Q')
	{
	fclose(bu);
	return(1);
	}
fclose(bu);
return(0);
}


printfilestats()
{
char s[80],drive[MAXDRIVE],dir[MAXDIR],filename[MAXFILE],ext[MAXEXT],*s1;
long d;

d=((((fstruct.bytes)+127)/128)) * (1620.0) / ((baud));

pl("2       Name         Cost   # DL      Uploader        Time4");
pl("   ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄÄ ÄÄÄÄÄÄ ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄÄÄ");
print("1. ");
fnsplit(fstruct.name,drive,dir,filename,ext);
print("4[2 %-8s",filename);
print("%-3s ",ext);
if(fstruct.filepoints==0)
	print("4][2 Free ");
if(fstruct.status!=0 && fstruct.filepoints!=0)
	print("4][3  %-3d ",fstruct.filepoints);
print("4][3  %-3d ",fstruct.timesdl);
print("4][3 %-15s ",fstruct.uploader);
sprintf(s,"%ld:%ld",d/60,(d%60)*1);
print("4][3 %-6s 4]",s);
nl();
if(fstruct.filepoints==0)
	print("Total:              Free                            %s",s);
else
	print("Total:               %-3d                            %s",fstruct.filepoints,s);
nl();
nl();
}

upload()
{
char s[80],path[80],k[80];
int n;
time_t tstart,tend;

n=0;
if(!fileidx[cur].ul)
	{
	nl();
	pl("4Not allowed to upload to this area!!");
	return;
	}
nl();
nl();
put("1Enter file to upload: ");
dl(12);
input(s,12);
strupr(s);
if(s[0]==0)
	return;
nl();
if(strchr(s,'*')!=NULL || strchr(s,'?')!=NULL)
	{
	put("4Invalid filename!!");
	return;
	}

while(n < numread)
	{
	cur=n;
	if(getsfile(s)==1)
		{
		pl("4File already exists");
		if(fileidx[cur].level > user.sl)
			pl("4File may be in a non-accessible file base");
		return;
		}
	n++;
	}
strcpy(fstruct.name,s);
put("1Enter a description:2 ");
input(fstruct.desc,40);
if(fstruct.desc[0]==0)
	strcpy(fstruct.desc,"[- No Description Given -]");
nl();
put("1Enter extended description [3y/N1]:2 ");
if(ny())
	{
	fstruct.extdesc[0][0]=fstruct.extdesc[1][0]=fstruct.extdesc[2][0]=0;
	nl();
	}
else
	getextdesc();
fstruct.status=0;
fstruct.filepoints=0;
strcpy(fstruct.uploader,user.name);
fstruct.timesdl=0;
strcpy(fstruct.path,fileidx[cur].path);
gettime(&fstruct.timeul);
getdate(&fstruct.dateul);
fstruct.bytes=0;

put_char(12);
nl();
title("Upload Protocols");
nl();
pl("4<2X4> Xmodem");
pl("4<2Y4> Ymodem");
pl("4<2G4> Ymodem-G");
pl("4<2Z4> Zmodem");
pl("4<2M4> Zmodem MobyTurbo");
pl("4<2T4> Tmodem");
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
strcpy(k,fstruct.path);
strcat(k,"\\");
strcat(k,fstruct.name);
remove("TRANSFER.LOG");
getcwd(path,80);
chdir(fstruct.path);
time(&tstart);
print("3%s 4uploading 3%s",user.name,fstruct.name);
ansic(4);
nl();
nl();

switch(s[0])
	{
	case 'X':
		sprintf(s," port %d speed %d est len %d rx %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'Y':
		sprintf(s," port %d speed %d est len %d rb -k %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'G':
		sprintf(s," port %d speed %d est len %d rb -g %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 0:
	case 'Z':
		sprintf(s," port %d speed %d est len %d rz %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'M':
		sprintf(s," port %d speed %d est len %d rz -m %s",config.port,baud,baud,k);
		spawnlp(P_WAIT,"DSZ.COM","DSZ.COM",s,NULL);
		break;
	case 'T':
		sprintf(s," -B %d -P %d -F %s",baud,config.port,k);
		spawnlp(P_WAIT,"TMODEM.EXE","TMODEM.EXE",s,NULL);
		break;
	}
chdir(path);
time(&tend);
if(checkvalid())
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
	}
}
