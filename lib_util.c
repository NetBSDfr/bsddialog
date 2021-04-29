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

#include <string.h>
#include <unistd.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "theme.h"

extern struct bsddialog_theme t;

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
	WINDOW *win;
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

	win = newwin(rows, cols, y, x);
	wbkgd(win, t.widgetcolor);

	if (elev != NOLINES) {
		if (asciilines) {
			ls = rs = '|';
			ts = bs = '-';
			tl = tr = bl = br = ltee = rtee = '+';
		}
		leftcolor  = elev == RAISED ? t.lineraisecolor : t.linelowercolor;
		rightcolor = elev == RAISED ? t.linelowercolor : t.lineraisecolor;
		wattron(win, leftcolor);
		wborder(win, ls, rs, ts, bs, tl, tr, bl, br);
		wattroff(win, leftcolor);

		wattron(win, rightcolor);
		mvwaddch(win, 0, cols-1, tr);
		mvwvline(win, 1, cols-1, rs, rows-2);
		mvwaddch(win, rows-1, cols-1, br);
		mvwhline(win, rows-1, 1, bs, cols-2);
		wattroff(win, rightcolor);

		if (subwindowborders) {
			wattron(win, leftcolor);
			mvwaddch(win, 0, 0, ltee);
			wattroff(win, leftcolor);

			wattron(win, rightcolor);
			mvwaddch(win, 0, cols-1, rtee);
			wattroff(win, rightcolor);
		}
	}

	if (title != NULL) {
		if (t.surroundtitle && elev != NOLINES) {
			wattron(win, leftcolor);
			mvwaddch(win, 0, cols/2 - strlen(title)/2 - 1, rtee);
			wattroff(win, leftcolor);
		}
		wattron(win, t.titlecolor);
		mvwaddstr(win, 0, cols/2 - strlen(title)/2, title);
		wattroff(win, t.titlecolor);
		if (t.surroundtitle && elev != NOLINES) {
			wattron(win, leftcolor);
			waddch(win, ltee);
			wattroff(win, leftcolor);
		}
	}

	if (bottomtitle != NULL) {
		wattron(win, t.bottomtitlecolor);
		wmove(win, rows - 1, cols/2 - strlen(bottomtitle)/2 - 1);
		waddch(win, '[');
		waddstr(win, bottomtitle);
		waddch(win, ']');
		wattroff(win, t.bottomtitlecolor);
	}

	return win;
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

void draw_buttons(WINDOW *window, int cols, struct buttons bs, bool shortkey)
{
	int i, x, start_x, size;
#define SIZEBUTTON  8
#define BUTTONSPACE 3

	size = MAX(SIZEBUTTON - 2, strlen(bs.label[0]));
	for (i=1; i < bs.nbuttons; i++)
		size = MAX(size, strlen(bs.label[i]));
	size += 2;

	start_x = size * bs.nbuttons + (bs.nbuttons - 1) * BUTTONSPACE;
	start_x = cols/2 - start_x/2;

	for (i = 0; i < bs.nbuttons; i++) {
		x = i * (size + BUTTONSPACE);
		draw_button(window, start_x + x, size, bs.label[i],
		    i == bs.curr, shortkey);
	}
}

void
get_buttons(struct buttons *bs, bool yesok, char *yesoklabel, bool extra,
    char *extralabel, bool nocancel, char *nocancellabel, bool defaultno,
    bool help, char *helplabel)
{

	bs->nbuttons = 0;
	bs->curr = 0;

	if (yesok) {
		bs->label[0] = yesoklabel;
		bs->value[0] = BSDDIALOG_YESOK;
		bs->nbuttons = bs->nbuttons + 1;
	}

	if (extra) {
		bs->label[bs->nbuttons] = extralabel;
		bs->value[bs->nbuttons] = BSDDIALOG_EXTRA;
		bs->nbuttons = bs->nbuttons + 1;
	}

	if (nocancel) {
		bs->label[bs->nbuttons] = nocancellabel;
		bs->value[bs->nbuttons] = BSDDIALOG_NOCANCEL;
		if (defaultno)
			bs->curr = bs->nbuttons;
		bs->nbuttons = bs->nbuttons + 1;
	}

	if (help) {
		bs->label[bs->nbuttons] = helplabel;
		bs->value[bs->nbuttons] = BSDDIALOG_HELP;
		bs->nbuttons = bs->nbuttons + 1;
	}

	if (bs->nbuttons == 0) {
		bs->label[0] = yesoklabel;
		bs->value[0] = BSDDIALOG_YESOK;
		bs->nbuttons = 1;
	}
}

int
widget_init(struct config conf, WINDOW **widget, int *y, int *x, char *text,
    int *h, int *w, WINDOW **shadow, bool buttons, WINDOW **buttonswin)
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

	*widget = new_window(*y, *x, *h, *w, conf.title,
	    buttons ? NULL : conf.hline, conf.no_lines ? NOLINES : RAISED,
	    conf.ascii_lines, false);
	if(*widget == NULL) {
		delwin(*shadow);
		return -1;
	}

	if (text != NULL) /* programbox etc */
		print_text_multiline(*widget, 1, 2, text, *w - 4);

	wrefresh(*widget);

	if (buttons) {
		*buttonswin = new_window(*y + *h - 3, *x, 3, *w, NULL,
		    conf.hline, conf.no_lines ? NOLINES : RAISED,
		    conf.ascii_lines, true);
		if (*buttonswin == NULL) {
			delwin(*shadow);
			delwin(*widget);
			return -1;
		}
		wrefresh(*buttonswin);
	}

	return 0;
}

void
widget_end(struct config conf, char *name, WINDOW *window, int h, int w,
    WINDOW *shadow, WINDOW *buttonswin)
{

	if (conf.sleep > 0)
		sleep(conf.sleep);

	if (buttonswin != NULL)
		delwin(buttonswin);

	delwin(window);

	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "%s size: (%d, %d)\n", name, h, w);
}