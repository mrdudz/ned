
/* ned.c 	A simple four-function text editor
 *
 * ned is a simple text editor for those who can't be bothered with the
 * finickiness of vi and don't like waiting for emacs to drag its dripping
 * elephantine carcass into core just for the sake of making a quick edit
 * to a file.
 *
 * ned was originally written for MS-DOS because I needed a simple editor
 * that would work from a session CTTY'd down a COM port.  It can still
 * fulfill that role.
 *
 * Usage: ned filename [line]
 *
 * Simple commands:
 *	PF1, Ctrl/V		             Function shift key (SHIFT below)
 *	Up, Down, Left, Right	   * Movement keys
 *	Ctrl/A			           * Beginning of line
 *	Ctrl/E			           * End of line
 *	PrevScreen, Ctrl/U	       * Up a screen
 *	NextScreen, Ctrl/N	       * Down a screen
 *	Ctrl/D			           * Delete character to right
 *	DEL, Ctrl/H		           Delete character to left
 *	Ctrl/K			Delete to end of line
 *	Select, Ctrl/B		Mark start of selection
 *	Remove, Ctrl/W		Cut from mark to cursor
 *	InsertHere, Ctrl/Y	Paste cut text
 *	Ctrl/J			Justify line
 *	Find, Ctrl/F		Find text
 *	Ctrl/G			Go to specific line number
 *	Ctrl/^			Insert control character
 *	Ctrl/L			Refresh screen
 *	Ctrl/C			Quit without saving
 *	Ctrl/X			Quit with save
 *
 * Shifted commands, key PF1 or Ctrl/V then command:
 *	PrevScreen, Ctrl/U	Go to top of file
 *	NextScreen, Ctrl/N	Go to bottom of file
 *	Find, Ctrl/F		Find text backwards
 *	i, I			Include file
 *	w, W			Save file under new name
 *	r, R			Replace found text with contents of cut buffer
 *
 * Above keys are DEC LK201/401 names; PC-101/104 equivalents are:
 *	PrevScreen	PageUp
 *	NextScreen	PageDn
 *	Find		Home
 *	Select		End
 *	Remove		Delete
 *	InsertHere	Insert
 *	DEL		Backspace
 *	PF1		NumLock (not on DOS version -- use Ctrl/V)
 *
 * Current version tested by me under NetBSD 1.*, Linux, MS-DOS (Turbo C V2.0).
 * Also seen working under Solaris 2.6 and HPUX 10.20.  Older version tested
 * with Digital Unix; should work with most Unices.
 *
 * Compiling under Unix:
 *	cc -o ned ned.c -lcurses -ltermcap
 *	or just sh ned.c
 *
 * Compiling under DOS (Turbo C v2.0; later versions should be similar --
 * switches are just to turn off some overly paranoid warnings, to use
 * the large memory model, and enable emulation of curses functions)
 *	tcc -DDOS -ml -w-pia -w-par ned
 *
 * Note: Under DOS requires ANSI.SYS or equivalent.
 *
 * Author:
 *	Don Stokes
 *	Daedalus Consulting Services
 *	Email: don@daedalus.co.nz
 *
 * Modifications (since v0.7):
 *	8/12/98/dcs
 *		Added horizontal panning
 *		Justify line fixed to work on last line of file, also
 *		move cursor to end of line.
 */

/* Copyright 1996, 1997, 1998 Don Stokes.  All rights reserved.
 *
 * Permission granted for individual use.  Unauthorised re-distribution
 * prohibited.  (That is, please ask permission before placing in public
 * archive or including in other non-commercial packages -- it will almost
 * certainly be given.  Arrangements can be reached for commercial
 * distribution.)
 *
 * No warranty of fitness expressed or implied.  No liability will be accepted
 * for loss or damage caused or contributed to by any use or misuse of this
 * program.
 *
 * All copies, regardless of individual arrangements, must retain this notice.
 */

/*
  new over 0.8u:
	- INSERT key (insert space at cursor position)
	- key- and charcode for TAB handled seperatly
	- some functions renamed for better readability and
	  to avoid possible conflicts (eg "delete")
*/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>

#ifdef __CC65__
#pragma staticlocals (1)
#endif

#define rename(_a,_b) (0)
#define chmod(f,m)
#define chown(f,o,g)
#define lstat(f,b) (0)

#define LINES 25
#define COLS  40

#define CH_CR 13
#define CH_LF 10

#define CH_CTRL_AT 0
#define CH_CTRL_A 1
#define CH_CTRL_B 2
#define CH_CTRL_C 3
#define CH_CTRL_D 4
#define CH_CTRL_E 5
#define CH_CTRL_F 6
#define CH_CTRL_G 7
#define CH_CTRL_H 8
#define CH_CTRL_I 9
#define CH_CTRL_J 10
#define CH_CTRL_K 11
#define CH_CTRL_L 12
#define CH_CTRL_M 13
#define CH_CTRL_N 14
#define CH_CTRL_O 15
#define CH_CTRL_P 16
#define CH_CTRL_Q 17
#define CH_CTRL_R 18
#define CH_CTRL_S 19
#define CH_CTRL_T 20
#define CH_CTRL_U 21
#define CH_CTRL_V 22
#define CH_CTRL_W 23
#define CH_CTRL_X 24
#define CH_CTRL_Y 25
#define CH_CTRL_Z 26

#define CH_ALT_AT 0+0x80
#define CH_ALT_A 1+0x80
#define CH_ALT_B 2+0x80
#define CH_ALT_C 3+0x80
#define CH_ALT_D 4+0x80
#define CH_ALT_E 5+0x80
#define CH_ALT_F 6+0x80
#define CH_ALT_G 7+0x80
#define CH_ALT_H 8+0x80
#define CH_ALT_I 9+0x80
#define CH_ALT_J 10+0x80
#define CH_ALT_K 11+0x80
#define CH_ALT_L 12+0x80
#define CH_ALT_M 13+0x80
#define CH_ALT_N 14+0x80
#define CH_ALT_O 15+0x80
#define CH_ALT_P 16+0x80
#define CH_ALT_Q 17+0x80
#define CH_ALT_R 18+0x80
#define CH_ALT_S 19+0x80
#define CH_ALT_T 20+0x80
#define CH_ALT_U 21+0x80
#define CH_ALT_V 22+0x80
#define CH_ALT_W 23+0x80
#define CH_ALT_X 24+0x80
#define CH_ALT_Y 25+0x80
#define CH_ALT_Z 26+0x80

#define VERSION "ned65 v0.9a"

/* (editor handles ~16k files on c64) */
#define FIRSTBUFCHUNK  (1024*4)
#define EXTENDBUFCHUNK (512)

#define INSBUFSIZE (512)

unsigned char *buffer;
unsigned char *cutbuffer;
unsigned int Cursor,scrtop,bufsize,cutbufsize,cutpoint,bufalloc;
char *showmessage;
//char scrnupd = 0;
char scrbuf[256];

int row,col,actualcol,leftmargin;
int ccol;

int refstate;
unsigned int refpos;

int selactive;

char *filename;
int modified;

char showtabs;
char t1='t',t2='.'; // tab characters

#define TRACE(m)

#define REFEOL 1
#define REFEOS 2
#define REFSCR 4
#define REFSTA 8

#define UPARR     CH_CURS_UP
#define DOWN      CH_CURS_DOWN
#define LEFT      CH_CURS_LEFT
#define RIGHT     CH_CURS_RIGHT

#define END       CH_CTRL_E 		/* Ctrl/E goto end of line */
#define HOME      CH_CTRL_A	     	/* Ctrl/A goto start of line */
#define PGUP      CH_CTRL_U   		/* Ctrl/U go one page up */
#define PGDOWN    CH_CTRL_N     	/* Ctrl/N go one page down */

#define DEL       CH_DEL			/* DEL */
#define BKSP      CH_DEL	        /* Backspace */

#define INS       CH_INS            /* INS insert space */
#define DELF  	  CH_CTRL_D	        /* Ctrl/D */
#define DELEOL    CH_CTRL_K	        /* Ctrl/K */

#define TABCODE   '\t'              /* Tab */
#define TABKEY    CH_CTRL_I 		/* Ctrl/I Tab */
#define TABTOGGLE CH_ALT_I 	    	/* Ctrl/I Tab */

#define CRET      CH_CR	         	/* CR */
#define LFEED     CH_LF          	/* LF (bloody unix!) */

#define JUSTIFY   CH_CTRL_J 		/* Ctrl/J */
#define REFR      CH_CTRL_L 		/* Ctrl/L refresh screen */
#define EXIT      CH_CTRL_X	     	/* Ctrl/X */
#define SELECT    CH_CTRL_B     	/* Ctrl/B */
#define CUT       CH_CTRL_W  		/* Ctrl/W */
#define PASTE     CH_CTRL_Y     	/* Ctrl/Y */
#define FIND      CH_CTRL_F 		/* Ctrl/F */
#define ABORT     CH_CTRL_C 		/* Ctrl/C */
#define QUOTE 	  CH_CTRL_AT	    /* Ctrl/@ */
#define SHIFT     CH_CTRL_V     	/* Ctrl/V */
#define GOTO 	  CH_CTRL_G	    	/* Ctrl/G */

typedef unsigned char   BOOLTYPE;
typedef unsigned char   KEYTYPE;
typedef unsigned char   CHARTYPE;

/*
  some util-routines
*/

// scan back to beginning of previous line
// in: current position  out: new position
//unsigned int __fastcall__ findbol(unsigned int c) {
unsigned int findbol(unsigned int c) {
unsigned char *buf= buffer;
    TRACE("findbol")

	while (c)
    {
		--c;
		if (buf[c] == '\n')
        {
			++c;
			break;
		}

	}
	return c;
}

unsigned int __fastcall__ findeol(unsigned int c) {
unsigned char *buf = buffer;
	TRACE("findeol")
  	for(; (buf[c] != '\n') && (c < bufsize); ++c);
//  while((buf[c] != '\n') && (c < bufsize)) c++;
	return (c);
}

unsigned int __fastcall__ newcol(unsigned int c)
{
unsigned char *buf = buffer;
CHARTYPE ch;
//	unsigned int d;
	unsigned int i;

	TRACE("newcol")

//	*(char*)0xd020+=1;

	if(!ccol) {
		i = findbol(Cursor);
		// find out cursor column
		for(ccol = 0; i < Cursor; ++i)
        {
//			if(buf[d] == TABCODE) while(++ccol % 8);
//			else if(buf[d] < ' ' || buf[d] == 127) ccol += 2;
//			else ccol++;
			if((ch=buf[i]) == TABCODE) while(++ccol & 7);
			else if(!isprint(ch)) ccol+=2;
			else ccol++;
		}
	}
	// scan to cursor column
	for(i = 0; i < ccol; ++c)
    {
//		if((buf[c] == '\n') || (c == bufsize)) return c;
//		if(buf[c] == TABCODE) while(++i % 8);
//		else if(buf[c] < ' ' || buf[c] == 255 ||
//			(buf[c] >= 127 && buf[c] < 160)) i += 2;
//		else		     i++;

		if(((ch=buf[c]) == '\n') || (c == bufsize)) return c;
		if(ch == TABCODE) while(++i & 7);
		else if(!isprint(ch)) i+=2;
		else i++;
	}
	if(i != ccol) c--;
	return c;
}

/*

  in:
		scrtop
		pos
		bufsize
		leftmargin

  out:
		row,col
		actualcol
		scrtop

*/

int __fastcall__ findpos(unsigned int pos)
{
unsigned char *buf = buffer;
CHARTYPE ch;
unsigned int c;
unsigned int r,rr /*,i*/;

	TRACE("findpos")
	r = 0;
	if(scrtop > pos) {
		rr = 0;
		for(c = pos; c < scrtop; ++c) if(buf[c] == '\n') --rr;
		scrtop = findbol(pos);
		row = 0; col = 0;
		for(c = scrtop; c < pos; ++c) {
			++col;
//			if(buf[c] == TABCODE) for(; (col % 8) != 0; col++);
//			else if(buf[c] < 32 || buf[c] == 255 ||
//				(buf[c] > 126 && buf[c] < 160)) col++;
			// count additional char for control-char

			if((ch=buf[c]) == TABCODE) for(; (col & 7) != 0; ++col);
			else if(!isprint(ch)) ++col;
		}
	} else {
		col = 0;
		rr = 0;
		for(c = scrtop; (c < bufsize) && (c != pos); ++c) {
			++col;
//			if(buf[c] == '\n') {
//				r++;
//				col = 0;
//			} else if(buf[c] == TABCODE) {
//				for(; (col % 8) != 0; col++);
//			} else if(buf[c] < ' ' || buf[c] == 255 ||
//				  (buf[c] > 126 && buf[c] < 160)) col++;


			if((ch=buf[c]) == '\n') {
				++r;
				col = 0;
			} else if(ch == TABCODE) {
				for(; (col & 7) != 0; ++col);
			// count additional char for control-char
			} else if(!isprint(ch)) ++col;
		}
		row = r;
		if(r >= (LINES-1)) {
			c = scrtop;
			scrtop = findbol(pos);
			for(; (c < bufsize) && (c != pos); ++c) {
				if(buf[c] == '\n') {
					--r;
					if(r < (LINES-1)) {
						rr = r;
						scrtop = (c + 1);
						break;
					}
				}
			}
			rr = r;
		}
	}
	actualcol = col;
	col -= leftmargin;
	if(col < 0) col = 0;
	if(col >= COLS) col = (COLS-1);
	return rr;
}

void __fastcall__ setref(int state)
{
unsigned char *buf = buffer;
	unsigned int c;

	TRACE("setref")
	if(state == REFSCR || state == REFSTA) {
		refstate |= state;
		return;
	}
	if(state == REFEOS) {
		if((refstate & REFEOS) && (Cursor > refpos)) return;
		refstate |= REFEOS;
		if((refstate & REFEOL) && (Cursor > refpos)) return;
		refpos = Cursor;
	} else if(state == REFEOL) {
		if(refstate & REFEOS) {
			if(Cursor < refpos) refpos = Cursor;
			return;
		}
		if(refstate & REFEOL) {
			if(Cursor < refpos) {
				for(c = Cursor; c < refpos; ++c) {
					if(buf[c] == '\n') {
						state |= REFEOS;
						refpos = Cursor;
						return;
					}
				}
			} else if(Cursor > refpos) {
				for(c = refpos; c < Cursor; ++c) {
					if(buf[c] == '\n') {
						refstate |= REFEOS;
						return;
					}
				}
			}
		}
		refpos = Cursor;
		refstate |= REFEOL;
	}
}

void __fastcall__ message(char *msg, int sts) {   /* sts ?! */
	sts=sts; // get rid of warning
	showmessage = msg;
	setref(REFSTA);
}

void __fastcall__ showbusy(unsigned char flg)
{
static char busy[4]={'.','o','O','o'};
static char cnt=0;
	++cnt;cnt&=3;
	gotoxy(COLS-1,LINES-1);
	if(flg) revers(1);
	cputc(busy[cnt]);
	if(flg) revers(0);
}

unsigned char __fastcall__ cgetcatxy(int x,int y) {
unsigned char *scrn=(char*)(0x0400+x+(y*40));
	return(*scrn);
}
void __fastcall__ cputctoxy(unsigned char c,int x,int y) {
unsigned char *scrn=(char*)(0x0400+x+(y*40));
	*scrn=c;
}

//#define cputctoxy(_c,_x,_y) cputcxy(_c,_x,_y)

static char oldrow=-1,oldcol=-1;
static char oldchar;

void __fastcall__ disp_cursor_forceoff(void) {
		// output original char at old cursor position
		if((cgetcatxy(oldcol,oldrow)^0x80)==oldchar) {
			cputctoxy(oldchar,oldcol,oldrow);
		}
		oldrow=-1;oldcol=-1;
}

void __fastcall__ disp_cursor(void) {
unsigned char row=wherey(),col=wherex();
	if((oldrow!=row)||(oldcol!=col)) { // if new position
		if((oldrow!=-1)&&(oldcol!=-1)) {
			// output original char at old cursor position
//			gotoxy(oldcol,oldrow);revers(1);cputc(oldchar);revers(0);
			if((cgetcatxy(oldcol,oldrow)^0x80)==oldchar) {
				cputctoxy(oldchar,oldcol,oldrow);
			}
		}
		// remember char at new position
		oldchar=cgetcatxy(col,row);
		// remember new position
		oldrow=row;oldcol=col;
	}
	// output inverted original char at cursor position
//	gotoxy(col,row);revers(1);cputc(oldchar);revers(0);
	cputctoxy(oldchar^0x80,col,row);
}

// updates screen
void __fastcall__ refrscr(void)
{
unsigned char *buf = buffer;
unsigned int rstate;
	unsigned int c/*,ch*/;
	char ch;
unsigned int i,r, co, cos, oc;
    static char outbuf[32];

	TRACE("refrscr")
	if((refstate & REFSTA) || showmessage) {
//		move(LINES-1,0);
		gotoxy(19,LINES-1);
		revers(1);
		if(showmessage) if(!showmessage[0]) showmessage = 0;
		if(showmessage) {
			if(showmessage != scrbuf) {
				strncpy(scrbuf, showmessage, 255);
				scrbuf[255] = 0;
			}
			if((i = strlen(scrbuf)) < 39) {
				for(; i < 40; ++i) scrbuf[i] = ' ';
				strncpy(&scrbuf[40], filename, 200);
			}
			showmessage = "";
//		} else sprintf(scrbuf, "%-10s%-30s%s",
//			VERSION, " (Ctrl/X to exit and save)", filename);
//		} else sprintf(scrbuf, "%-23s","(Ctrl/X to exit and save)");
		} else sprintf(scrbuf, "\"%-16s\"",filename);

//		for(i = strlen(scrbuf); (i < 255) && (i < (COLS-1)); i++)
//			scrbuf[i] = ' ';
//		if(i > (COLS-1)) i = (COLS-1);

		// changed to use bottom right corner character aswell
		for(i = strlen(scrbuf); (i < 255) && (i < (COLS)); ++i) scrbuf[i] = ' ';
		if(i > (COLS)) i = (COLS);

		scrbuf[i] = 0;

		cputs(scrbuf);
		revers(0);
	}

	// added display of current line/column in file
	gotoxy(0,(LINES-1));
	revers(1);
	sprintf(scrbuf, "%5d:%5d:%04x %c ",row,actualcol,bufsize,t1);
	cputs(scrbuf);
	revers(0);

	rstate = refstate & (REFSCR|REFEOS|REFEOL);

	if(findpos(Cursor)) rstate = REFSCR;

	if(leftmargin && (actualcol < COLS) && (ccol < COLS)) {
		for(c = Cursor; (c < bufsize) && (buf[c] != '\n'); ++c) ;
		if(((c - Cursor) + actualcol) < COLS) {
			leftmargin = 0;
			rstate = REFSCR;
		}
	}
	while(actualcol >= (leftmargin + COLS)) {
		rstate = REFSCR;
		leftmargin += 8;
	}
	while(actualcol < leftmargin) {
		rstate = REFSCR;
		leftmargin -= 8;
	}

	// return if screen doesnt have to be updated
	if(!rstate)
    {
/*      col/row etc unchanged until now so this one is not needed!
		findpos(Cursor);
*/
		return;
	}

	disp_cursor_forceoff();

	col = actualcol - leftmargin;

	if(rstate & REFSCR) refpos = scrtop;
	if(findpos(refpos)) {
		rstate = REFSCR;
		refpos = scrtop;
		row = col = actualcol = 0;
	}

	// update displayed file

		r = row;
		co = actualcol;
		gotoxy( col,row);

//		for(c = refpos; (r < (LINES-1)) && (c < bufsize); c++) {
		for(c = refpos; (c < bufsize); ++c)
        {

//		*(char*)0xd020+=1;

			cos = co;    // current column
			++co;
			ch = buf[c]; // current byte in buffer

			if(isprint(ch)) {
					if((cos >= leftmargin) && (cos < (leftmargin+COLS))) {
						cputc(ch);
					}
			} else { // is controlchar
				// goto next line in file
				if(ch == '\n') {
					if(cos < (COLS+leftmargin)) cclear(COLS-wherex());
					if(rstate == REFEOL) {
						findpos(Cursor);
						refstate = 0;
						return;
					}
					++r;
					gotoxy(0,r);
					co = 0;

					if((r >= (LINES-1))){
						break;
					}

				} else {
				// output control character

						// display tabs
						if (ch == TABCODE) {

							outbuf[0] = t1;
							oc = 1;
							i = co & 7;
							if (i) {
								for(; i < 8; ++co) {
									outbuf[oc] = t2;
									++oc;
									++i;
								}
							}

							// output a number of characters
							for(i = 0; i < oc; ++i) {
								if(((cos+i) >= leftmargin) && ((cos+i) < (leftmargin+COLS)))
                                {
									cputc(outbuf[i]);
								}
							}

						}
						// display control chars
    					else {

							if (ch < 32) {
								outbuf[0] = '^';
								outbuf[1] = ch + '@';
							} else if (ch == 127) {
								outbuf[0] = '^';
								outbuf[1] = '?';
							} else if (ch < 160) {
								outbuf[0] = '&';
								outbuf[1] = (ch - 128) + '@';
							} else if (ch == 255) {
								outbuf[0] = '&';
								outbuf[1] = '?';
							}

							co++;

							if(((cos) >= leftmargin) && ((cos) < (leftmargin+COLS)))
								cputc(outbuf[0]);
							cos++;
							if(((cos) >= leftmargin) && ((cos) < (leftmargin+COLS)))
								cputc(outbuf[1]);

						}

				}  // output ctrlchar
			} // is ctrlchar

		}

		// print EOF mark
		if((c == bufsize) && (r < (LINES-1))) {
			revers(1);
			cputs("[eof]");
			revers(0);
		}
		// clear until end of each line
		if(r < LINES-1) cclear(COLS-wherex());
		while(++r < (LINES-1)) {
			gotoxy(0,r);
			cclear(COLS-wherex());
		}

	findpos(Cursor);
	refstate = 0;
}

void __fastcall__ redraw(int k) /* k ?! */
{
	k=k; // get rid of warning
	TRACE("redraw")

    setref(REFSCR);
	setref(REFSTA);
	refrscr();

	gotoxy(col,row);
}

unsigned char* __fastcall__ setbufsize(unsigned int newsize)
{
	unsigned char *b;
	TRACE("setbufsize")
	if(!buffer) {
		buffer = (unsigned char *)malloc(FIRSTBUFCHUNK);
		bufalloc = FIRSTBUFCHUNK;
	}
	if(newsize >= bufalloc) {
		if(!(b = (unsigned char *)realloc(buffer, newsize+EXTENDBUFCHUNK))) {
			message("Insufficient memory", 1);
			return 0;
		}
		*(char*)0xd021+=1;
		buffer = b;
		bufalloc = newsize+EXTENDBUFCHUNK;
	}
	bufsize = newsize;
	return buffer;
}

unsigned char* __fastcall__ setcutbufsize(unsigned int newsize) {
	unsigned char *b;
	TRACE("setcutbufsize")
	if(cutbuffer) free(cutbuffer);
	if(!(b = (unsigned char *) malloc(newsize))) {
		message("Insufficent memory", 1);
		return 0;
	}
	cutbuffer = b;
	cutbufsize = newsize;
	gotoxy(0,1);cprintf("%04x %d",cutbuffer,cutbufsize);
	return cutbuffer;
}

// prompt for user input
int __fastcall__ ask(char *prompt, char *buf, int siz) {
	int r,c,k,first,i;

	TRACE("ask")
	r = 0;

	disp_cursor_forceoff();

	gotoxy( 0,LINES-1);
	revers(1);
	sprintf(scrbuf,"%s%s", prompt, buf);
	for(i = strlen(scrbuf); i < 255 && i < COLS-1; ++i) scrbuf[i] = ' ';
	if(i > COLS-1) i = COLS-1;
	scrbuf[i] = 0;
	cputs(scrbuf);

	r = strlen(prompt);
	c = 0;
	first = 1;
	while(1) {
		gotoxy(r,LINES-1);

		disp_cursor();

//		refresh(); /* empty! */
		k = cgetc();

		switch(k) {
		case FIND:
		case CRET:
		case LFEED:
			if(!first) buf[c] = 0;
			revers(0);
			disp_cursor_forceoff();

			setref(REFSTA);
			return c;
#if !(BKSP == DEL)
		case BKSP:
#endif
		case DEL:
			if(c) {
				--c; --r;
				gotoxy(r,LINES-1);
				cputc(' ');
			}
			break;
		default:
			if((c<siz)&(isprint(k)!=0))
            {
				cputc(k);
				if(first) {
					for(i = strlen(buf)-1; i > 0; --i)
						cputc(' ');
					first = 0;
				}
				buf[c++] = k;
				++r;
			}
			break;     /* bug if this break; is missing! */
		}
	}
}

// return keycode for last key
// this function also takes care of refreshing the screen
KEYTYPE __fastcall__ getkey(void)
{
	TRACE("getkey")
	// update screen
	refrscr();
	// get char
	gotoxy(col,row);
    disp_cursor();
	return((KEYTYPE)cgetc());
}

/*
	subroutines for editor commands
*/

BOOLTYPE __fastcall__ left(void)
{
	TRACE("left")
	if(!Cursor) {
		message("At top of file",1);
		return (0);
	}
	--Cursor;
	ccol = 0;
	return (1);
}

BOOLTYPE __fastcall__ right(void)
{
	TRACE("right")
	if(Cursor == bufsize) {
		message("At bottom of file",1);
		return (0);
	}
	++Cursor;
	ccol = 0;
	return (1);
}

BOOLTYPE __fastcall__ up(void)
{
	unsigned int c;

	TRACE("up")
	c = findbol(Cursor);
	if(!c) {
		message("At top of file",1);
		return (0);
	}
	Cursor = newcol(findbol(c-1));
	return (1);
}

BOOLTYPE __fastcall__ down(void)
{
	unsigned int c;

	TRACE("down")
	c = findeol(Cursor);
	if(c++ == bufsize) {
		message("At bottom of file",1);
		return (0);
	}
	Cursor = newcol(c);
	return (1);
}

void __fastcall__ startselect(void)
{
	TRACE("startselect")
	cutpoint = Cursor;
	selactive = 1;
	message("Mark set",0);
}

// type one char at cursor position, fast version of "insert"
void __fastcall__ type(unsigned char ch)
{
unsigned char *buf = buffer;
//unsigned int c;

	TRACE("type")

	if(!setbufsize(bufsize+1)) return;
	setref(REFEOL);
//	for(c = bufsize; c >= (Cursor + 1); c--) buf[c] = buf[c-1];
	memmove(&buf[Cursor+1],&buf[Cursor],(bufsize-Cursor));

	if(cutpoint > Cursor) ++cutpoint;
	buf[Cursor++] = ch;
	if(ch == '\n') setref(REFEOS);
	ccol = 0;
	modified = 1;
}


//void __fastcall__ insert(unsigned char *k, int l) {
void insert(unsigned char *k, int l) {
unsigned char *buf = buffer;
unsigned int c;

	TRACE("insert")

	if(!setbufsize(bufsize+l))
    {
         return;
    }
	setref(REFEOL);
//	for(c = bufsize; c >= (Cursor + l); c--) buf[c] = buf[c-l];
	memmove(&buf[Cursor+l],&buf[Cursor],(bufsize-Cursor));
	if(cutpoint > Cursor) cutpoint += l;
	for(c = 0; c < l; ++c)
    {
		buf[Cursor++] = k[c];
		if(k[c] == '\n') setref(REFEOS);
	}
	ccol = 0;
	modified = 1;
}

// delete N chars at cursor position
void __fastcall__ cur_delete(unsigned int n)
{
	unsigned int c;
	unsigned char *buf = buffer;

	TRACE("cur_delete")
	setref(REFEOL);
	if(n > (bufsize - Cursor)) n = bufsize - Cursor;
	for(c = Cursor; c<Cursor+n; ++c) if(buf[c] == '\n') setref(REFEOS);

//	for(c = Cursor; c+n < bufsize; c++) buf[c] = buf[c+n];
	memmove(&buf[Cursor],&buf[Cursor+n],(bufsize-(Cursor+n)));

	bufsize -= n;
	if(cutpoint > Cursor) cutpoint -= n;
	ccol = 0;
	modified = 1;
}

// cuts text from mark to cursor position
void __fastcall__ cut(void)
{
	unsigned int c;
//	,d;
	unsigned char *buf = buffer;

	TRACE("cut")
	if(!selactive) {
		message("No mark set", 1);
		return;
	}
	if(Cursor > cutpoint) {
		c = cutpoint;
		cutpoint = Cursor;
		Cursor = c;
	}
	if(cutpoint > bufsize) cutpoint = bufsize;
	if(!setcutbufsize(cutpoint-Cursor)) return;

// call to stdlib might be faster and saves a variable here
//	for(c = Cursor,d = 0; c < cutpoint;) cutbuffer[d++] = buf[c++];
	memmove(&cutbuffer[0],&buf[Cursor],(cutpoint-Cursor));

//	gotoxy(0,0);cprintf("%04x %d",&cutbuffer[0],(cutpoint-Cursor));

	cur_delete(cutpoint-Cursor);
	cutpoint = Cursor;
	selactive = 0;
	modified = 1;
}

void __fastcall__ deleol(void) {
	TRACE("deleol")
	if(buffer[Cursor] == '\n') cur_delete(1);
	else {
		startselect();
		Cursor = findeol(Cursor);
		cut();
	}
}

void __fastcall__ paste(void)
{
/* unsigned int c,l; */

	TRACE("paste")
	insert(cutbuffer, cutbufsize);
}

unsigned int __fastcall__ find(char *f)
{
unsigned int c;
int l;

	TRACE("find")
	if(!(*f)) return Cursor;
	l = strlen(f);

	for(c = Cursor+1; c + l < bufsize; ++c)
		if(!memcmp(f, &buffer[c], l)) return c;

	message("Not found",1);
	return Cursor;
}

unsigned int __fastcall__ findreverse(char *f)
{
	unsigned int c;
	int l;

	TRACE("find")
	if(!(*f)) return Cursor;
	l = strlen(f);

	if(Cursor) for(c = Cursor-1;;) {
		if(!memcmp(f, &buffer[c], l)) return c;
		if(!c--) break;
	}

	message("Not found",1);
	return Cursor;
}

void __fastcall__ justify(void)
{
unsigned char *buf = buffer;
	unsigned c, i;

	Cursor = findeol(Cursor);
	if(Cursor >= bufsize) return;
	for(Cursor--; Cursor > 0 && (buf[Cursor] == ' ' ||
				     buf[Cursor]==TABCODE); --Cursor) cur_delete(1);
	++Cursor;
	c = findbol(Cursor);
	if(c == Cursor) {
		if(Cursor < bufsize) Cursor++;
		return;
	}
	ccol = 0;
	newcol(Cursor);
	if(ccol < 72) {
		if(Cursor >= bufsize) return;
		if(buf[Cursor+1] == '\n') {
			++Cursor;
			return;
		}
		buf[Cursor++] = ' ';
		setref(REFSCR);
		for(i = 0; buf[Cursor+i] == ' ' || buf[Cursor+i] == TABCODE;  ++i);
		if(i) cur_delete(i);
		ccol = 0;
		Cursor = findeol(Cursor);
		newcol(Cursor);
		modified = 1;
	}

	if(ccol > 72) {
		ccol = 72;
		Cursor = newcol(findbol(Cursor));
		for(;; --Cursor) {
			if(!Cursor || buf[Cursor] == '\n') {
				down();
				break;
			}
			if(buf[Cursor] != ' ' && buf[Cursor] != TABCODE)
				continue;
			for(i = 0; Cursor && (buf[Cursor] == ' ' ||
					 buf[Cursor] == TABCODE); --Cursor) ++i;
			++Cursor;
			if(i) cur_delete(i);
			insert("\n", 1);
			modified = 1;
			break;
		}
	} else ++Cursor;

	for(i = 0; buf[Cursor+i] == ' ' || buf[Cursor+i] == TABCODE; ++i);
	if(i) cur_delete(i);

	if(Cursor >= bufsize || buf[Cursor] == '\n') return;

	c = findbol(Cursor - 1);
	for(i = 0; buf[c+i] == ' ' || buf[c+i] == TABCODE; ++i);
	insert(&buf[c], i);

	Cursor = findeol(Cursor);
}

// abort without saving
void __fastcall__ abortedit(int i)
{
char ynbuf[4];

	strcpy(ynbuf, "YES");
	if(modified) {
		ask("Really quit? ", ynbuf, 3);
/* we like the opposite behaviour as default better
		if(*ynbuf == 'n' || *ynbuf == 'N') return;
*/
		if(ynbuf[0] != 'Y' || ynbuf[1] != 'E'|| ynbuf[2] != 'S') return;
	}
//	refresh();
//    noraw();
//	nl();
//    echo();
//    endwin();
	putchar('\n');
	exit(i);
}

/*
 file i/o
 */

BOOLTYPE __fastcall__ insertfile(char *filename)
{
	unsigned int i;
	//CHARTYPE
    int k;
	FILE *f;
//	static char buf[INSBUFSIZE];
CHARTYPE *buf;

	i = 0;
	if(f = fopen(filename, "rb"))
    {
		if(!(buf = (CHARTYPE*) malloc(INSBUFSIZE*sizeof(CHARTYPE))))
        {
			message("Insufficent memory", 1);
			return (1);
		}
		showbusy(0);
		while((k = fgetc(f)) != EOF) if(k)
        {

//			buf[i++] = k;
			buf[i] = k; ++i;
//            gotoxy(2,2+i);cprintf("buf:%x",k);
			if(i == INSBUFSIZE)
            {
				showbusy(1);
				insert(buf, INSBUFSIZE);
				showbusy(0);
				i = 0;
			}

		}
		if(i)
        {
//        gotoxy(2,2+i);cprintf("buf:%s",buf);cgetc();
            insert(buf, i);
        }
		fclose(f);
		free(buf);
	} else {
		sprintf(scrbuf, "ERROR: could not read %s", filename);
		message(scrbuf,0);
		return (1);
	}
	return (0);
}

BOOLTYPE __fastcall__ writefile(char *filename)
{
	FILE *f /*, *fb*/;
	static char backupfile[255];
	char *dot /*, *c*/;
	int notnew, bytes;
	unsigned int i;

	notnew = 1;
	if((f = fopen(filename, "r")) && notnew)
    {
		fclose(f);

		strncpy(backupfile, filename, 250);
		backupfile[250] = 0;
		dot = strchr(backupfile, 0);
		strcpy(dot, ".bak");

		unlink(backupfile);
		if(strcmp(filename, backupfile))
        {
			if(lstat(filename, &statbuf) ||
			   rename(filename, backupfile))
            {
				gotoxy( 0,(LINES-1));
				cputs("ERROR: could not make backup ");
				cputs(backupfile);
				return 0;
			}
		} else notnew = 0;
	} else notnew = 0;

	if(!(f = fopen(filename, "w")))
    {
		gotoxy( 0,(LINES-1));
		sprintf(scrbuf, "ERROR: could not write %s", filename);
		message(scrbuf,0);
		return (0);
	}
	bytes = 0;
	for(i = 0; i < bufsize; ++i) {
		fputc(buffer[i], f);
		++bytes;
	}
	fclose(f);

	if(notnew)
    {
		chmod(filename, statbuf.st_mode);
		chown(filename, statbuf.st_uid, statbuf.st_gid);
	}

	gotoxy(0,LINES-1);
	cclear(COLS-wherex());
	sprintf(scrbuf,"%s %d bytes", filename, bytes);
	message(scrbuf, 0);
	modified = 0;
	return (1);
}

/*
   main loop
*/

int main(int argc, char **argv) {

//	FILE *f;
	unsigned int i, j;

	KEYTYPE k;
#if sizeof(CHARTYPE) != sizeof(KEYTYPE)
	CHARTYPE ch;
#endif

	static char findbuffer[32];
	static char linbuf[16];
	static char filenambuf[80];

/*
	if(argc != 2 && argc != 3) {
		cputs(VERSION);
		cputs("Usage: ned <filename> [<line>]");
		return 1;
	}
*/

	// init editor globals
	buffer=cutbuffer=0;
	setbufsize(0);

	leftmargin = 0;
	ccol = 0;
	cutbuffer = 0;
	cutpoint = 0; selactive = 0;
	scrtop = Cursor = 0;
	findbuffer[0] = 0;
	modified = 0;

	showmessage=0;

	showtabs=1;
	refstate = REFSCR|REFSTA;

	findpos(Cursor);

 	filename = "";

/*
	if(argc != 2 && argc != 3) {

//    	filename = "";

	} else {

		filename = argv[1];

		insertfile(filename);

		if(argc == 3) {
			j = atoi(argv[2]);
			for(i = 1; i < j; i++) down();
		}

	}
*/

	k=0;while(k!=ABORT)
    {
		// read char from keyboard
		k = getkey();
		// check for control commands
		switch(k) {
			case LEFT:
				left();
				break;
			case UPARR:
				up();
				break;
			case DOWN:
				down();
				break;
			case RIGHT:
				right();
				break;
			case PGUP:
				for(i = 0; i < (LINES-1); ++i) up();
				break;
			case PGDOWN:
				for(i = 0; i < (LINES-1); ++i) down();
				break;
			case HOME:
				Cursor = findbol(Cursor);
				ccol = 0;
				break;
			case END:
				Cursor = findeol(Cursor);
				ccol = 0;
				break;
			case FIND:
				ask("Find: ", findbuffer, 31);
				Cursor = find(findbuffer);
				ccol = 0;
				break;
			case GOTO:
				*linbuf = 0;
				ask("Goto: ", linbuf, 15);
				if(j = atoi(linbuf)) {
					Cursor = 0;
					for(i = 1; i < j; ++i) down();
				}
				break;
			case SELECT:
				startselect();
				break;
			case CUT:
				cut();
				break;
			case PASTE:
				paste();
				break;
			case DELF:
				cur_delete(1);
				break;
			case DELEOL:
				deleol();
				break;
	#if !(BKSP == DEL)
			case BKSP:
	#endif
			case DEL:
				if(left()) cur_delete(1);
				break;
			/* added key for inserting a space at cursor position */
			case INS:
				insert(" ", 1);
				left();
				break;
 	/* 	case LFEED: */
			case JUSTIFY:
				justify();
				break;
			case CRET:
				insert("\n", 1);
				break;
			case EXIT:
				if (modified) {
					writefile(filename);
				}
				if (!modified) {
					cputs(scrbuf);
					abortedit(0);
				}
				break;
			case ABORT:
				gotoxy(0,(LINES-1));
				abortedit(0);
				break;
			case REFR:
				redraw(0);
				break;
			case QUOTE:
				k = getkey();
//				if(k >= 0 && k < 256) {
#if sizeof(CHARTYPE) != sizeof(KEYTYPE)
					ch = k;
					insert(&ch, 1);
#else
//					insert(&k, 1);
					type(k);
#endif
//				}
				break;
			// "shifted" (originally preceeded by ctrl/v) control-keys
			case SHIFT:
				switch(getkey()) {
					case PGUP:
						Cursor = 0;
						break;
					case PGDOWN:
						Cursor = bufsize;
						break;
					case FIND:
						ask("Reverse find: ", findbuffer, 31);
						Cursor = findreverse(findbuffer);
						ccol = 0;
						break;
					case 'i':
//					case 'I':
						*filenambuf = 0;
						ask("Insert: ", filenambuf, 71);
						insertfile(filenambuf);
						break;
//					case 'W':
					case 'w':
						*filenambuf = 0;
						ask("Write to: ", filenambuf, 68);
						if(*filenambuf) writefile(filenambuf);
						break;
					case 'r':
//					case 'R':
						cur_delete(strlen(findbuffer));
						paste();
						Cursor = find(findbuffer);
						break;
				}
				break;
			/* added key for switching display of tab-control characters on/off */
			case TABTOGGLE:
					showtabs^=1;
					if(showtabs==0) { t1=t2=' '; }
					else {t1='t';t2='.';}
					redraw(0);
				break;
			/* added this one to handle different char/keycode
		   	and to make the statement below cleaner */
			case TABKEY:
#if sizeof(CHARTYPE) != sizeof(KEYTYPE)
					ch = TABCODE;
					insert(&ch, 1);
#else
//					k = TABCODE;
//					insert(&k, 1);
					type(TABCODE);
#endif
				break;
			default:
			/* this one is hax0r crap ;=P
				if((k >= ' ' && k < DEL) ||
			   	(k >= 160 && k < 255) || k == TABCODE) {
		 	*/
				if(isprint(k)!=0) {
#if sizeof(CHARTYPE) != sizeof(KEYTYPE)
					ch = k;
					insert(&ch, 1);
#else
//					insert(&k, 1);
					type(k);
#endif
				}
				break; /* bug if this break; is missing! */
			}

	}

	return(0);
}
