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
#else
#include <curses.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "theme.h"

extern struct bsddialog_theme t;

/* Error buffer */

#define ERRBUFLEN 1024
static char errorbuffer[ERRBUFLEN];

const char *get_error_string(void)
{
	return errorbuffer;
}

void set_error_string(char *str)
{

	strncpy(errorbuffer, str, ERRBUFLEN-1);
}

// old text, to delete in the future

/* Text */
enum token { TEXT, WS, END };

static bool check_set_ncurses_attr(WINDOW *win, char *text)
{
	bool isattr;
	int colors[8] = {
	    COLOR_BLACK,
	    COLOR_RED,
	    COLOR_GREEN,
	    COLOR_YELLOW,
	    COLOR_BLUE,
	    COLOR_MAGENTA,
	    COLOR_CYAN,
	    COLOR_WHITE
	};

	if (text[0] == '\0' || text[0] != '\\')
		return false;
	if (text[1] == '\0' || text[1] != 'Z')
		return false;
	if (text[2] == '\0')
		return false;

	if ((text[2] - 48) >= 0 && (text[2] - 48) < 8) {
		// tocheck: import BSD_COLOR
		// tofix color background
		wattron(win, COLOR_PAIR(colors[text[2] - 48] * 8 + COLOR_WHITE + 1));
		return true;
	}

	isattr = true;
	switch (text[2]) {
	case 'n':
		wattrset(win, A_NORMAL);
		break;
	case 'b':
		wattron(win, A_BOLD);
		break;
	case 'B':
		wattroff(win, A_BOLD);
		break;
	case 'r':
		wattron(win, A_REVERSE);
		break;
	case 'R':
		wattroff(win, A_REVERSE);
		break;
	case 'u':
		wattron(win, A_UNDERLINE);
		break;
	case 'U':
		wattroff(win, A_UNDERLINE);
		break;
	default:
		isattr = false;
	}

	return isattr;
}

static bool isws(int ch)
{

	return (ch == ' ' || ch == '\t' || ch == '\n');
}

static int
next_token(struct bsddialog_conf conf, char *text, char *valuestr, int *valueint)
{
	int i, j;
	enum token tok;

	i = j = 0;

	if (text[0] == '\0')
		return END;

	while (text[i] != '\0') {
		if (isws(text[i])) {
			if (i == 0) {
				valuestr[0] = text[i];
				valuestr[1] = '\0';
				tok = WS;
			}
			break;
		}

		valuestr[j] = text[i];
		j++;
		valuestr[j] = '\0';
		i++;
		tok = TEXT;
	}

	return tok;
}

static void
print_string(WINDOW *win, int *y, int *x, int minx, int maxx, char *str, bool color)
{
	int i, j, len, reallen;

	if(strlen(str) == 0)
		return;

	len = reallen = strlen(str);
	if (color) {
		i=0;
		while (i < len) {
			if (check_set_ncurses_attr(win, str+i))
				reallen -= 3;
			i++;
		}
	}

	i = 0;
	while (i < len) {
		if (*x + reallen > maxx) {
			*y = (*x != minx ? *y+1 : *y);
			*x = minx;
		}
		j = *x;
		while (j < maxx && i < len) {
			if (color && check_set_ncurses_attr(win, str+i)) {
				i += 3;
			} else {
				mvwaddch(win, *y, j, str[i]);
				i++;
				reallen--;
				j++;
				*x = j;
			}
		}
	}
}

void
print_text(struct bsddialog_conf conf, WINDOW *pad, int starty, int minx, int maxx,
    char *text)
{
	char *valuestr;
	int valueint, x, y;
	bool loop;
	enum token tok;

	valuestr = malloc(strlen(text) + 1);

	x = minx;
	y = starty;
	loop = true;
	while (loop) {
		tok = next_token(conf, text, valuestr, &valueint);
		switch (tok) {
		case END:
			loop = false;
			break;
		case WS:
			text += strlen(valuestr);
			print_string(pad, &y, &x, minx, maxx, valuestr, false /*useless*/);
			break;
		case TEXT:
			text += strlen(valuestr);
			print_string(pad, &y, &x, minx, maxx, valuestr, conf.colors);
			break;
		}
	}

	free(valuestr);
}

// new text funcs

static void prepare_text(struct bsddialog_conf conf, char *text, char *buf)
{
	int i, j;

	i = j = 0;
	while (text[i] != '\0') {
		switch (text[i]) {
		case '\\':
			buf[j] = '\\';
			switch (text[i+1]) {
			case '\\':
				i++;
				break;
			case 'n':
				if (conf.no_nl_expand) {
					j++;
					buf[j] = 'n';
				} else
					buf[j] = '\n';
				i++;
				break;
			case 't':
				if (conf.no_collapse) {
					j++;
					buf[j] = 't';
				} else
					buf[j] = '\t';
				i++;
				break;
			}
			break;
		case '\n':
			buf[j] = conf.cr_wrap ? ' ' : '\n';
			break;
		case '\t':
			buf[j] = conf.no_collapse ? '\t' : ' ';
			break;
		default:
			buf[j] = text[i];
		}
		i++;
		j = (conf.trim && buf[j] == ' ') ? j : j+1;
	}
	buf[j] = '\0';
}

static bool is_ncurses_attr(char *text)
{
	bool isattr;

	if (text[0] == '\0' || text[0] != '\\')
		return false;
	if (text[1] == '\0' || text[1] != 'Z')
		return false;
	if (text[2] == '\0')
		return false;

	if ((text[2] - 48) >= 0 && (text[2] - 48) < 8) {
		return true;
	}

	isattr = text[2] == 'n' || text[2] == 'b' || text[2] == 'B' ||
	    text[2] == 'r' || text[2] == 'R' || text[2] == 'u' || text[2] == 'U';

	return isattr;
}

unsigned int maxword(struct bsddialog_conf conf, char *text)
{
	char *buf;
	unsigned int max;
	int i, len, buflen;

	if ((buf = malloc(strlen(text) + 1)) == NULL)
		return 0;

	prepare_text(conf, text, buf);

	buflen = strlen(buf) + 1;
	max = len = 0;
	for (i=0; i < buflen; i++) {
		if (buf[i] == '\t' || buf[i] == '\n' || buf[i] == ' ' || buf[i] == '\0')
			if (len != 0) {
				// to check \Zx?
				max = MAX(max, len);
				len = 0;
				continue;
			}
		if (conf.colors && is_ncurses_attr(buf + i))
			i+=3;
		else
			len++;
	}

	free(buf);

	return max;
}

static void
print_str(WINDOW *win, int *rows, int *y, int *x, int cols, char *str, bool color)
{
	int i, j, len, reallen;

	if(strlen(str) == 0)
		return;

	len = reallen = strlen(str);
	if (color) {
		i=0;
		while (i < len) {
			if (is_ncurses_attr(str+i))
				reallen -= 3;
			i++;
		}
	}

	i = 0;
	while (i < len) {
		if (*x + reallen > cols) {
			*y = (*x != 0 ? *y+1 : *y);
			if (*y >= *rows) {
				*rows = *y + 1;
				wresize(win, *rows, cols);
			}
			*x = 0;
		}
		j = *x;
		while (j < cols && i < len) {
			if (color && check_set_ncurses_attr(win, str+i)) {
				i += 3;
			} else {
				mvwaddch(win, *y, j, str[i]);
				i++;
				reallen--;
				j++;
				*x = j;
			}
		}
	}
}

static void
print_textpad(struct bsddialog_conf conf, WINDOW *pad, int *rows, int cols, char *text)
{
	char *buf, *string;
	int i, j, x, y;
	bool loop;

	if ((buf = malloc(strlen(text) + 1)) == NULL) {
		delwin(pad);
		return;
	}
	prepare_text(conf, text, buf);

	if ((string = malloc(strlen(text) + 1)) == NULL) {
		delwin(pad);
		free(buf);
		return;
	}
	i = j = x = y = 0;
	loop = true;
	while (loop) {
		string[j] = buf[i];
		if (string[j] == '\0' || string[j] == '\n' ||
		    string[j] == '\t' || string[j] == ' ') {
			if (j != 0) {
				string[j] = '\0';
				print_str(pad, rows, &y, &x, cols, string, conf.colors);
			}
		}

		switch (buf[i]) {
		case '\0':
			loop = false;
			break;
		case '\n':
			j = -1;
			x = 0;
			y++;
			break;
		case '\t':
			for (j=0; j<4 /*tablen*/; j++) {
				x++;
				if (x >= cols) {
					x = 0;
					y++;
				}
			}
			j = -1;
			break;
		case ' ':
			x++;
			if (x >= cols) {
				x = 0;
				y++;
			}
			j = -1;
		}

		if (y >= *rows) { /* check for whitespaces */
			*rows = y + 1;
			wresize(pad, *rows, cols);
		}

		j++;
		i++;
	}

	free(string);
	free(buf);
}

/* Buttons */
void
draw_button(WINDOW *window, int y, int x, int size, char *text, bool selected,
    bool shortkey)
{
	int i, color_arrows, color_shortkey, color_button;

	if (selected) {
		color_arrows = t.currbuttdelimcolor;
		color_shortkey = t.currshortkeycolor;
		color_button = t.currbuttoncolor;
	} else {
		color_arrows = t.buttdelimcolor;
		color_shortkey = t.shortkeycolor;
		color_button = t.buttoncolor;
	}

	wattron(window, color_arrows);
	mvwaddch(window, y, x, t.buttleftch);
	wattroff(window, color_arrows);
	wattron(window, color_button);
	for(i = 1; i < size - 1; i++)
		waddch(window, ' ');
	wattroff(window, color_button);
	wattron(window, color_arrows);
	mvwaddch(window, y, x + i, t.buttrightchar);
	wattroff(window, color_arrows);

	x = x + 1 + ((size - 2 - strlen(text))/2);
	wattron(window, color_button);
	mvwaddstr(window, y, x, text);
	wattroff(window, color_button);

	if (shortkey) {
		wattron(window, color_shortkey);
		mvwaddch(window, y, x, text[0]);
		wattroff(window, color_shortkey);
	}
}

void
draw_buttons(WINDOW *window, int y, int cols, struct buttons bs, bool shortkey)
{
	int i, x, start_x;

	start_x = bs.sizebutton * bs.nbuttons + (bs.nbuttons - 1) * t.buttonspace;
	start_x = cols/2 - start_x/2;

	for (i = 0; i < bs.nbuttons; i++) {
		x = i * (bs.sizebutton + t.buttonspace);
		draw_button(window, y, start_x + x, bs.sizebutton, bs.label[i],
		    i == bs.curr, shortkey);
	}
}

void
get_buttons(struct buttons *bs, bool yesok, char *yesoklabel, bool extra,
    char *extralabel, bool nocancel, char *nocancellabel, bool defaultno,
    bool help, char *helplabel)
{
	int i;
#define SIZEBUTTON  8

	bs->nbuttons = 0;
	bs->curr = 0;
	bs->sizebutton = 0;

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

	bs->sizebutton = MAX(SIZEBUTTON - 2, strlen(bs->label[0]));
	for (i=1; i < bs->nbuttons; i++)
		bs->sizebutton = MAX(bs->sizebutton, strlen(bs->label[i]));
	bs->sizebutton += 2;
}

/* Widgets builders */
WINDOW *
new_window(int y, int x, int rows, int cols, char *title, char *bottomtitle,
    enum elevation elev, bool asciilines)
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

int
widget_withtextpad_init(struct bsddialog_conf conf, WINDOW **shadow, WINDOW **widget,
    int y, int x, int h, int w, WINDOW **textpad, int *htextpad, char *text,
    bool buttons)
{
	int ts, ltee, rtee;

	if (conf.shadow) {
		if ((*shadow = newwin(h, w+1, y+1, x+1)) == NULL)
			RETURN_ERROR("Cannot build shadow");
		wbkgd(*shadow, t.shadowcolor);
		wrefresh(*shadow);
	}

	*widget = new_window(y, x, h, w, conf.title, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines);
	if(*widget == NULL) {
		if (conf.shadow)
			delwin(*shadow);
		RETURN_ERROR("Cannot build widget window");
	}

	if (textpad == NULL && text != NULL) /* no pad */
		print_text(conf, *widget, 1, 2, w-3, text);

	if (buttons && conf.no_lines != true) {
		ts = conf.ascii_lines ? '-' : ACS_HLINE;
		ltee = conf.ascii_lines ? '+' : ACS_LTEE;
		rtee = conf.ascii_lines ? '+' : ACS_RTEE;

		wattron(*widget, t.lineraisecolor);
		mvwaddch(*widget, h-3, 0, ltee);
		mvwhline(*widget, h-3, 1, ts, w-2);
		wattroff(*widget, t.lineraisecolor);

		wattron(*widget, t.linelowercolor);
		mvwaddch(*widget, h-3, w-1, rtee);
		wattroff(*widget, t.linelowercolor);
	}

	wrefresh(*widget);

	if (textpad == NULL)
		return 0; /* widget_init() ends */

	if (text != NULL) { /* programbox etc */
		if ((*textpad = newpad(*htextpad, w-4)) == NULL) {
			if (conf.shadow)
				delwin(*shadow);
			delwin(*textpad);
			RETURN_ERROR("Cannot build text");
		}
		wbkgd(*textpad, t.widgetcolor);
		print_textpad(conf, *textpad, htextpad, w-4, text);
	}

	return 0;
}

int
widget_init(struct bsddialog_conf conf, WINDOW **widget, int *y, int *x, char *text,
    int *h, int *w, WINDOW **shadow, bool buttons)
{
	int output;

	// to delete (each widget has to check its x,y,h,w)
	if (*h <= 0)
		; /* todo */

	if (*w <= 0)
		; /* todo */

	*y = (conf.y < 0) ? (LINES/2 - *h/2) : conf.y;
	*x = (conf.x < 0) ? (COLS/2 - *w/2) : conf.x;

	output = widget_withtextpad_init(conf, shadow, widget, *y, *x, *h, *w,
	    NULL, NULL, text, buttons);

	return output;
}

void
widget_withtextpad_end(struct bsddialog_conf conf, char *name, WINDOW *window, int h,
    int w, WINDOW *textpad, WINDOW *shadow)
{

	if (conf.sleep > 0)
		sleep(conf.sleep);

	if (textpad != NULL)
		delwin(textpad);

	delwin(window);

	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "%s size: (%d, %d)\n", name, h, w);
}

void
widget_end(struct bsddialog_conf conf, char *name, WINDOW *window, int h, int w,
    WINDOW *shadow)
{

	widget_withtextpad_end(conf, name, window, h, w, NULL, shadow);
}
