/*--------------- Twindow.h ----------------------------*/
/*
 *         Uncomment this:
 */
#define FASTWINDOWS
/*
 *         for stacked windows
 */

/*------- Window Colors -------*/
#define RED	 	  4
#define GREEN    2
#define BLUE	  1
#define WHITE    (RED+GREEN+BLUE)
#define YELLOW   (RED+GREEN)
#define AQUA     (GREEN+BLUE)
#define MAGENTA  (RED+BLUE)
#define BLACK	  0
#define BRIGHT   8
#define DIM		  0

#define BORDER	 0
#define TITLE 	 1
#define ACCENT	 2
#define NORMAL  3
#define ALL		 4

#define TRUE	 1
#define FALSE	 0
#define ERROR  -1
#define OK	    0


/*------- Window control structs -------*/
typedef struct field
{
char *fmask;
char *fbuff;
char *fhwin;
int fprot;
int ftype;
int frow;
int fcol;
int flx, fly;
void (*fhelp)();
int  (*fvalid)();
struct field *fnxt;
struct field *fprv;
} FIELD;


typedef struct _wnd
{
char *_ws;
char *_tl;
int _wv;
int _hd;
int _wx;
int _wy;
int _ww;
int _wh;
int _wsp;
int _sp;
int _cr;
int btype;
int wcolor[4];
int _pn;
struct _wnd *_nx;
struct _wnd *_pv;
FIELD *_fh;
FIELD *_ft;
} WINDOW;

typedef struct w_menu
{
char *mname;
char **mselcs;
void (**func)();
} MENU;



#define SAV		 (wnd->_ws)
#define WTITLE	 (wnd->_tl)
#define COL		 (wnd->_wx)
#define ROW		 (wnd->_wy)
#define WIDTH	 (wnd->_ww)
#define HEIGHT  (wnd->_wh)
#define SCROLL  (wnd->_wsp)
#define SELECT  (wnd->_sp)
#define WCURS   (wnd->_cr)
#define WBORDER (wnd->wcolor[BORDER])
#define WTITLEC (wnd->wcolor[TITLE])
#define WACCENT (wnd->wcolor[ACCENT])
#define WNORMAL (wnd->wcolor[NORMAL])
#define PNORMAL (wnd->_pn)
#define BTYPE 	 (wnd->btype)
#define NEXT	 (wnd->_nx)
#define PREV    (wnd->_pv)
#define WCOLOR  (wnd->wcolor)
#define VISIBLE (wnd->_wv)
#define HIDDEN	 (wnd->_hd)
#define FHEAD	 (wnd->_fh)
#define FTAIL	 (wnd->_ft)

#define NW	  (wcs[wnd->btype].nw)
#define NE	  (wcs[wnd->btype].ne)
#define SE	  (wcs[wnd->btype].se)
#define SW	  (wcs[wnd->btype].sw)
#define SIDE  (wcs[wnd->btype].side)
#define LINE  (wcs[wnd->btype].line)

/*------- Function proto's and macros --------*/

/*------- General Purpose funcs and macs -----*/
void clear_screen(void);
int vmode(void);
void cursor(int, int);
void curr_cursor(int *,int *);
int cursor_type(void);
void set_cursor_type(int);
int get_char(void);
int scroll_lock(void);

#define vpoke(vseg,adr,chr) 	poke(vseg,adr,chr)
#define vpeek(vseg,adr)			peek(vseg,adr)

/* Uncomment after getting MASM 5.1
 * void vpoke(unsigned,unsigned,unsigned);
 * int vpeek(unsigned,unsigned);
 */

/*------- Window funcs and macs --------------*/
WINDOW *establish_window(int,int,int,int);
void set_border(WINDOW *,int);
void set_colors(WINDOW *,int,int,int,int);
void set_intensity(WINDOW *,int);
void set_title(WINDOW *,char *);
void display_window(WINDOW *);
void delete_window(WINDOW *);
void clear_window(WINDOW *);
void hide_window(WINDOW *);
void wprintf(WINDOW *, char *, ...);
void wputchar(WINDOW *,int);
void close_all(void);
void wcursor(WINDOW *,int x, int y);
void error_message(char *);
void clear_message(void);
int get_selection(WINDOW *,int,char *);

#define reverse_video(wnd)	 	wnd->wcolor[3]=wnd->wcolor[2]
#define normal_video(wnd)		wnd->wcolor[3]=wnd->_pn
#define rmove_window(wnd,x,y)	repos_wnd(wnd,x,y,0)
#define move_window(wnd,x,y)	repos_wnd(wnd,COL-x,ROW-y,0)
#define forefront(wnd) 			repos_wnd(wnd,0,0,1)
#define rear_window(wnd)		repos_wnd(wnd,0,0,-1)


/*------- Internal window funcs --------------*/
void accent(WINDOW *);
void deaccent(WINDOW *);
void scroll(WINDOW *,int);
void repos_wnd(WINDOW *,int, int, int);
void acline(WINDOW *, int);

#define accent(wnd) acline(wnd,WACCENT)
#define deaccent(wnd) acline(wnd,WNORMAL)
#define clr(bg,fg,in) ((fg)|(bg<<4)|(in))
#define vad(x,y) ((y)*160+(x)*2)

#ifdef FASTWINDOWS
#define cht(ch,at) (((ch)&255)|((at)<<8))
#define displ(w,x,y,c,a) vpoke(VSG,vad(x+COL,y+ROW),cht(c,a))
#define dget(w,x,y) vpeek(VSG,vad(x+COL,y+ROW))
#define verify_wnd(w) (*(w)=listtail)!=0
#else
void displ(WINDOW *wnd,int x,int y,int ch,int at);
#endif


/*------- Editor function --------------------*/
void text_editor(WINDOW *,char *,unsigned);

/*------- Menu function ----------------------*/
void menu_select(char *name,MENU *mn);

/*------- Help funcs -------------------------*/
void load_help(char *);
void set_help(char *,int,int);

/*------- Data entry funcs and macs ----------*/
void init_template(WINDOW *);
FIELD *establish_field(WINDOW *,int,int,char *,char *,int);
void clear_template(WINDOW *);
void field_tally(WINDOW *);
int data_entry(WINDOW *);
void wprompt(WINDOW *,int,int,char *);
void error_message(char *);
void clear_notice(void);
void field_window(FIELD *,char *, int, int);
#define field_protect(f,s)  f->fprot=s
#define field_help(f,h)		 f->fhelp=h
#define field_validate(f,v) f->fvalid=v
