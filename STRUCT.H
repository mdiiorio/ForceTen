#include <dos.h>

struct clrs
	{
	int fore,
		 back,
		 hilit,
		 blink;
	};

extern struct userfile
	{
	int  number;
	char name[40],
		  realname[50],
		  pass[15],
		  note[50],
		  macro[80],
		  phone[13],
		  votes[20];
	struct date firston;
	struct date laston;
	struct date newscan;
	struct time timelaston;
	struct clrs colors[10];
	int  sl,
		  fullscreen,
		  flags[10],
		  pause,
		  lines,
		  filesl,
		  gfilesl,
		  menutype,							/* 1=ASCII 2=ANSI 3=Enhacned ANSI */
		  filepoints,
		  exempt,							/* 1=No PCR 2=No File Pt. Check */
		  waiting,
		  hacks,
		  screenlines,
		  timeall,
		  timeon,
		  timebank,
		  logons,
		  del,
		  msgpost,
		  emailsent,
		  num_dl,
		  num_ul;
	long k_dl,
		  k_ul;
	}user;


extern struct configfile
	{
	char name[50],
		  sysop[30],
		  rootdir[30],
		  boarddir[30],
		  gfiledir[30],
		  supportdir[30],
		  string[30],
		  systempass[15],
		  syspass[15],
		  copass[15],
		  xpass[50],
		  lastcaller[60];
	int  matrix,
		  times[255],
		  xsec,
		  port,
		  baud,
		  mtype,
		  calls,
		  cosysop,
		  daysup,
		  callstoday,
		  fonline,
		  messages,
		  newuploads,
		  newmsgs;
	unsigned long kdl,
					  kul;
	}config;


extern struct boardindex
	{
	int num,
		 level,
		 maxmsgs,
		 net,
		 totalmsgs;
		 options;
	char name[50],
		  pass[50];
	}bidx[30];


/* nextpoint is the byte-pointer to next header Data - (null) if last  
 * datapoint is the byte-pointer to the cur. msg. text data
 */ 


extern struct headerinfo
	{
	int num,
		 prevrep,
		 lastrep,
		 nextpoint,
		 numreps;
	long datapoint;
	char title[50],
		  poster[50],
		  posternote[50],
		  to[50];
	struct date postdate;
	struct time posttime;
	}hdr;

extern struct fileindex
	{
	int num,
		 level,
		 ul,
		 dl;
	char name[50],
		  path[50],
		  pass[50];
	} fileidx[30];

extern struct confindex
	{
	int num,
		 level,
		 flags[10];
	char name[50],
		  pass[50];
	}cidx[10];


extern struct filestruct
	{
	long bytes;
	char name[20],
		  path[80],
		  uploader[50],
		  desc[80];
		  extdesc[3][80];
	int status,
		 timesdl,
		 filepoints;
	struct date dateul;
	struct time timeul;
	} fstruct;

extern struct bque
	{
	int files;
	struct filestruct fstruct[20];
	} que;

