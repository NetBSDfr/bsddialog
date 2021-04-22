/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Alfonso Sabato Siciliano
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#include <ncurses/form.h>
#else
#include <curses.h>
#include <form.h>
#endif

#include "bsddialog.h"
#include "theme.h"

struct bsddialogtheme s;

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MAXINPUT 2048

enum elevation { RAISED, LOWERED, NOLINES };

WINDOW *
new_window(int y, int x, int rows, int cols, char *title, char *bottomtitle,
    enum elevation elev, bool asciilines, bool subwindowborders);
void window_scrolling_handler(WINDOW *pad, int rows, int cols);
int print_text_multiline(WINDOW *win, int y, int x, const char *str, int size_line);

void
draw_button(WINDOW *window, int x, int size, char *text, bool selected,
    bool shortkey);
void
draw_buttons(WINDOW *window, int cols, int nbuttons, char **buttons,
    int selected, bool shortkey);
void
get_buttons(int *nbuttons, char *buttons[4], int values[4], bool yesok,
    char* yesoklabel, bool extra, char *extralabel, bool nocancel,
    char *nocancellabel, bool help, char *helplabel, bool defaultno,
    int *defbutton);

int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int sleep, int fd);
int
bar_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons, int *values,
    int selected, bool shortkey, WINDOW *bar, int sizebar, int min, int max,
    int def, int sleeptime, int fd);


int bsddialog_init(void)
{
	int i, j, c = 1, error = 0;

	if(initscr() == NULL)
		return -1;

	error += keypad(stdscr, TRUE);
	nl();
	error += cbreak();
	error += noecho();
	curs_set(0);

	error += start_color();
	for (i=0; i< 8; i++)
		for(j=0; j<8; j++) {
			error += init_pair(c, i, j);
			c++;
	}

	set_theme(DEFAULT, &s);

	bkgd(s.backgroundcolor);

	refresh();

	return error;
}

void bsddialog_end(void)
{

	endwin();
}

int bsddialog_backtitle(struct config conf, char *backtitle)
{

	mvaddstr(0, 1, backtitle);
	if (conf.no_lines != true)
		mvhline(1, 1, conf.ascii_lines ? '-' : ACS_HLINE, COLS-2);

	refresh();

	return 0;
}

int print_text_multiline(WINDOW *win, int y, int x, const char *str, int size_line)
{
	char fmtstr[8];
	int line = 0;
	
	sprintf(fmtstr, "%%.%ds", size_line);
	while(strlen(str) > 0) {
		mvwprintw(win, y + line, x, fmtstr, str);
		if((int)strlen(str) > size_line)
			str += size_line;
		else
			str += strlen(str);

		line++;
	}
	line = line > 0 ? line-1 : 0;
	return line;
}

WINDOW *
new_window(int y, int x, int rows, int cols, char *title, char *bottomtitle,
    enum elevation elev, bool asciilines, bool subwindowborders)
{
	WINDOW *popup;
	int leftcolor, rightcolor;
	int ls, rs, ts, bs, tl, tr, bl, br;
	int ltee, rtee;

	ls = rs = ACS_VLINE;
	ts = bs = ACS_HLINE;
	tl = ACS_ULCORNER;
	tr = ACS_URCORNER;
	bl = ACS_LLCORNER;
	br = ACS_LRCORNER;
	ltee = ACS_LTEE;
	rtee = ACS_RTEE;

	popup = newwin(rows, cols, y, x);
	wbkgd(popup, s.widgetcolor);

	if (elev != NOLINES) {
		if (asciilines) {
			ls = rs = '|';
			ts = bs = '-';
			tl = tr = bl = br = ltee = rtee = '+';
		}
		leftcolor  = elev == RAISED ? s.lineraisecolor : s.linelowercolor;
		rightcolor = elev == RAISED ? s.linelowercolor : s.lineraisecolor;
		wattron(popup, leftcolor);
		wborder(popup, ls, rs, ts, bs, tl, tr, bl, br);
		wattroff(popup, leftcolor);

		wattron(popup, rightcolor);
		mvwaddch(popup, 0, cols-1, tr);
		mvwvline(popup, 1, cols-1, rs, rows-2);
		mvwaddch(popup, rows-1, cols-1, br);
		mvwhline(popup, rows-1, 1, bs, cols-2);
		wattroff(popup, rightcolor);

		if (subwindowborders) {
			wattron(popup, leftcolor);
			mvwaddch(popup, 0, 0, ltee);
			wattroff(popup, leftcolor);

			wattron(popup, rightcolor);
			mvwaddch(popup, 0, cols-1, rtee);
			wattroff(popup, rightcolor);
		}
	}

	if (title != NULL) {
		wattron(popup, s.titlecolor);
		wmove(popup, 0, cols/2 - strlen(title)/2);
		waddstr(popup, title);
		wattroff(popup, s.titlecolor);
	}

	if (bottomtitle != NULL) {
		wattron(popup, s.bottomtitlecolor);
		wmove(popup, rows - 1, cols/2 - strlen(bottomtitle)/2 - 1);
		waddch(popup, '[');
		waddstr(popup, bottomtitle);
		waddch(popup, ']');
		wattroff(popup, s.bottomtitlecolor);
	}

	return popup;
}

void window_scrolling_handler(WINDOW *pad, int rows, int cols)
{
	int input, cur_line = 0, shown_lines;
	bool loop = true;
	int x = 2, y = COLS/2 - cols/2; /* tofix x & y*/

	shown_lines = rows > (LINES - x - 1) ? (LINES - x - 1) : rows;
	wattron(pad, s.widgetcolor);
	while(loop) {
		mvwvline(pad, 1, cols-1, ACS_VLINE, rows-2);
		if(cur_line > 0)
			mvwaddch(pad, cur_line, cols-1, ACS_UARROW);
		if(cur_line + shown_lines < rows)
			mvwaddch(pad, cur_line + shown_lines-1, cols-1, 'v');
		prefresh(pad, cur_line, 0, x, y, shown_lines+1, COLS-2);
		input = getch();
		switch(input) {
		case KEY_UP:
			if (cur_line > 0)
				cur_line--;
			break;
		case KEY_DOWN:
			if (cur_line + shown_lines < rows)
				cur_line++;
			break;
		default:
			loop = false;
		}
	}
	wattroff(pad, s.widgetcolor);
}

void
draw_button(WINDOW *window, int x, int size, char *text, bool selected,
    bool shortkey)
{
	int i, color_arrows, color_shortkey, color_button;

	if (selected) {
		color_arrows = s.currshortkeycolor;
		color_shortkey = s.currshortkeycolor;
		color_button = s.currbuttoncolor;
	} else {
		color_arrows = s.buttoncolor;
		color_shortkey = s.shortkeycolor;
		color_button = s.buttoncolor;
	}

	wattron(window, color_arrows);
	mvwaddch(window, 1, x, '<');
	for(i = 1; i < size - 1; i++)
		mvwaddch(window, 1, x + i, ' ');
	mvwaddch(window, 1, x + i, '>');
	wattroff(window, color_arrows);

	x = x + 1 + ((size - 2 - strlen(text))/2);

	wattron(window, color_button);
	mvwaddstr(window, 1, x, text);
	wattroff(window, color_button);

	if (shortkey) {
		wattron(window, color_shortkey);
		mvwaddch(window, 1, x, text[0]);
		wattroff(window, color_shortkey);
	}
}

void
draw_buttons(WINDOW *window, int cols, int nbuttons, char **buttons,
    int selected, bool shortkey)
{
	int i, x, start_x, size;
#define SIZEBUTTON  8
#define BUTTONSPACE 3

	size = MAX(SIZEBUTTON - 2, strlen(buttons[0]));
	for (i=1; i < nbuttons; i++)
		size = MAX(size, strlen(buttons[i]));
	size += 2;

	start_x = size * nbuttons + (nbuttons - 1) * BUTTONSPACE;
	start_x = cols/2 - start_x/2;

	for (i = 0; i < nbuttons; i++) {
		x = i * (size + BUTTONSPACE);
		draw_button(window, start_x + x, size, buttons[i],
		    i == selected, shortkey);
	}
}

void
get_buttons(int *nbuttons, char *buttons[4], int values[4], bool yesok, 
    char *yesoklabel, bool extra, char *extralabel, bool nocancel,
    char *nocancellabel, bool help, char *helplabel, bool defaultno,
    int *defbutton)
{

	*nbuttons = 0;
	*defbutton = 0;

	if (yesok) {
		buttons[0] = yesoklabel;
		values[0] = BSDDIALOG_YESOK;
		*nbuttons = *nbuttons + 1;
	}

	if (extra) {
		buttons[*nbuttons] = extralabel;
		values[*nbuttons] = BSDDIALOG_EXTRA;
		*nbuttons = *nbuttons + 1;
	}

	if (nocancel) {
		buttons[*nbuttons] = nocancellabel;
		values[*nbuttons] = BSDDIALOG_NOCANCEL;
		if (defaultno)
			*defbutton = *nbuttons;
		*nbuttons = *nbuttons + 1;
	}

	if (help) {
		buttons[*nbuttons] = helplabel;
		values[*nbuttons] = BSDDIALOG_HELP;
		*nbuttons = *nbuttons + 1;
	}

	if (*nbuttons == 0) {
		buttons[0] = yesoklabel;
		values[0] = BSDDIALOG_YESOK;
		*nbuttons = 1;
	}
}

int
widget_init(struct config conf, WINDOW **widget, int *y, int *x, char *text,
    int *h, int *w, WINDOW **shadow)
{

	if (*h <= 0)
		; /* todo */

	if (*w <= 0)
		; /* todo */

	*y = (*y < 0) ? (LINES/2 - *h/2 - 1) : conf.y;
	*x = (*x < 0) ? (COLS/2 - *w/2) : conf.x;

	if (conf.shadow) {
		if ((*shadow = newwin(*h, *w+1, *y+1, *x+1)) == NULL)
			return -1;
		wbkgd(*shadow, s.shadowcolor);
		wrefresh(*shadow);
	}

	*widget = new_window(*y, *x, *h, *w, conf.title, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false);
	if(*widget == NULL) {
		delwin(*shadow);
		return -1;
	}

	if (text != NULL) /* programbox etc */
		print_text_multiline(*widget, 1, 2, text, *w - 4);

	wrefresh(*widget);

	return 0;
}

void
widget_end(struct config conf, char *name, WINDOW *window, int h, int w,
    WINDOW *shadow)
{

	delwin(window);

	if(conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "%s size: (%d, %d)\n", name, h, w);
}

/*
 * No handler: infobox
 */
int
bsddialog_infobox(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *shadow;
	int y, x;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	getch();

	widget_end(conf, "Infobox", widget, rows, cols, shadow);

	return (BSDDIALOG_YESOK);
}

/*
 * Checklist, Menu, Radiolist
 */
struct myitem {
	char *name;
	char *desc;
	bool on;
};

enum menumode { CHECKLISTMODE, MENUMODE, RADIOLISTMODE };

void
draw_myitem(WINDOW *pad, int y, struct myitem item, enum menumode mode,
    int xdesc, bool selected)
{
	int color, colorname;

	color = selected ? s.curritemcolor : s.itemcolor;
	colorname = selected ? s.currtagcolor : s.tagcolor;

	wmove(pad, y, 0);
	wattron(pad, color);
	if (mode == CHECKLISTMODE)
		wprintw(pad, "[%c]", item.on ? 'X' : ' ');
	if (mode == RADIOLISTMODE)
		wprintw(pad, "(%c)", item.on ? '*' : ' ');
	wattron(pad, color);

	wattron(pad, colorname);
	if (mode != MENUMODE)
		wmove(pad, y, 4);
	waddstr(pad, item.name);
	wattron(pad, colorname);

	wattron(pad, color);
	mvwaddstr(pad, y, xdesc + (mode==MENUMODE ? 0 : 4), item.desc);
	wattron(pad, color);
}

int
do_menu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int line, int xdesc, enum menumode mode,
    int nitems, struct myitem *items)
{
	WINDOW *widget, *button, *menuwin, *menupad, *shadow;
	char *buttons[4];
	int i, values[4], output, nbuttons, defbutton, y, x, input, curr;
	int ys, ye, xs, xe;
	bool loop, buttupdate, sep;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	menuwin = new_window(y + rows - 5 - menurows, x + 2, menurows+2, cols-4,
	    NULL, NULL, conf.no_lines ? NOLINES : LOWERED,
	    conf.ascii_lines, false);
	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	menupad = newpad(nitems, line);
	wbkgd(menupad, s.widgetcolor);

	for (i=0; i<nitems; i++) {
		draw_myitem(menupad, i, items[i], mode, xdesc, i == 0);
	}

	ys = y + rows - 5 - menurows + 1;
	ye = ys + menurows + 2 -1;
	xs = (line > cols - 6) ? (x + 2 + 1) : x + 3 + (cols-6)/2 - line/2;
	xe = (line > cols - 6) ? xs + cols - 7 : xs + cols - 4 -1;

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	    conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	wrefresh(menuwin);
	prefresh(menupad, 0, 0, ys, xs, ye, xe);//delete?

	curr = 0;
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, defbutton,
			    true);
			wrefresh(button);
			buttupdate = false;
		}
		//wrefresh(menuwin);
		prefresh(menupad, 0, 0, ys, xs, ye, xe);

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[defbutton]; // -> buttvalues[selbutton]
			loop = false;
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			defbutton = (defbutton + 1) % nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (defbutton > 0) {
				defbutton--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (defbutton < nbuttons - 1) {
				defbutton++;
				buttupdate = true;
			}
			break;
		}

		if (nitems <= 0)
			continue;

		switch(input) {
		case KEY_UP:
			draw_myitem(menupad, curr, items[curr], mode, xdesc, false);
			curr = (curr > 0) ? curr - 1 : 0;
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true);
			break;
		case KEY_DOWN:
			draw_myitem(menupad, curr, items[curr], mode, xdesc, false);
			curr = (curr < nitems-1) ? curr +1 : nitems-1;
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true);
			break;
		case ' ': /* Space */
			if (mode == MENUMODE)
				break;
			else if (mode == CHECKLISTMODE)
				items[curr].on = ! items[curr].on;
			else { //RADIOLISTMODE
				if (items[curr].on == true)
					break;
				for (i=0; i<nitems; i++)
					if (items[i].on == true) {
						items[i].on = false;
						draw_myitem(menupad, i, items[i],
						    mode, xdesc, false);
					}
				items[curr].on = true;
			}
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true);
			break;
		default:
			
			break;
		}
	}

	if (conf.sleep > 0)
		sleep(conf.sleep);

	if (output == BSDDIALOG_YESOK && nitems > 0) {
		if (mode == MENUMODE)
			dprintf(conf.output_fd, "%s", items[curr].name);
		else { /* CHECKLIST or RADIOLIST */
			sep = false;
			for (i=0; i<nitems; i++)
				if (items[i].on == true) {
					dprintf(conf.output_fd, "%s",items[i].name);
					if (sep == true)
					    dprintf(conf.output_fd, " ");
					sep = true;
				}
		}
	}

	delwin(button);
	delwin(menupad);
	delwin(menuwin);
	widget_end(conf, "MenuToFix", widget, rows, cols, shadow); // tofix name

	return output;
}

int
bsddialog_checklist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int argc, char **argv)
{
	int i, output, nitems, line, maxname, maxdesc;
	struct myitem items[128];

	if ((argc % 3) != 0)
		return (-1);

	line = maxname = maxdesc = 0;
	nitems = argc / 3;
	for (i=0; i<nitems; i++) {
		items[i].name = argv[3*i];
		items[i].desc = argv[3*i+1];
		items[i].on = strcmp(argv[3*i+2], "on") == 0 ? true : false;

		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc + 4);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    CHECKLISTMODE, nitems, items);

	return output;
}

int
bsddialog_menu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int argc, char **argv)
{
	int i, output, nitems, line, maxname, maxdesc;
	struct myitem items[128];

	if ((argc % 2) != 0)
		return (-1);

	line = maxname = maxdesc = 0;
	nitems = argc / 2;
	for (i=0; i<nitems; i++) {
		items[i].name = argv[2*i];
		items[i].desc = argv[2*i+1];

		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    MENUMODE, nitems, items);

	return output;
}

int
bsddialog_radiolist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int argc, char **argv)
{
	int i, output, nitems, line, maxname, maxdesc;
	struct myitem items[128];
	bool on = false;

	if ((argc % 3) != 0)
		return (-1);

	line = maxname = maxdesc = 0;
	nitems = argc / 3;
	for (i=0; i<nitems; i++) {
		items[i].name = argv[3*i];
		items[i].desc = argv[3*i+1];
		if (on == false && (strcmp(argv[3*i+2], "on") == 0)) {
			items[i].on = true;
			on = true;
		} else
			items[i].on = false;

		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc + 4);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    RADIOLISTMODE, nitems, items);

	return output;
}

/*
 * Buttons handler: msgbox, yesno
 */
int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int sleeptime, int fd)
{
	bool loop, update;
	int i, input;
	int output;

	loop = update = true;
	while(loop) {
		if (update) {
			draw_buttons(window, cols, nbuttons, buttons, selected,
			    shortkey);
			update = false;
		}
		wrefresh(window);
		input = getch();
		switch (input) {
		case 10: /* Enter */
			output = values[selected];
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': /* TAB */
			selected = (selected + 1) % nbuttons;
			update = true;
			break;
		case KEY_LEFT:
			if (selected > 0) {
				selected--;
				update = true;
			}
			break;
		case KEY_RIGHT:
			if (selected < nbuttons - 1) {
				selected++;
				update = true;
			}
			break;
		default:
			if (shortkey) {
				for (i = 0; i < nbuttons; i++)
					if (input == (buttons[i])[0]) {
						output = values[selected];
						loop = false;
				}
			}
		}
	}

	sleep(sleeptime);

	return output;
}

int
bsddialog_msgbox(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[3];
	int values[3], output, nbuttons, defbutton, y, x;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, false, NULL,
	conf.help_button, conf.help_label, false, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values, 0,
	    true, conf.sleep, /*fd*/ 0);

	delwin(button);
	widget_end(conf, "Msgbox", widget, rows, cols, shadow);

	return output;
}

int
bsddialog_yesno(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton, y, x;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.yes_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.no_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /*fd*/ 0);

	delwin(button);
	widget_end(conf, "Yesno", widget, rows, cols, shadow);

	return output;
}

/* Forms: Form, Inputbox, Inputmenu, Mixedform, Password, Passwordform */
int bsddialog_inputmenu(struct config conf, char* text, int rows, int cols)
{
	return 0;
}

#define ITEMHIDDEN 0x1
#define ISITEMHIDDEN(item) (item.itemflags & 0x1)
#define ITEMREADONLY 0x2
#define ISITEMREADONLY(item) (item.itemflags & 0x2)
struct formitem {
	char *label;
	unsigned int ylabel;
	unsigned int xlabel;
	char *item;
	unsigned int yitem;
	unsigned int xitem;
	int itemlen;
	unsigned int inputlen;
	unsigned int itemflags;
};

int
mixedform_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *entry, FORM *form,
    FIELD **field, int nitems, struct formitem *items, int sleeptime, int fd)
{
	bool loop, buttupdate, inentry = true;
	int i, input, output;
	char *bufp;

	curs_set(2);
	pos_form_cursor(form);
	loop = buttupdate = true;
	selected = -1;
	while(loop) {
		if (buttupdate) {
			draw_buttons(buttwin, cols, nbuttons, buttons, selected,
			    shortkey);
			wrefresh(buttwin);
			buttupdate = false;
		}
		wrefresh(entry);
		input = getch();
		switch(input) {
		case 10: // Enter
			if (inentry)
				break;
			output = values[selected]; // values -> buttvalues
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
			for (i=0; i<nitems; i++) {
				bufp = field_buffer(field[i], 0);
				dprintf(fd, "\n+%s", bufp);
				bufp = field_buffer(field[i], 1);
				dprintf(fd, "-%s+", bufp);
			}
			loop = false;
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			if (inentry) {
				selected = 0;
				inentry = false;
				curs_set(0);
			} else {
				selected++;
				inentry = selected >= nbuttons ? true : false;
				if (inentry) {
					curs_set(2);
					pos_form_cursor(form);
				}
			}
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (inentry) {
				form_driver(form, REQ_PREV_CHAR);
			} else {
				if (selected > 0) {
					selected--;
					buttupdate = true;
				}
			}
			break;
		case KEY_RIGHT:
			if (inentry) {
				form_driver(form, REQ_NEXT_CHAR);
			} else {
				if (selected < nbuttons - 1) {
					selected++;
					buttupdate = true;
				}
			}
			break;
		case KEY_UP:
			if (nitems < 2)
				break;
			set_field_fore(current_field(form), s.fieldcolor);
			set_field_back(current_field(form), s.fieldcolor);
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), s.currfieldcolor);
			set_field_back(current_field(form), s.currfieldcolor);
			break;
		case KEY_DOWN:
			if (nitems < 2)
				break;
			set_field_fore(current_field(form), s.fieldcolor);
			set_field_back(current_field(form), s.fieldcolor);
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), s.currfieldcolor);
			set_field_back(current_field(form), s.currfieldcolor);
			break;
		case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
			break;
		case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
			break;
		default:
			if (inentry) {
				form_driver(form, input);
			}
			break;
		}
	}

	sleep(sleeptime);

	curs_set(0);

	return output;
}

int
do_mixedform(struct config conf, char* text, int rows, int cols,
    int formheight, int nitems, struct formitem *items)
{
	WINDOW *widget, *button, *entry, *shadow;
	char *buttons[4];
	int i, values[4], output, nbuttons, defbutton, color, y, x;
	FIELD **field;
	FORM *form;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	entry = new_window(y + rows - 3 - formheight -2, x +1,
	    formheight+2, cols-2, NULL, NULL, conf.no_lines ? NOLINES : LOWERED,
	    conf.ascii_lines, false);
	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	field = calloc(nitems + 1, sizeof(FIELD*));
	for (i=0; i < nitems; i++) {
		field[i] = new_field(1, items[i].itemlen, items[i].yitem-1, items[i].xitem-1, 0, 1);
		field_opts_off(field[i], O_STATIC);
		set_max_field(field[i], items[i].inputlen);
		set_field_buffer(field[i], 0, items[i].item);
		set_field_buffer(field[i], 1, items[i].item);
		field_opts_off(field[i], O_AUTOSKIP);
		field_opts_off(field[i], O_BLANK);
		//field_opts_off(field[i], O_BS_OVERLOAD);

		if (ISITEMHIDDEN(items[i]))
			field_opts_off(field[i], O_PUBLIC);

		if (ISITEMREADONLY(items[i])) {
			field_opts_off(field[i], O_EDIT);
			field_opts_off(field[i], O_ACTIVE);
			color = s.fieldreadonlycolor;
		} else {
			color = i == 0 ? s.currfieldcolor : s.fieldcolor;
		}
		set_field_fore(field[i], color);
		set_field_back(field[i], color);
	}
	field[i] = NULL;

	if (nitems == 1) {// inputbox or passwordbox
		set_field_fore(field[0], s.widgetcolor);
		set_field_back(field[0], s.widgetcolor);
	}

	form = new_form(field);
	set_form_win(form, entry);
	set_form_sub(form, derwin(entry, nitems, cols-4, 1, 1));
	post_form(form);

	for (i=0; i < nitems; i++)
		mvwaddstr(entry, items[i].ylabel, items[i].xlabel, items[i].label);

	wrefresh(entry);

	output = mixedform_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, entry, form, field, nitems, items, conf.sleep,
	    conf.output_fd);

	unpost_form(form);
	free_form(form);
	for (i=0; i < nitems; i++)
		free_field(field[i]);
	free(field);

	delwin(button);
	delwin(entry);
	widget_end(conf, "Mixedform", widget, rows, cols, shadow);

	return output;
}

int bsddialog_inputbox(struct config conf, char* text, int rows, int cols)
{
	int output;
	struct formitem item;

	item.label	= "";
	item.ylabel	= 0;
	item.xlabel	= 0;
	item.item	= ""; // TODO add argv
	item.yitem	= 1;
	item.xitem	= 1;
	item.itemlen	= cols-4;
	item.inputlen	= 2048; // todo conf.sizeinput
	item.itemflags	= 0;

	output = do_mixedform(conf, text, rows, cols, 1, 1, &item);

	return output;
}

int bsddialog_passwordbox(struct config conf, char* text, int rows, int cols)
{
	int output;
	struct formitem item;

	item.label	= "";
	item.ylabel	= 0;
	item.xlabel	= 0;
	item.item	= ""; // TODO add argv
	item.yitem	= 1;
	item.xitem	= 1;
	item.itemlen	= cols-4;
	item.inputlen	= 2048; // todo conf.sizeinput
	item.itemflags	= ITEMHIDDEN;

	output = do_mixedform(conf, text, rows, cols, 1, 1, &item);

	return output;
}

int
bsddialog_mixedform(struct config conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv)
{
	int i, output, nitems;
	struct formitem items[128];

	if ((argc % 9) != 0)
		return (-1);

	nitems = argc / 9;
	for (i=0; i<nitems; i++) {
		items[i].label	   = argv[9*i];
		items[i].ylabel	   = atoi(argv[9*i+1]);
		items[i].xlabel	   = atoi(argv[9*i+2]);
		items[i].item	   = argv[9*i+3];
		items[i].yitem	   = atoi(argv[9*i+4]);
		items[i].xitem	   = atoi(argv[9*i+5]);
		items[i].itemlen   = atoi(argv[9*i+6]);
		items[i].inputlen  = atoi(argv[9*i+7]);
		items[i].itemflags = atoi(argv[9*i+8]);
	}

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

int
bsddialog_form(struct config conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv)
{
	int i, output, nitems, itemlen, inputlen;
	unsigned int flags = 0;
	struct formitem items[128];

	if ((argc % 8) != 0)
		return (-1);

	nitems = argc / 8;
	for (i=0; i<nitems; i++) {
		items[i].label	   = argv[8*i];
		items[i].ylabel	   = atoi(argv[8*i+1]);
		items[i].xlabel	   = atoi(argv[8*i+2]);
		items[i].item	   = argv[8*i+3];
		items[i].yitem	   = atoi(argv[8*i+4]);
		items[i].xitem	   = atoi(argv[8*i+5]);

		itemlen = atoi(argv[8*i+6]);
		items[i].itemlen   = abs(itemlen);

		inputlen = atoi(argv[8*i+7]);
		items[i].inputlen = inputlen == 0 ? abs(itemlen) : inputlen;

		flags = flags | (itemlen < 0 ? ITEMREADONLY : 0);
		items[i].itemflags = flags;
	}

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

int
bsddialog_passwordform(struct config conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv)
{
	int i, output, nitems, itemlen, inputlen;
	unsigned int flags = ITEMHIDDEN;
	struct formitem items[128];

	if ((argc % 8) != 0)
		return (-1);

	nitems = argc / 8;
	for (i=0; i<nitems; i++) {
		items[i].label	   = argv[8*i];
		items[i].ylabel	   = atoi(argv[8*i+1]);
		items[i].xlabel	   = atoi(argv[8*i+2]);
		items[i].item	   = argv[8*i+3];
		items[i].yitem	   = atoi(argv[8*i+4]);
		items[i].xitem	   = atoi(argv[8*i+5]);

		itemlen = atoi(argv[8*i+6]);
		items[i].itemlen   = abs(itemlen);

		inputlen = atoi(argv[8*i+7]);
		items[i].inputlen = inputlen == 0 ? abs(itemlen) : inputlen;

		flags = flags | (itemlen < 0 ? ITEMREADONLY : 0);
		items[i].itemflags = flags;
	}

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

/*
 * Bar: gauge, mixedgauge, rangebox and pause
 */

void draw_perc_bar(WINDOW *win, int y, int x, int size, int perc)
{
	char percstr[5];
	int i, blue_x, color;

	blue_x = (int)((perc*(size))/100);

	wmove(win, y, x);
	for (i = 0; i < size; i++) {
		color = (i <= blue_x) ? s.currbarcolor : s.barcolor;
		wattron(win, color);
		waddch(win, ' ');
		wattroff(win, color);
	}

	sprintf(percstr, "%3d%%", perc);
	wmove(win, y, x + size/2 - 2);
	for (i=0; i<4; i++) {
		color = ( (blue_x + 1) <= (size/2 - 2 + i) ) ?
		    s.barcolor : s.currbarcolor;
		wattron(win, color);
		waddch(win, percstr[i]);
		wattroff(win, color);
	}
}

int bsddialog_gauge(struct config conf, char* text, int rows, int cols, int perc)
{
	WINDOW *widget, *bar, *shadow;
	char input[2048];
	int i, y, x;
	bool mainloop = true;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	bar = new_window(y+rows -4, x+3, 3, cols-6, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false);

	wrefresh(widget);
	wrefresh(bar);

	while (mainloop) {
		draw_perc_bar(bar, 1, 1, cols-8, perc);

		wrefresh(widget);
		wrefresh(bar);

		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
		}
		scanf("%d", &perc);
		perc = perc < 0 ? 0 : perc;
		perc = perc > 100 ? 100 : perc;
		i = 2;
		wmove(widget, 1, 1);
		wclrtoeol(widget);
		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
			print_text_multiline(widget, 1, i, input, cols - 4);
			i = i + strlen(input) + 1;
			wrefresh(widget);
		}
	}

	if (conf.sleep > 0)
		sleep(conf.sleep);

	delwin(bar);
	widget_end(conf, "Gauge", widget, rows, cols, shadow);

	return BSDDIALOG_YESOK;
}

int bsddialog_mixedgauge(struct config conf, char* text, int rows, int cols,
    unsigned int perc, int argc, char **argv)
{
WINDOW *widget, *bar, *shadow;
	int i, miniperc, y, x;
	char states[11][16] = {
	    "[  Succeeded  ]",
	    "[   Failed    ]",
	    "[   Passed    ]",
	    "[  Completed  ]",
	    "[   Checked   ]",
	    "[    Done     ]",
	    "[   Skipped   ]",
	    "[ In Progress ]",
	    "!!!  BLANK  !!!",
	    "[     N/A     ]",
	    "[   UNKNOWN   ]",};

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, NULL, &rows, &cols, &shadow) < 0)
		return -1;

	bar = new_window(y+rows -4, x+3, 3, cols-6, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false);

	/* mini bars */
	for (i=0; i < (argc/2); i++) {
		miniperc = atol(argv[i*2 + 1]);
		if (miniperc == 8)
			continue;
		mvwaddstr(widget, i+1, 2, argv[i*2]);
		if (miniperc > 9)
			mvwaddstr(widget, i+1, cols-2-15, states[10]);
		else if (miniperc >= 0 && miniperc <= 9)
			mvwaddstr(widget, i+1, cols-2-15, states[miniperc]);
		else { //miniperc < 0
			miniperc = abs(miniperc);
			mvwaddstr(widget, i+1, cols-2-15, "[             ]");
			draw_perc_bar(widget, i+1, 1+cols-2-15, 13, miniperc);
		}
	}

	print_text_multiline(widget, rows -6, 2, text, cols - 4);

	/* main bar */
	draw_perc_bar(bar, 1, 1, cols-8, perc);

	wattron(bar, s.barcolor);
	mvwaddstr(bar, 0, 2, "Overall Progress");
	wattroff(bar, s.barcolor);

	wrefresh(widget);
	wrefresh(bar);

	getch();

	if (conf.sleep > 0)
		sleep(conf.sleep);

	delwin(bar);
	widget_end(conf, "Mixedgaugebox", widget, rows, cols, shadow);

	return BSDDIALOG_YESOK;
}

int bar_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *bar, int sizebar, int min,
    int max, int def, int sleeptime, int fd)
{
	bool loop, buttupdate, barupdate;
	int i, input, currvalue, output, pos, color;
	float unitxpos;
	char valuestr[128];

	currvalue = def;
	sizebar = sizebar - 2;
	unitxpos = ((float)(max - min + 1))/sizebar;

	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			pos = (int)( ((float)(currvalue - min)) / unitxpos);
			for (i = 0; i < sizebar; i++) {
				color = i <= pos ? s.currbarcolor : s.barcolor;
				wattron(bar, color);
				mvwaddch(bar, 1, i + 1, ' ');
				wattroff(bar, color);
			}
			sprintf(valuestr, "%d", currvalue);
			wmove(bar, 1, sizebar/2 - strlen(valuestr)/2 + 1);
			for (i=0; i<strlen(valuestr); i++) {
				color = (pos < sizebar/2 - strlen(valuestr)/2 + i) ?
				    s.barcolor : s.currbarcolor;
				wattron(bar, color);
				waddch(bar, valuestr[i]);
				wattroff(bar, color);
			}
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(buttwin, cols, nbuttons, buttons, selected,
			    shortkey);
			wrefresh(buttwin);
			buttupdate = false;
		}

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selected]; // values -> outputs
			loop = false;
			dprintf(fd, "%d", currvalue);
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			selected = (selected + 1) % nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (selected > 0) {
				selected--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (selected < nbuttons - 1) {
				selected++;
				buttupdate = true;
			}
			break;
		case KEY_UP:
			if (currvalue < max) {
				currvalue++;
				barupdate = true;
			}
			break;
		case KEY_DOWN:
			if (currvalue > min) {
				currvalue--;
				barupdate = true;
			}
			break;
		}
	}

	sleep(sleeptime);

	return output;
}

int bsddialog_rangebox(struct config conf, char* text, int rows, int cols, int min, int max, int def)
{
	WINDOW *widget, *button, *bar, *shadow;
	char*buttons[4];
	int nbuttons, defbutton, values[4], y, x;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	bar = new_window(y + rows - 6, x +7, 3, cols-14, NULL, NULL,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false);
	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel,
	    conf.cancel_label, conf.help_button, conf.help_label,
	    conf.defaultno, &defbutton);

	bar_handler(button, cols, nbuttons, buttons, values, defbutton, true,
	    bar, cols-14, min, max, def, conf.sleep, conf.output_fd);

	delwin(button);
	delwin(bar);
	widget_end(conf, "Rangebox", widget, rows, cols, shadow);

	return BSDDIALOG_YESOK;
}

int pause_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *bar, int sizebar, int sec,
    int sleeptime)
{
	bool loop, buttupdate, barupdate;
	int i, input, currvalue, output, pos, color;
	float unitxpos;
	char valuestr[128];

	currvalue = sec;
	sizebar = sizebar - 2;
	unitxpos = ((float)sizebar) / sec;

	nodelay(stdscr, TRUE);
	timeout(1000);
	//wtimeout(buttwin, 2);
	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			pos = (int)(currvalue * unitxpos);
			for (i = 0; i < sizebar; i++) {
				color = i <= pos ? s.currbarcolor : s.barcolor;
				wattron(bar, color);
				mvwaddch(bar, 1, i + 1, ' ');
				wattroff(bar, color);
			}
			sprintf(valuestr, "%d", currvalue);
			wmove(bar, 1, sizebar/2 - strlen(valuestr)/2 + 1);
			for (i=0; i<strlen(valuestr); i++) {
				color = (pos < sizebar/2 - strlen(valuestr)/2 + i) ?
				    s.barcolor : s.currbarcolor;
				wattron(bar, color);
				waddch(bar, valuestr[i]);
				wattroff(bar, color);
			}
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(buttwin, cols, nbuttons, buttons, selected,
			    shortkey);
			wrefresh(buttwin);
			buttupdate = false;
		}

		input = getch();
		if(input < 0) {
			currvalue--;
			if (currvalue < 0) {
				output = BSDDIALOG_ERROR;
				break;
			}
			else {
				barupdate = true;
				continue;
			}
		}
		switch(input) {
		case 10: // Enter
			output = values[selected]; // values -> outputs
			loop = false;
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			selected = (selected + 1) % nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (selected > 0) {
				selected--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (selected < nbuttons - 1) {
				selected++;
				buttupdate = true;
			}
			break;
		}
	}

	nodelay(stdscr, FALSE);

	sleep(sleeptime);

	return output;
}

int bsddialog_pause(struct config conf, char* text, int rows, int cols, int sec)
{
	WINDOW *widget, *button, *bar, *shadow;
	char*buttons[4];
	int output, nbuttons, defbutton, values[4], y, x;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	bar = new_window(y + rows - 6, x +7, 3, cols-14, NULL, NULL,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false);
	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel,
	    conf.cancel_label, conf.help_button, conf.help_label,
	    conf.defaultno, &defbutton);

	output = pause_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, bar, cols-14, sec, conf.sleep);

	delwin(button);
	delwin(bar);
	widget_end(conf, "Pause", widget, rows, cols, shadow);

	return output;
}

/* timebox and calendar-todo */

int bsddialog_timebox(struct config conf, char* text, int rows, int cols,
    unsigned int hh, unsigned int mm, unsigned int ss)
{
	WINDOW *widget, *button, *hhwin, *mmwin, *sswin, *shadow;
	char *buttons[4];
	int input, output, nbuttons, selbutton, values[4], y, x;
	bool loop, buttupdate, inhh, inmm, inss;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	hhwin = new_window(y + rows - 6, x + cols/2 - 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	mvwaddch(widget, rows - 5, cols/2 - 3, ':');
	mmwin = new_window(y + rows - 6, x + cols/2 - 2, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	mvwaddch(widget, rows - 5, cols/2 + 2, ':');
	sswin = new_window(y + rows - 6, x + cols/2 + 3, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel,
	    conf.cancel_label, conf.help_button, conf.help_label,
	    conf.defaultno, &selbutton);

	mvwprintw(hhwin, 1, 1, "%2d", hh);
	wrefresh(hhwin);
	mvwprintw(mmwin, 1, 1, "%2d", mm);
	wrefresh(mmwin);
	mvwprintw(sswin, 1, 1, "%2d", ss);
	wrefresh(sswin);

	curs_set(2);
	inmm = inss = false;
	loop = buttupdate = inhh = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, selbutton,
			    true);
			wrefresh(button);
			buttupdate = false;
		}

		if (inhh) {
			mvwprintw(hhwin, 1, 1, "%2d", hh);
			wmove(hhwin, 1, 2);
			wrefresh(hhwin);
		} else if (inmm) {
			mvwprintw(mmwin, 1, 1, "%2d", mm);
			wmove(mmwin, 1, 2);
			wrefresh(mmwin);
		} else { //inss
			mvwprintw(sswin, 1, 1, "%2d", ss);
			wmove(sswin, 1, 2);
			wrefresh(sswin);
		}

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selbutton]; // values -> outputs
			loop = false;
			dprintf(conf.output_fd, "%u:%u:%u", hh, mm, ss);
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			if (inhh) {
				inhh = false;
				inmm = true;
			} else if (inmm) {
				inmm = false;
				inss = true;
			} else { //inss
				inss = false;
				inhh = true;
			}
			break;
		case KEY_LEFT:
			if (selbutton > 0) {
				selbutton--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (selbutton < nbuttons - 1) {
				selbutton++;
				buttupdate = true;
			}
			break;
		case KEY_UP:
			if (inhh) {
				hh = hh < 24 ? hh + 1 : 0;
			} else if (inmm) {
				mm = mm < 60 ? mm + 1 : 0;
			} else { //inss
				ss = ss < 60 ? ss + 1 : 0;
			}
			break;
		case KEY_DOWN:
			if (inhh) {
				hh = hh > 0 ? hh - 1 : 24;
			} else if (inmm) {
				mm = mm > 0 ? mm - 1 : 60;
			} else { //inss
				ss = ss > 0 ? ss - 1 : 60;
			}
			break;
		}
	}

	curs_set(0);

	if (conf.sleep > 0)
		sleep(conf.sleep);

	delwin(button);
	delwin(hhwin);
	delwin(mmwin);
	delwin(sswin);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}

/*
 * prgbox, programbox and progressbox
 */

int
bsddialog_prgbox(struct config conf, char* text, int rows, int cols, char *command)
{
	return 0;
}

int bsddialog_programbox(struct config conf, char* text, int rows, int cols)
{
	char line[MAXINPUT];
	WINDOW *widget, *pad, *button, *shadow;
	int i, y, x, padrows, padcols, ys, ye, xs, xe;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;

	y = conf.y;
	x = conf.x;
	widget = shadow = NULL;
	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows - 3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel,
	    conf.cancel_label, conf.help_button, conf.help_label,
	    conf.defaultno, &defbutton);

	if (text != NULL && conf.no_lines == false) {
		mvwhline(widget, 2, 2, conf.ascii_lines ? '-' : ACS_HLINE, cols -4);
		wrefresh(widget);
	}

	padrows = text == NULL ? rows - 4 : rows - 6;
	padcols = cols - 2;
	ys = text == NULL ? y + 1 : y + 3;
	xs = x + 1;
	ye = ys + padrows;
	xe = xs + padcols;

	pad = newpad(padrows, padcols);

	i = 0;
	//while (fgets(line, MAXINPUT, stdin) != NULL) {
	while(getstr(line) != ERR){
		mvwaddstr(pad, i, 0, line);
		prefresh(pad, 0, 0, ys, xs, ye, xe);
		i++;
	}
	
	output = buttons_handler(button, cols, nbuttons, buttons, values, defbutton,
	    true, conf.sleep, /*fd*/ 0);

	return output;
}

int bsddialog_progressbox(struct config conf, char* text, int rows, int cols)
{
	return 0;
}

