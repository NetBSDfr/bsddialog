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
#include <time.h>
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

struct bsddialogtheme t;

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MAXINPUT 2048
#define TREESPACE 4

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
    int *values, int selected, bool shortkey, int fd);


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

	set_theme(DEFAULT, &t); /* EIGHTCOLORS WHIPTAILPURPLE WHIPTAIL */

	bkgd(t.backgroundcolor);

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
	wbkgd(popup, t.widgetcolor);

	if (elev != NOLINES) {
		if (asciilines) {
			ls = rs = '|';
			ts = bs = '-';
			tl = tr = bl = br = ltee = rtee = '+';
		}
		leftcolor  = elev == RAISED ? t.lineraisecolor : t.linelowercolor;
		rightcolor = elev == RAISED ? t.linelowercolor : t.lineraisecolor;
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
		if (t.surroundtitle && elev != NOLINES) {
			wattron(popup, rightcolor);
			mvwaddch(popup, 0, cols/2 - strlen(title)/2 - 1, rtee);
			mvwaddch(popup, 0, cols/2 + strlen(title)/2 + 1, ltee);
			wattroff(popup, leftcolor);
		}

		wattron(popup, t.titlecolor);
		wmove(popup, 0, cols/2 - strlen(title)/2);
		waddstr(popup, title);
		wattroff(popup, t.titlecolor);
	}

	if (bottomtitle != NULL) {
		wattron(popup, t.bottomtitlecolor);
		wmove(popup, rows - 1, cols/2 - strlen(bottomtitle)/2 - 1);
		waddch(popup, '[');
		waddstr(popup, bottomtitle);
		waddch(popup, ']');
		wattroff(popup, t.bottomtitlecolor);
	}

	return popup;
}

void window_scrolling_handler(WINDOW *pad, int rows, int cols)
{
	int input, cur_line = 0, shown_lines;
	bool loop = true;
	int x = 2, y = COLS/2 - cols/2; /* tofix x & y*/

	shown_lines = rows > (LINES - x - 1) ? (LINES - x - 1) : rows;
	wattron(pad, t.widgetcolor);
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
	wattroff(pad, t.widgetcolor);
}

void
draw_button(WINDOW *window, int x, int size, char *text, bool selected,
    bool shortkey)
{
	int i, color_arrows, color_shortkey, color_button;

	if (selected) {
		color_arrows = t.currshortkeycolor;
		color_shortkey = t.currshortkeycolor;
		color_button = t.currbuttoncolor;
	} else {
		color_arrows = t.buttoncolor;
		color_shortkey = t.shortkeycolor;
		color_button = t.buttoncolor;
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

	*y = (conf.y < 0) ? (LINES/2 - *h/2 - 1) : conf.y;
	*x = (conf.x < 0) ? (COLS/2 - *w/2) : conf.x;

	if (conf.shadow) {
		if ((*shadow = newwin(*h, *w+1, *y+1, *x+1)) == NULL)
			return -1;
		wbkgd(*shadow, t.shadowcolor);
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

	if (conf.sleep > 0)
		sleep(conf.sleep);

	delwin(window);

	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "%s size: (%d, %d)\n", name, h, w);
}

/*
 * SECTIONS
 *  1) "Info"    infobox
 *  2) "Button"  msgbox - yesno
 *  3) "Menu"    buildlist - checklist - menu - radiolist - treeview
 *  4) "Form"    inputbox - passwordbox - form - passwordform - mixedform
 *  5) "Editor"  editbox(todo)
 *  6) "Bar"     gauge - mixedgauge - rangebox - pause
 *  7) "Time"    timebox - calendar
 *  8) "Command" prgbox - programbox(todo) - progressbox(todo)
 *  9) "Text"    tailbox - tailboxbg(todo) - textbox
 * 10) "File"    dselect(todo) - fselect(todo)
 */

/*
 * SECTION 1 "Info": infobox
 */
int
bsddialog_infobox(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *shadow;
	int y, x;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	getch();

	widget_end(conf, "Infobox", widget, rows, cols, shadow);

	return (BSDDIALOG_YESOK);
}

/*
 * SECTION 2 "Button": msgbox - yesno
 */
int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int fd)
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

	return output;
}

int
bsddialog_msgbox(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[3];
	int values[3], output, nbuttons, defbutton, y, x;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, false, NULL, conf.help_button,
	    conf.help_label, false, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values, 0,
	    true, /*fd*/ 0);

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

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.yes_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.no_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, /*fd*/ 0);

	delwin(button);
	widget_end(conf, "Yesno", widget, rows, cols, shadow);

	return output;
}

/*
 * SECTION 3 "Menu": checklist - menu - radiolist - treeview - buildlist
 */
enum menumode { BUILDLISTMODE, CHECKLISTMODE, MENUMODE, RADIOLISTMODE, TREEVIEWMODE };

void
draw_myitem(WINDOW *pad, int y, struct bsddialog_menuitem item, enum menumode mode,
    int xdesc, bool selected, bool bottomdesc)
{
	int color, colorname;

	color = selected ? t.curritemcolor : t.itemcolor;
	colorname = selected ? t.currtagcolor : t.tagcolor;

	wmove(pad, y, 0);
	wattron(pad, color);
	if (mode == CHECKLISTMODE)
		wprintw(pad, "[%c]", item.on ? 'X' : ' ');
	if (mode == RADIOLISTMODE || mode == TREEVIEWMODE)
		wprintw(pad, "(%c)", item.on ? '*' : ' ');
	wattron(pad, color);

	if (mode != BUILDLISTMODE && mode != TREEVIEWMODE) {
		wattron(pad, colorname);
		if (mode != MENUMODE)
			wmove(pad, y, 4);
		waddstr(pad, item.name);
		wattron(pad, colorname);
	}

	if ((mode == BUILDLISTMODE || mode == TREEVIEWMODE) && selected == false)
		color = item.on ? t.tagcolor : t.itemcolor;
	wattron(pad, color);
	if (mode == CHECKLISTMODE || mode == RADIOLISTMODE || mode == TREEVIEWMODE)
		xdesc +=4;
	if (mode == TREEVIEWMODE)
		xdesc = xdesc + item.depth * TREESPACE;
	mvwaddstr(pad, y, xdesc, item.desc);
	wattron(pad, color);

	if (selected && bottomdesc == true) {
		move(LINES-2, 5);
		clrtoeol();
		addstr(item.bottomdesc);
		
		refresh();
	}
}

int
do_menu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int line, int xdesc, enum menumode mode,
    int nitems, struct bsddialog_menuitem *items)
{
	WINDOW *widget, *button, *menuwin, *menupad, *shadow;
	char *buttons[4], *sepstr, quotech;
	int i, values[4], output, nbuttons, defbutton, y, x, input, curr;
	int ys, ye, xs, xe;
	bool loop, buttupdate, sep;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	menuwin = new_window(y + rows - 5 - menurows, x + 2, menurows+2, cols-4,
	    NULL, NULL, conf.no_lines ? NOLINES : LOWERED,
	    conf.ascii_lines, false);
	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	menupad = newpad(nitems, line);
	wbkgd(menupad, t.widgetcolor);

	curr = -1;
	if (conf.default_item != NULL) {
		for (i=0; i<nitems; i++) {
			if (strcmp(items[i].name, conf.default_item) == 0) {
				curr = i;
				break;
			}
		}
	}
	curr = curr < 0 ? 0 : curr;
	for (i=0; i<nitems; i++)
		draw_myitem(menupad, i, items[i], mode, xdesc, i == curr, conf.item_help);

	ys = y + rows - 5 - menurows + 1;
	ye = ys + menurows + 2 -1;
	xs = (line > cols - 6) ? (x + 2 + 1) : x + 3 + (cols-6)/2 - line/2;
	xe = (line > cols - 6) ? xs + cols - 7 : xs + cols - 4 -1;
	if (mode == TREEVIEWMODE)
		xs = x + 2 + 1;

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	    conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	wrefresh(menuwin);
	prefresh(menupad, 0, 0, ys, xs, ye, xe);//delete?

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
			draw_myitem(menupad, curr, items[curr], mode, xdesc, false, conf.item_help);
			curr = (curr > 0) ? curr - 1 : 0;
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true, conf.item_help);
			break;
		case KEY_DOWN:
			draw_myitem(menupad, curr, items[curr], mode, xdesc, false, conf.item_help);
			curr = (curr < nitems-1) ? curr +1 : nitems-1;
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true, conf.item_help);
			break;
		case ' ': /* Space */
			if (mode == MENUMODE)
				break;
			else if (mode == CHECKLISTMODE)
				items[curr].on = ! items[curr].on;
			else { //RADIOLISTMODE and TREEVIEWMODE
				if (items[curr].on == true)
					break;
				for (i=0; i<nitems; i++)
					if (items[i].on == true) {
						items[i].on = false;
						draw_myitem(menupad, i, items[i],
						    mode, xdesc, false, conf.item_help);
					}
				items[curr].on = true;
			}
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true, conf.item_help);
			break;
		default:
			
			break;
		}
	}

	sep = false;
	quotech = conf.single_quoted ? '\'' : '"';

	if (output == BSDDIALOG_HELP && nitems >0) {
		dprintf(conf.output_fd, "HELP %s", items[curr].name);
		sep = true;
	}

	sepstr = conf.separate_output ? "\n" : " ";

	if ((output == BSDDIALOG_YESOK || conf.help_status == true) && nitems > 0) {
		if (mode == MENUMODE) {
			dprintf(conf.output_fd, "%s", items[curr].name);
			items[curr].on = true; // for library
		} else { /* CHECKLIST or RADIOLIST */
			for (i=0; i<nitems; i++) {
				if (items[i].on == true) {
					if (sep == true)
					    dprintf(conf.output_fd, "%s", sepstr);
					sep = true;
					dprintf(conf.output_fd, "%s",items[i].name);
				}
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
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxname, maxdesc;

	line = maxname = maxdesc = 0;
	for (i=0; i<nitems; i++) {
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
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxname, maxdesc;

	line = maxname = maxdesc = 0;
	for (i=0; i<nitems; i++) {
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
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxname, maxdesc;
	bool on = false;

	line = maxname = maxdesc = 0;
	for (i=0; i<nitems; i++) {
		if (on == true)
			items[i].on = false;

		if (items[i].on == true)
			on = true;

		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc + 4);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    RADIOLISTMODE, nitems, items);

	return output;
}

int
bsddialog_treeview(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxdesc;
	bool on = false;

	line = maxdesc = 0;
	for (i=0; i<nitems; i++) {
		if (on == true)
			items[i].on = false;

		if (items[i].on == true)
			on = true;
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxdesc + 4 + items[i].depth * TREESPACE);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, 0,
	    TREEVIEWMODE, nitems, items);

	return output;
}

int
do_buildlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int line, int nitems, struct bsddialog_menuitem *items,
    bool startleft)
{
	WINDOW *widget, *button, *leftwin, *leftpad, *rightwin, *rightpad, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton, i, x, y, input;
	bool loop, buttupdate, padsupdate;
	int nlefts, nrights, leftwinx, rightwinx, winsy, padscols, curr;
	enum side {LEFT, RIGHT} currV;
	int currH;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	winsy = y + rows - 5 - menurows;
	leftwinx = x+2;
	leftwin = new_window(winsy, leftwinx, menurows+2, (cols-5)/2, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	rightwinx = x + cols - 2 -(cols-5)/2;
	rightwin = new_window(winsy, rightwinx, menurows+2, (cols-5)/2, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	wrefresh(leftwin);
	wrefresh(rightwin);

	padscols = (cols-5)/2 - 2;
	leftpad  = newpad(nitems, line);
	rightpad = newpad(nitems, line);
	wbkgd(leftpad, t.widgetcolor);
	wbkgd(rightpad, t.widgetcolor);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	    conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	currH = 0;
	currV = startleft ? LEFT : RIGHT;
	loop = buttupdate = padsupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, defbutton,
			    true);
			wrefresh(button);
			buttupdate = false;
		}

		if (padsupdate) {
			werase(leftpad);
			werase(rightpad);
			curr = -1;
			nlefts = nrights = 0;
			for (i=0; i<nitems; i++) {
				if (items[i].on == false) {
					if (currV == LEFT && currH == nlefts)
						curr = i;
					draw_myitem(leftpad, nlefts, items[i],
					    BUILDLISTMODE, 0, curr == i, conf.item_help);
					nlefts++;
				} else {
					if (currV == RIGHT && currH == nrights)
						curr = i;
					draw_myitem(rightpad, nrights, items[i],
					    BUILDLISTMODE, 0, curr == i, conf.item_help);
					nrights++;
				}
			}
			prefresh(leftpad, 0, 0, winsy+1, leftwinx+1, winsy+1+menurows, leftwinx + 1 + padscols);
			prefresh(rightpad, 0, 0, winsy+1, rightwinx+1, winsy+1+menurows, rightwinx + 1 + padscols);
			padsupdate = false;
		}

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
		}

		if (nitems <= 0)
			continue;

		switch(input) {
		case KEY_LEFT:
			if (currV == RIGHT && nrights > 0) {
				currV = LEFT;
				currH = 0;
				padsupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (currV == LEFT && nrights > 0) {
				currV = RIGHT;
				currH = 0;
				padsupdate = true;
			}
			break;
		case KEY_UP:
			currH = (currH > 0) ? currH - 1 : 0;
			padsupdate = true;
			break;
		case KEY_DOWN:
			if (currV == LEFT)
				currH = (currH < nlefts-1) ? currH +1 : currH;
			else
				currH = (currH < nrights-1)? currH +1 : currH;
			padsupdate = true;
			break;
		case ' ': // Space
			items[curr].on = ! items[curr].on;
			if (currV == LEFT) {
				if (nlefts > 1)
					currH = currH > 0 ? currH-1 : 0;
				else {
					currH = 0;
					currV = RIGHT;
				}
			} else {
				if (nrights > 1)
					currH = currH > 0 ? currH-1 : 0;
				else {
					currH = 0;
					currV = LEFT;
				}
			}
			padsupdate = true;
			break;
		default:

			break;
		}
	}

	delwin(button);
	delwin(leftpad);
	delwin(leftwin);
	delwin(rightpad);
	delwin(rightwin);
	widget_end(conf, "Buildlist", widget, rows, cols, shadow);

	return output;
}

int
bsddialog_buildlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line;
	bool startleft = false;

	line = 0;
	for (i=0; i<nitems; i++) {
		line = MAX(line, strlen(items[i].desc));
		if (items[i].on == false)
			startleft = true;
	}

	output = do_buildlist(conf, text, rows, cols, menurows, line, nitems,
	    items, startleft);

	return output;
}

/*
 * SECTIONS 4 "Form": inputbox - passwordbox - form - passwordform - mixedform
 */
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
    FIELD **field, int nitems, struct formitem *items, int fd)
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
			set_field_fore(current_field(form), t.fieldcolor);
			set_field_back(current_field(form), t.fieldcolor);
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), t.currfieldcolor);
			set_field_back(current_field(form), t.currfieldcolor);
			break;
		case KEY_DOWN:
			if (nitems < 2)
				break;
			set_field_fore(current_field(form), t.fieldcolor);
			set_field_back(current_field(form), t.fieldcolor);
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), t.currfieldcolor);
			set_field_back(current_field(form), t.currfieldcolor);
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
			color = t.fieldreadonlycolor;
		} else {
			color = i == 0 ? t.currfieldcolor : t.fieldcolor;
		}
		set_field_fore(field[i], color);
		set_field_back(field[i], color);
	}
	field[i] = NULL;

	if (nitems == 1) {// inputbox or passwordbox
		set_field_fore(field[0], t.widgetcolor);
		set_field_back(field[0], t.widgetcolor);
	}

	form = new_form(field);
	set_form_win(form, entry);
	set_form_sub(form, derwin(entry, nitems, cols-4, 1, 1));
	post_form(form);

	for (i=0; i < nitems; i++)
		mvwaddstr(entry, items[i].ylabel, items[i].xlabel, items[i].label);

	wrefresh(entry);

	output = mixedform_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, entry, form, field, nitems, items, conf.output_fd);

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
 * SECTION 5 "Editor":  editbox(todo)
 */
int bsddialog_editbox(struct config conf, char* text, int rows, int cols)
{

	return (BSDDIALOG_ERROR);
}

/*
 * SECTION 6 "Bar": gauge - mixedgauge - rangebox - pause
 */
void
draw_perc_bar(WINDOW *win, int y, int x, int size, int perc, bool withlabel, int label)
{
	char labelstr[128];
	int i, blue_x, color;

	blue_x = (int)((perc*(size))/100);

	wmove(win, y, x);
	for (i = 0; i < size; i++) {
		color = (i <= blue_x) ? t.currbarcolor : t.barcolor;
		wattron(win, color);
		waddch(win, ' ');
		wattroff(win, color);
	}

	if (withlabel)
		sprintf(labelstr, "%d", label);
	else
		sprintf(labelstr, "%3d%%", perc);
	wmove(win, y, x + size/2 - 2);
	for (i=0; i<strlen(labelstr); i++) {
		color = ( (blue_x + 1) <= (size/2 - strlen(labelstr)/2 + i) ) ?
		    t.barcolor : t.currbarcolor;
		wattron(win, color);
		waddch(win, labelstr[i]);
		wattroff(win, color);
	}
}

int bsddialog_gauge(struct config conf, char* text, int rows, int cols, int perc)
{
	WINDOW *widget, *bar, *shadow;
	char input[2048];
	int i, y, x;
	bool mainloop = true;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	bar = new_window(y+rows -4, x+3, 3, cols-6, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false);

	wrefresh(widget);
	wrefresh(bar);

	while (mainloop) {
		draw_perc_bar(bar, 1, 1, cols-8, perc, false, -1 /*unused*/);

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
			draw_perc_bar(widget, i+1, 1+cols-2-15, 13, miniperc,
			    false, -1 /*unused*/);
		}
	}

	print_text_multiline(widget, rows -6, 2, text, cols - 4);

	/* main bar */
	draw_perc_bar(bar, 1, 1, cols-8, perc, false, -1 /*unused*/);

	wattron(bar, t.barcolor);
	mvwaddstr(bar, 0, 2, "Overall Progress");
	wattroff(bar, t.barcolor);

	wrefresh(widget);
	wrefresh(bar);

	getch();

	delwin(bar);
	widget_end(conf, "Mixedgaugebox", widget, rows, cols, shadow);

	return BSDDIALOG_YESOK;
}

int
bsddialog_rangebox(struct config conf, char* text, int rows, int cols, int min,
    int max, int def)
{
	WINDOW *widget, *button, *bar, *shadow;
	char*buttons[4];
	int nbuttons, defbutton, values[4], y, x;
	bool loop, buttupdate, barupdate;
	int input, currvalue, output, sizebar;
	float perc;
	int positions = max - min + 1;

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

	currvalue = def;
	sizebar = cols - 16;
	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			perc = ((float)(currvalue - min)*100) / ((float)positions-1);
			draw_perc_bar(bar, 1, 1, sizebar, perc, true, currvalue);
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, defbutton,
			    true);
			wrefresh(button);
			buttupdate = false;
		}

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[defbutton]; // values -> outputs
			loop = false;
			dprintf(conf.output_fd, "%d", currvalue);
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

	delwin(button);
	delwin(bar);
	widget_end(conf, "Rangebox", widget, rows, cols, shadow);

	return output;
}

int bsddialog_pause(struct config conf, char* text, int rows, int cols, int sec)
{
	WINDOW *widget, *button, *bar, *shadow;
	char*buttons[4];
	int output, nbuttons, defbutton, values[4], y, x;
	bool loop, buttupdate, barupdate;
	int input, currvalue, sizebar;
	float perc;

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

	currvalue = sec;
	sizebar = cols-16;
	nodelay(stdscr, TRUE);
	timeout(1000);
	//wtimeout(buttwin, 2);
	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			perc = ((float)(currvalue*100)) / ((float)sec);
			draw_perc_bar(bar, 1, 1, sizebar, perc, true, currvalue);
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, defbutton,
			    true);
			wrefresh(button);
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
			output = values[defbutton]; // values -> outputs
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
			if (defbutton< nbuttons - 1) {
				defbutton++;
				buttupdate = true;
			}
			break;
		}
	}

	nodelay(stdscr, FALSE);

	delwin(button);
	delwin(bar);
	widget_end(conf, "Pause", widget, rows, cols, shadow);

	return output;
}

/*
 * SECTION 7 "Time": timebox - calendar
 */
int bsddialog_timebox(struct config conf, char* text, int rows, int cols,
    unsigned int hh, unsigned int mm, unsigned int ss)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[4], stringtime[1024];
	int i, input, output, nbuttons, selbutton, values[4], y, x, sel;
	bool loop, buttupdate;
	time_t clock;
	struct tm *localtm;
	struct myclockstruct {
		unsigned int max;
		unsigned int curr;
		WINDOW *win;
	} c[3] = { {23, hh, NULL}, {59, mm, NULL}, {59, ss, NULL} };

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;

	c[0].win = new_window(y + rows - 6, x + cols/2 - 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	mvwaddch(widget, rows - 5, cols/2 - 3, ':');
	c[1].win = new_window(y + rows - 6, x + cols/2 - 2, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	mvwaddch(widget, rows - 5, cols/2 + 2, ':');
	c[2].win = new_window(y + rows - 6, x + cols/2 + 3, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel,
	    conf.cancel_label, conf.help_button, conf.help_label,
	    conf.defaultno, &selbutton);

	sel=0;
	curs_set(2);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, selbutton,
			    true);
			wrefresh(button);
			buttupdate = false;
		}

		for (i=0; i<3; i++) {
			mvwprintw(c[i].win, 1, 1, "%2d", c[i].curr);
			wrefresh(c[i].win);
		}
		wmove(c[sel].win, 1, 2);
		wrefresh(c[sel].win);

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selbutton]; // values -> outputs
			loop = false;
			if (conf.time_format == NULL) {
				dprintf(conf.output_fd, "%u:%u:%u", hh, mm, ss);
			} else {
				time(&clock);
				localtm = localtime(&clock);
				localtm->tm_hour = c[0].curr;
				localtm->tm_min  = c[1].curr;
				localtm->tm_sec  = c[2].curr;
				clock = mktime(localtm);
				localtm = localtime(&clock);
				strftime(stringtime, 1024, conf.time_format, localtm);
				dprintf(conf.output_fd, "%s", stringtime);
			}
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			sel = (sel + 1) % 3;
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
			c[sel].curr = c[sel].curr < c[sel].max ? c[sel].curr + 1 : 0;
			break;
		case KEY_DOWN:
			c[sel].curr = c[sel].curr > 0 ? c[sel].curr - 1 : c[sel].max;
			break;
		}
	}

	curs_set(0);

	delwin(button);
	for (i=0; i<3; i++)
		delwin(c[i].win);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}

int bsddialog_calendar(struct config conf, char* text, int rows, int cols,
    unsigned int yy, unsigned int mm, unsigned int dd)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[4], stringtime[1024];
	int i, input, output, nbuttons, selbutton, values[4], y, x, sel;
	bool loop, buttupdate;
	time_t clock;
	struct tm *localtm;
	struct calendar {
		unsigned int max;
		unsigned int curr;
		WINDOW *win;
		unsigned int x;
	} c[3] = {{9999, yy, NULL, 4 }, {12, mm, NULL, 9 }, {31, dd, NULL, 2 }};
	struct month {
		char *name;
		unsigned int days;
	} m[12] = {
	    { "January", 30 }, { "February", 30 }, { "March",     30 },
	    { "April",   30 }, { "May",      30 }, { "June",      30 },
	    { "July",    30 }, { "August",   30 }, { "September", 30 },
	    { "October", 30 }, { "November", 30 }, { "December",  30 }
	};

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow) < 0)
		return -1;
	c[0].win = new_window(y + rows - 6, x + cols/2 - 12, 3, 6, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	mvwaddch(widget, rows - 5, cols/2 - 6, '/');
	c[1].win = new_window(y + rows - 6, x + cols/2 - 5, 3, 11, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	mvwaddch(widget, rows - 5, cols/2 + 6, '/');
	c[2].win = new_window(y + rows - 6, x + cols/2 + 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);

	wrefresh(widget);

	button = new_window(y + rows -3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, ! conf.no_cancel,
	    conf.cancel_label, conf.help_button, conf.help_label,
	    conf.defaultno, &selbutton);

	sel=2;
	curs_set(2);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, nbuttons, buttons, selbutton,
			    true);
			wrefresh(button);
			buttupdate = false;
		}

		mvwprintw(c[0].win, 1, 1, "%4d", c[0].curr);
		mvwprintw(c[1].win, 1, 1, "%9s", m[c[1].curr-1].name);
		mvwprintw(c[2].win, 1, 1, "%2d", c[2].curr);
		for (i=0; i<3; i++) {
			wrefresh(c[i].win);
		}
		wmove(c[sel].win, 1, c[sel].x);
		wrefresh(c[sel].win);

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selbutton]; // values -> outputs
			loop = false;
			if (conf.date_format == NULL) {
				dprintf(conf.output_fd, "%u/%u/%u",
				    c[0].curr, c[1].curr, c[2].curr);
			} else {
				time(&clock);
				localtm = localtime(&clock);
				localtm->tm_year = c[0].curr - 1900;
				localtm->tm_mon  = c[1].curr;
				localtm->tm_mday = c[2].curr;
				clock = mktime(localtm);
				localtm = localtime(&clock);
				strftime(stringtime, 1024, conf.date_format, localtm);
				dprintf(conf.output_fd, "%s", stringtime);
			}
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			sel = (sel + 1) % 3;
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
			c[sel].curr = c[sel].curr < c[sel].max ? c[sel].curr + 1 : 1;
			break;
		case KEY_DOWN:
			c[sel].curr = c[sel].curr > 1 ? c[sel].curr - 1 : c[sel].max;
			break;
		}
	}

	curs_set(0);

	delwin(button);
	for (i=0; i<3; i++)
		delwin(c[i].win);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}

/*
 * SECTION 8 "Command": prgbox - programbox(todo) - progressbox(todo)
 */
int
bsddialog_prgbox(struct config conf, char* text, int rows, int cols, char *command)
{
	char line[MAXINPUT];
	WINDOW *widget, *pad, *button, *shadow;
	int i, y, x, padrows, padcols, ys, ye, xs, xe;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;
	int pipefd[2];

	if (widget_init(conf, &widget, &y, &x, /*text*/NULL, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows - 3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	    conf.extra_button, conf.extra_label, false, NULL, conf.help_button,
	    conf.help_label, false, &defbutton);

	if (text != NULL && conf.no_lines == false) {
		print_text_multiline(widget, 1, 2, text, cols - 4);
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
	wbkgd(pad, t.widgetcolor);

	pipe(pipefd);
	if (fork() == 0)
	{
		close(pipefd[0]);    // close reading

		dup2(pipefd[1], 1);  // send stdout to the pipe
		dup2(pipefd[1], 2);  // send stderr to the pipe

		close(pipefd[1]);    // this descriptor is no longer needed

		//const char *ls="/bin/ls";
		execl(command, command, NULL);
		return 0;
	}
	else
	{
		close(pipefd[1]);  // close write

		i = 0;
		while (read(pipefd[0], line, MAXINPUT) != 0) {
			mvwaddstr(pad, i, 0, line);
			prefresh(pad, 0, 0, ys, xs, ye, xe);
			i++;
		}
	}

	output = buttons_handler(button, cols, nbuttons, buttons, values, defbutton,
	    true, /*fd*/ 0);

	return output;
}

int bsddialog_programbox(struct config conf, char* text, int rows, int cols)
{
	char line[MAXINPUT];
	WINDOW *widget, *pad, *button, *shadow;
	int i, y, x, padrows, padcols, ys, ye, xs, xe;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;

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
	    true, /*fd*/ 0);

	return output;
}

int bsddialog_progressbox(struct config conf, char* text, int rows, int cols)
{
	return 0;
}

/*
 * SECTION 9 "Text": tailbox - tailboxbg(todo) - textbox
 */
enum textmode { TAILMODE, TAILBGMODE, TEXTMODE};

int
do_text(enum textmode mode, struct config conf, char* path, int rows, int cols)
{
	WINDOW *widget, *pad, *button, *shadow;
	int i, input, y, x, padrows, padcols, ypad, xpad, ys, ye, xs, xe;
	char buf[BUFSIZ], *exitbutt ="EXIT";
	FILE *fp;
	bool loop;

	if (widget_init(conf, &widget, &y, &x, NULL, &rows, &cols, &shadow) < 0)
		return -1;

	button = new_window(y + rows - 3, x, 3, cols, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true);

	exitbutt = conf.exit_label == NULL ? exitbutt : conf.exit_label;
	draw_button(button, (cols-2)/2 - strlen(exitbutt)/2, strlen(exitbutt)+2,
	    exitbutt, true, true);

	wrefresh(button);

	padrows = rows - 4;
	padcols = cols - 2;
	pad = newpad(padrows, padcols);
	wbkgd(pad, t.widgetcolor);

	fp = fopen(path, "r");
	/*if (mode == TAILMODE) {
		fseek (fp, 0, SEEK_END);
		i = nlines = 0;
		while (i < padrows) {
			line = ;
		}
		for (i=padrows-1; i--; i>=0) {
		}
	}*/
	i = 0;
	while(fgets(buf, BUFSIZ, fp) != NULL) {
		if (strlen(buf) > padcols) {
			padcols = strlen(buf);
			wresize(pad, padrows, padcols);
		}
		if (i > padrows-1) {
			padrows++;
			wresize(pad, padrows, padcols);
		}
		mvwaddstr(pad, i, 0, buf);
		i++;
	}

	ys = y + 1;
	xs = x + 1;
	ye = ys + rows-5;
	xe = xs + cols-3;
	ypad = xpad = 0;
	loop = true;
	while(loop) {
		prefresh(pad, ypad, xpad, ys, xs, ye, xe);
		input = getch();
		switch(input) {
		case 10: // Enter
			loop = false;
			break;
		case 27: // Esc
			loop = false;
			break;
		case KEY_LEFT:
			xpad = xpad > 0 ? xpad - 1 : 0;
			break;
		case KEY_RIGHT:
			xpad = (xpad + cols-2) < padcols-1 ? xpad + 1 : xpad;
			break;
		case KEY_UP:
			ypad = ypad > 0 ? ypad - 1 : 0;
			break;
		case KEY_DOWN:
			ypad = (ypad + rows-4) <= padrows ? ypad + 1 : ypad;
			break;
		}
	}

	return (BSDDIALOG_ERROR);
}

int bsddialog_tailbox(struct config conf, char* text, int rows, int cols)
{

	return (do_text(TAILMODE, conf, text, rows, cols));
}

int bsddialog_tailboxbg(struct config conf, char* text, int rows, int cols)
{

	return (do_text(TAILBGMODE, conf, text, rows, cols));
}


int bsddialog_textbox(struct config conf, char* text, int rows, int cols)
{

	return (do_text(TEXTMODE, conf, text, rows, cols));
}

/*
 * SECTION 10 "File": dselect(todo) - fselect(todo)
 */
int bsddialog_dselect(struct config conf, char* text, int rows, int cols)
{

	return (BSDDIALOG_ERROR);
}

int bsddialog_fselect(struct config conf, char* text, int rows, int cols)
{

	return (BSDDIALOG_ERROR);
}
