/*=======------ Entry.C -----======*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <alloc.h>
#include <mem.h>
#include <string.h>
#include "twindow.h"
#include "keys.h"

#define FIELDCHAR '_'
int insert_mode=FALSE;
extern int helpkey;

/*------- Local proto's -----*/
void addfield(WINDOW *wnd, FIELD *fld);
void disp_field(WINDOW *wnd, char *bf, char *msk);
void data_value(WINDOW *wnd, FIELD *fld);
void insert_status(void);
int read_field(WINDOW *wnd, FIELD *fld);
void right_justify(char *s);
void right_justify_zero_fill(char *s);
int validate_date(char *s);
int endstroke(int c);
int spaces(char *c);

/*----- Init a template -----*/
void init_template(WINDOW *wnd)
{
FIELD *fld, *fl;
fld=FHEAD;
while(fld)
	{
	fl=fld->fnxt;
	free(fld);
	fld=fl;
	}
FHEAD=NULL;
}

/*------- establish a field in a template -------*/
FIELD *establish_field(WINDOW *wnd, int cl, int rw, char *msk, char *bf, int ty)
{
FIELD *fld;
if((fld=malloc(sizeof(FIELD)))==NULL)
	return NULL;
fld->fmask=msk;
fld->frow=rw;
fld->fcol=cl;
fld->fbuff=bf;
fld->ftype=ty;
fld->fprot=0;
fld->fnxt=fld->fprv=NULL;
fld->fvalid=NULL;
fld->fhelp=NULL;
fld->fhwin=NULL;
fld->flx=fld->fly=0;
addfield(wnd,fld);
return(fld);
}

/*------ Add a field to the end of the list -----*/
static void addfield(WINDOW *wnd,FIELD *fld)
{
if(FTAIL)
	{
	fld->fprv=FTAIL;
	FTAIL->fnxt=fld;
	}
FTAIL=fld;
if(!FHEAD)
	FHEAD=fld;
}

/*----- display a data field -----*/
static void disp_field(WINDOW *wnd,char *bf, char *msk)
{
while(*msk)
	{
	wputchar(wnd,*msk != FIELDCHAR ? *msk : *bf++);
	msk++;
	}
}

/*------- display the data value in a field -----*/
static void data_value(WINDOW *wnd, FIELD *fld)
{
wcursor(wnd,fld->fcol,fld->frow);
disp_field(wnd,fld->fbuff,fld->fmask);
}

/*----- display all fields ----*/
void field_tally(WINDOW *wnd)
{
FIELD *fld;
fld=FHEAD;
while(fld != NULL)
	{
	data_value(wnd,fld);
	fld=fld->fnxt;
	}
}

/*---- set a field's help window ------*/
void field_window(FIELD *fld,char *hwin,int x,int y)
{
fld->fhwin=hwin;
fld->flx=x;
fld->fly=y;
}

/*------- clear template ------*/
void clear_template(WINDOW *wnd)
{
FIELD *fld;
char *bf,*msk;

fld=FHEAD;
while(fld != NULL)
	{
	bf=fld->fbuff;
	msk=fld->fmask;
	while(*msk)
		{
		if(*msk==FIELDCHAR)
			*bf++=' ';
		msk++;
		}
	fld=fld->fnxt;
	}
field_tally(wnd);
}


/*------ change cursor according to insert -----*/
static void insert_status()
{
set_cursor_type(insert_mode ? 0x0107 : 0x0607);
}

/*-------- read a field ------*/
static int read_field(WINDOW *wnd,FIELD *fld)
{
char *mask = fld->fmask, *buff = fld->fbuff;
int done=FALSE,c,column;

column=fld->fcol;
while(*mask != FIELDCHAR)
	{
	column++;
	mask++;
	}
while(TRUE)
	{
	wcursor(wnd,column,fld->frow);
	c=get_char();
	if(fld->ftype=='A')
		c=toupper(c);
	clear_message();
	switch(c)
		{
		case '\b':
		case   BS:
			if(buff==fld->fbuff)
				{
				done=c==BS;
				break;
				}
			--buff;
			do
				{
				--mask;
				--column;
				} while(*mask != FIELDCHAR);
			if(c==BS)
				break;
		case DEL:
    		movmem(buff+1,buff,strlen(buff));
			*(buff+strlen(buff)) = ' ';
			wcursor(wnd,column,fld->frow);
			disp_field(wnd,buff,mask);
			break;
		case FWD:
			do
				{
				column++;
				mask++;
				} while(*mask && *mask != FIELDCHAR);
			buff++;
			break;
		case INS:
			insert_mode ^= TRUE;
			insert_status();
			break;
		case '.':
			if(fld->ftype=='C')
				{
				if(*mask++ && *buff == ' ')
					{
					*buff++ = '0';
					if(*mask++ && *buff == ' ')
						*buff++ = '0';
					}
				right_justify(fld->fbuff);
				wcursor(wnd,fld->fcol,fld->frow);
				disp_field(wnd,fld->fbuff,fld->fmask);
				column=fld->fcol+strlen(fld->fmask)-2;
				mask=fld->fmask+strlen(fld->fmask)-2;
				buff=fld->fbuff+strlen(fld->fbuff)-2;
				break;
				}
		default:
			if(endstroke(c))
				{
				done=TRUE;
				break;
				}
			if(toupper(fld->ftype)!='A'&&!isdigit(c))
				{
				error_message("Numbers only");
				break;
				}
			if(insert_mode)
				{
				movmem(buff,buff+1,strlen(buff)-1);
				disp_field(wnd,buff,mask);
				wcursor(wnd,column,fld->frow);
				}
			*buff++=c;
         wputchar(wnd,c);
         do
         	{
            column++;
            mask++;
            } while(*mask && *mask != FIELDCHAR);
			if(!*mask)
				c=FWD;
			break;
		}
	if(!*mask)
		done=TRUE;
	if(done)
		{
		if(fld->ftype=='D' && c != ESC && validate_date(fld->fbuff) != OK)
			return ERROR;
		break;
		}
	}
if(c != ESC && toupper(fld->ftype) != 'A')
	{
	if(fld->ftype == 'C')
		{
		if(*mask++ && *buff == ' ')
			{
			*buff++='0';
			if(*mask++ && *buff == ' ')
				*buff++ = '0';
			}
		}
   if(fld->ftype == 'Z' || fld->ftype == 'D')
		right_justify_zero_fill(fld->fbuff);
	else
		right_justify(fld->fbuff);
	wcursor(wnd,fld->fcol,fld->frow);
	disp_field(wnd, fld->fbuff,fld->fmask);
	}
return c;
}

/*-------- test c -------*/
static int endstroke(int c)
{
switch(c)
	{
	case '\r':
	case '\n':
	case '\t':
	case  ESC:
	case   F1:
	case   F2:
	case   F3:
   case   F4:
   case   F5:
	case   F6:
	case   F7:
	case   F8:
	case   F9:
   case  F10:
	case PGUP:
	case PGDN:
	case HOME:
	case  END:
	case   UP:
	case   DN:
		return TRUE;
	default:
		return FALSE;
	}
}

/*------ right justify -----*/
static void right_justify(char *s)
{
int len;
len=strlen(s);
while(*s==' ' || *s == '0' && len)
	{
	len--;
	*s++ = ' ';
	}
if(len)
	while(*(s+(len-1)) == ' ')
		{
		movmem(s,s+1,len-1);
		*s = ' ';
		}
}

/*------ right justify zero fill ----*/
static void right_justify_zero_fill(char *s)
{
int len;
if(spaces(s))
	return;
len=strlen(s);
while(*(s+len-1) == ' ')
	{
	movmem(s,s+1,len-1);
	*s='0';
	}
}

/*------ Test for spaces -----*/
int spaces(char *c)
{
while(*c==' ')
	c++;
return !*c;
}

/*----- validate a date ------*/
static int validate_date(char *s)
{
int mo;
static int days [] =
	{ 31,28,31,30,31,30,31,31,30,31,30,31 };
char date[7];

strcpy(date,s);
if(spaces(date))
	return OK;
days[1]=(atoi(date+4)%4) ? 28 : 29;
*(date+4)='\0';
mo=atoi(date+2);
*(date+2)='\0';
if(mo && mo<13 && atoi(date) && atoi(date) <= days[mo-1])
	return OK;
error_message("Invalid date");
return ERROR;
}

/*------ process data entry -------*/
int data_entry(WINDOW *wnd)
{
FIELD *fld;
int exitcode,isvalid,done=FALSE,oldhelpkey=helpkey;
field_tally(wnd);
fld=FHEAD;

while(fld != NULL && done == FALSE)
	{
	helpkey=(fld->fhelp) ? 0 : oldhelpkey;
	wcursor(wnd,fld->fcol,fld->frow);
	if(fld->fprot == FALSE)
		{
		reverse_video(wnd);
		data_value(wnd,fld);
		wcursor(wnd,fld->fcol,fld->frow);
		exitcode=read_field(wnd,fld);
		isvalid=(exitcode != ESC && fld->fvalid) ? (*(fld->fvalid))(fld->fbuff) : OK;
		}
	else
		{
		exitcode=FWD;
		isvalid=OK;
		}
	if(isvalid==OK)
		{
		normal_video(wnd);
		data_value(wnd,fld);
		switch(exitcode)
			{
			case F1:
				if(fld->fhelp)
					{
					(*(fld->fhelp))(fld->fbuff);
					data_value(wnd,fld);
					}
			case   DN:
			case '\r':
			case '\t':
			case  FWD:
				fld=fld->fnxt;
				if(fld==NULL)
					fld=FHEAD;
				break;
			case UP:
			case BS:
				fld=fld->fprv;
				if(fld==NULL)
					fld=FTAIL;
				break;
			default:
				done=endstroke(exitcode);
				break;
			}
		}
	}
helpkey=oldhelpkey;
return(exitcode);
}

/*----- display a prompt ------*/
void wprompt(WINDOW *wnd,int x,int y,char *s)
{
wcursor(wnd,x,y);
wprintf(wnd,s);
}
