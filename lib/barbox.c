/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021-2023 Alfonso Sabato Siciliano
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

#include <ctype.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bsddialog.h"
#include "bsddialog_progressview.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define BARPADDING     2
#define MINBARLEN      15
#define MINBARWIDTH    (2 + 2 * BARPADDING + MINBARLEN)
#define MINMGBARLEN    18
#define MINMGBARWIDTH  (2 + 2 * BARPADDING + MINMGBARLEN)
#define HBAR           3

bool bsddialog_interruptprogview;
bool bsddialog_abortprogview;
int  bsddialog_total_progview;

struct bar {
	bool toupdate;
	WINDOW *win;
	int y;           /* bar y in win */
	int x;           /* bar x in win */
	int w;           /* width in win */
	int perc;        /* barlen = (w * perc) / 100*/
	const char* fmt; /* format for label */
	int label;       /* rangebox and pause perc!=label */
};

static void draw_bar(struct bar *b)
{
	int barlen, xlabel;
	chtype ch;
	char label[128];

	barlen = b->perc > 0 ? (b->perc * b->w) / 100 : 0;

	ch = ' ' | t.bar.f_color;
	mvwhline(b->win, b->y, b->x, ch, barlen);
	ch = ' ' | t.bar.color;
	mvwhline(b->win, b->y, b->x + barlen, ch, b->w - barlen);

	sprintf(label, b->fmt, b->label);
	xlabel = b->x + b->w/2 - (int)strlen(label)/2; /* 1-byte-char string */
	wattron(b->win, t.bar.color);   /* x+barlen < xlabel */
	mvwaddstr(b->win, b->y, xlabel, label);
	wattroff(b->win, t.bar.color);
	wattron(b->win, t.bar.f_color); /* x+barlen >= xlabel */
	mvwaddnstr(b->win, b->y, xlabel, label, MAX((b->x+barlen) - xlabel, 0));
	wattroff(b->win, t.bar.f_color);

	if (b->toupdate)
		wrefresh(b->win);
	b->toupdate = false;
}

int
bsddialog_gauge(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int perc, int fd, const char *sep)
{
	bool mainloop;
	int y, x, h, w, fd2;
	FILE *input;
	WINDOW *widget, *textpad, *shadow;
	struct bar b;
	char inputbuf[2048], ntext[2048], *pntext;

	if (widget_size_position(conf, rows, cols, text, HBAR, MINBARWIDTH,
	    NULL, &y, &x, &h, &w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, NULL) != 0)
		return (BSDDIALOG_ERROR);

	b.y = b.x = 1;
	b.w = w - 8;
	if ((b.win = new_boxed_window(conf, y+h-4, x+3, 3, w-6, RAISED)) == NULL)
		return (BSDDIALOG_ERROR);
	b.fmt = "%3d%%";

	input = NULL;
	if (fd >= 0) {
		fd2 = dup(fd);
		if ((input = fdopen(fd2, "r")) == NULL)
			RETURN_FMTERROR("Cannot build FILE* from fd %d", fd);
	}

	mainloop = true;
	while (mainloop) {
		wrefresh(widget);
		prefresh(textpad, 0, 0, y+1, x+1+TEXTHMARGIN, y+h-4,
		    x+w-1-TEXTHMARGIN);
		draw_borders(conf, b.win, 3, w-6, RAISED);
		b.perc = b.label = perc;
		b.toupdate = true;
		draw_bar(&b);
		if (input == NULL) /* that is fd < 0 */
			break;

		while (true) {
			fscanf(input, "%s", inputbuf);
			if (strcmp(inputbuf,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(inputbuf, sep) == 0)
				break;
		}
		if (mainloop == false)
			break;
		fscanf(input, "%d", &perc);
		perc = perc > 100 ? 100 : perc;
		pntext = &ntext[0];
		ntext[0] = '\0';
		while (true) {
			fscanf(input, "%s", inputbuf);
			if (strcmp(inputbuf,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(inputbuf, sep) == 0)
				break;
			strcpy(pntext, inputbuf);
			pntext += strlen(inputbuf); /* end string, no strlen */
			pntext[0] = ' ';
			pntext++;
		}
		pntext[0] = '\0';
		if (update_dialog(conf, shadow, widget, y, x, h, w, textpad,
		    ntext, NULL) != 0)
			return (BSDDIALOG_ERROR);
	}

	if (input != NULL)
		fclose(input);
	delwin(b.win);
	end_dialog(conf, shadow, widget, textpad);

	return (BSDDIALOG_OK);
}

/* Mixedgauge */
static int
do_mixedgauge(struct bsddialog_conf *conf, const char *text, int rows, int cols,
    unsigned int mainperc, unsigned int nminibars, const char **minilabels,
    int *minipercs, bool color)
{
	int i, miniperc, y, x, h, w, max_minbarlen;
	int ytext, htext;
	int minicolor, red, green;
	WINDOW *widget, *textpad, *shadow;
	struct bar b;
	char states[12][14] = {
		"  Succeeded  ", /* -1  */
		"   Failed    ", /* -2  */
		"   Passed    ", /* -3  */
		"  Completed  ", /* -4  */
		"   Checked   ", /* -5  */
		"    Done     ", /* -6  */
		"   Skipped   ", /* -7  */
		" In Progress ", /* -8  */
		"(blank)      ", /* -9  */
		"     N/A     ", /* -10 */
		"   Pending   ", /* -11 */
		"   UNKNOWN   ", /* < -11, no API */
	};

	red   = bsddialog_color(BSDDIALOG_WHITE,BSDDIALOG_RED,  BSDDIALOG_BOLD);
	green = bsddialog_color(BSDDIALOG_WHITE,BSDDIALOG_GREEN,BSDDIALOG_BOLD);

	max_minbarlen = 0;
	for (i = 0; i < (int)nminibars; i++)
		max_minbarlen = MAX(max_minbarlen, (int)strcols(minilabels[i]));
	max_minbarlen += 3 + 16; /* seps + [...] */
	max_minbarlen = MAX(max_minbarlen, MINMGBARWIDTH); /* mainbar */

	if (widget_size_position(conf, rows, cols, text, nminibars + HBAR,
	    max_minbarlen, NULL, &y, &x, &h, &w, &htext) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, NULL) != 0)
		return (BSDDIALOG_ERROR);

	/* mini bars */
	b.win = widget;
	b.x = 1 + w - 2 - 15;
	b.w = 13;
	b.fmt = "%3d%%";
	b.toupdate = false;
	for (i = 0; i < (int)nminibars; i++) {
		miniperc = minipercs[i];
		/* label */
		if (color && miniperc >= 0)
			wattron(widget, A_BOLD);
		mvwaddstr(widget, i+1, 2, minilabels[i]);
		if (color && miniperc >= 0)
			wattroff(widget, A_BOLD);
		/* perc */
		if (miniperc == BSDDIALOG_MG_BLANK)
			continue;
		mvwaddstr(widget, i+1, w-2-15, "[             ]");
		if (miniperc >= 0) {
			b.y = i + 1;
			b.perc = b.label = MIN(miniperc, 100);
			draw_bar(&b);
		} else { /* miniperc < 0 */
			if (miniperc < BSDDIALOG_MG_PENDING)
				miniperc = -12; /* UNKNOWN */
			minicolor = t.dialog.color;
			if (color && miniperc == BSDDIALOG_MG_FAILED)
				minicolor = red;
			else if (color && miniperc == BSDDIALOG_MG_DONE)
				minicolor = green;
			wattron(widget, minicolor);
			miniperc = abs(miniperc + 1);
			mvwaddstr(widget, i+1, 1+w-2-15, states[miniperc]);
			wattroff(widget, minicolor);
		}
	}
	wrefresh(widget);

	/* text */
	ytext = MAX(y + h - BORDER - HBAR - htext, y + BORDER + (int)nminibars);
	prefresh(textpad, 0, 0, ytext, x+2, y+h-5, x+w-2);

	/* main bar */
	if ((b.win = new_boxed_window(conf, y+h -4, x+3, 3, w-6, RAISED)) == NULL)
		return (BSDDIALOG_ERROR);
	wattron(b.win, t.bar.color);
	mvwaddstr(b.win, 0, 2, "Overall Progress");
	wattroff(b.win, t.bar.color);

	b.y = b.x = 1;
	b.w = w - 8;
	b.fmt = "%3d%%";
	b.perc = b.label = MIN(mainperc, 100);
	b.toupdate = true;
	draw_bar(&b);
	/* getch(); "alternate mode" shows nothing (ex. port devel/ncurses) */

	delwin(b.win);
	end_dialog(conf, shadow, widget, textpad);

	return (BSDDIALOG_OK);
}

int
bsddialog_mixedgauge(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int mainperc, unsigned int nminibars,
    const char **minilabels, int *minipercs)
{
	int retval;

	retval = do_mixedgauge(conf, text, rows, cols, mainperc, nminibars,
	    minilabels, minipercs, false);

	return (retval);
}

int
bsddialog_progressview (struct bsddialog_conf *conf, const char *text, int rows,
    int cols, struct bsddialog_progviewconf *pvconf, unsigned int nminibar,
    struct bsddialog_fileminibar *minibar)
{
	bool update;
	int perc, retval, *minipercs;
	unsigned int i, mainperc, totaltodo;
	float readforsec;
	const char **minilabels;
	time_t tstart, told, tnew, refresh;

	if ((minilabels = calloc(nminibar, sizeof(char*))) == NULL)
		RETURN_ERROR("Cannot allocate memory for minilabels");
	if ((minipercs = calloc(nminibar, sizeof(int))) == NULL)
		RETURN_ERROR("Cannot allocate memory for minipercs");

	totaltodo = 0;
	for (i = 0; i < nminibar; i++) {
		totaltodo += minibar[i].size;
		minilabels[i] = minibar[i].label;
		minipercs[i] = minibar[i].status;
	}

	refresh = pvconf->refresh == 0 ? 0 : pvconf->refresh - 1;
	retval = BSDDIALOG_OK;
	i = 0;
	update = true;
	time(&told);
	tstart = told;
	while (!(bsddialog_interruptprogview || bsddialog_abortprogview)) {
		if (bsddialog_total_progview == 0 || totaltodo == 0)
			mainperc = 0;
		else
			mainperc = (bsddialog_total_progview * 100) / totaltodo;

		time(&tnew);
		if (update || tnew > told + refresh) {
			retval = do_mixedgauge(conf, text, rows, cols, mainperc,
			    nminibar, minilabels, minipercs, true);
			if (retval == BSDDIALOG_ERROR)
				return (BSDDIALOG_ERROR);

			move(SCREENLINES - 1, 2);
			clrtoeol();
			readforsec = ((tnew - tstart) == 0) ? 0 :
			    bsddialog_total_progview / (float)(tnew - tstart);
			printw(pvconf->fmtbottomstr, bsddialog_total_progview,
			    readforsec);
			refresh();

			time(&told);
			update = false;
		}

		if (i >= nminibar)
			break;
		if (minibar[i].status == BSDDIALOG_MG_FAILED)
			break;

		perc = pvconf->callback(&minibar[i]);

		if (minibar[i].status == BSDDIALOG_MG_DONE) { /*||perc >= 100)*/
			minipercs[i] = BSDDIALOG_MG_DONE;
			update = true;
			i++;
		} else if (minibar[i].status == BSDDIALOG_MG_FAILED || perc < 0) {
			minipercs[i] = BSDDIALOG_MG_FAILED;
			update = true;
		} else /* perc >= 0 */
			minipercs[i] = perc;
	}

	free(minilabels);
	free(minipercs);
	return (retval);
}

static int
rangebox_redraw(struct bsddialog_conf *conf, WINDOW *shadow, WINDOW* widget,
    WINDOW *textpad, const char *text, int rows, int cols, int *y, int *x,
    int *h, int *w, struct bar *b,int *bigchange, struct buttons *bs)
{
	/* Important for decreasing screen */
	hide_dialog(*y, *x, *h, *w, conf->shadow);
	refresh();

	if (widget_size_position(conf, rows, cols, text, HBAR, MINBARWIDTH, bs,
	    y, x, h, w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (update_dialog(conf, shadow, widget, *y, *x, *h, *w, textpad, text, bs) != 0)
		return (BSDDIALOG_ERROR);
	pnoutrefresh(textpad, 0, 0, *y+1, *x+1+TEXTHMARGIN, *y+*h-7, *x+*w-1-TEXTHMARGIN);
	doupdate();

	wclear(b->win);
	b->w = *w - BORDERS - (2 * BARPADDING) - 2;
	*bigchange = MAX(1, b->w  / 10);
	wresize(b->win, 3, b->w + 2);
	mvwin(b->win, *y + *h - 6, *x + 1 + BARPADDING);
	draw_borders(conf, b->win, 3, b->w+2, RAISED);
	b->toupdate = true;

	return (0);
}

int
bsddialog_rangebox(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, int min, int max, int *value)
{
	bool loop;
	int y, x, h, w;
	int currvalue, retval, bigchange, positions;
	wint_t input;
	WINDOW *widget, *textpad, *shadow;
	struct bar b;
	struct buttons bs;

	CHECK_PTR(value, int);
	if (min >= max)
		RETURN_FMTERROR("min (%d) >= max (%d)", min, max);
	if (*value < min)
		RETURN_FMTERROR("value (%d) < min (%d)", *value, min);
	if (*value > max)
		RETURN_FMTERROR("value (%d) > max (%d)", *value, max);

	currvalue = *value;
	positions = max - min + 1;

	get_buttons(conf, &bs, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if (widget_size_position(conf, rows, cols, text, HBAR, MINBARWIDTH, &bs,
	    &y, &x, &h, &w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);
	pnoutrefresh(textpad, 0, 0, y+1, x+1+TEXTHMARGIN, y+h-7, x+w-1-TEXTHMARGIN);
	doupdate();

	b.y = b.x = 1;
	b.w = w - BORDERS - (2 * BARPADDING) - 2;
	bigchange = MAX(1, b.w / 10);
	if ((b.win = new_boxed_window(conf, y + h - 6, x + 1 + BARPADDING, 3,
	    b.w + 2, RAISED)) == NULL)
		return (BSDDIALOG_ERROR);
	b.fmt = "%d";
	b.toupdate = true;

	loop = true;
	while (loop) {
		if (b.toupdate) {
			b.perc = ((float)(currvalue - min)*100) / (positions-1);
			b.label = currvalue;
			draw_bar(&b);
		}

		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			retval = BUTTONVALUE(bs);
			*value = currvalue;
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			DRAW_REFRESH_BUTTONS(widget, bs);
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				DRAW_REFRESH_BUTTONS(widget, bs);
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				DRAW_REFRESH_BUTTONS(widget, bs);
			}
			break;
		case KEY_HOME:
			currvalue = max;
			b.toupdate = true;
			break;
		case KEY_END:
			currvalue = min;
			b.toupdate = true;
			break;
		case KEY_NPAGE:
			currvalue -= bigchange;
			if (currvalue < min)
				currvalue = min;
			b.toupdate = true;
			break;
		case KEY_PPAGE:
			currvalue += bigchange;
			if (currvalue > max)
				currvalue = max;
			b.toupdate = true;
			break;
		case KEY_UP:
			if (currvalue < max) {
				currvalue++;
				b.toupdate = true;
			}
			break;
		case KEY_DOWN:
			if (currvalue > min) {
				currvalue--;
				b.toupdate = true;
			}
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help_dialog(conf) != 0)
				return (BSDDIALOG_ERROR);
			if (rangebox_redraw(conf, shadow, widget, textpad, text,
			    rows, cols, &y, &x, &h, &w, &b, &bigchange, &bs) != 0)
				return (BSDDIALOG_ERROR);
			break;
		case KEY_RESIZE:
			if (rangebox_redraw(conf, shadow, widget, textpad, text,
			    rows, cols, &y, &x, &h, &w, &b, &bigchange, &bs) != 0)
				return (BSDDIALOG_ERROR);
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				retval = BUTTONVALUE(bs);
				loop = false;
			}
		}
	}

	delwin(b.win);
	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}

static int
pause_redraw(struct bsddialog_conf *conf, WINDOW *shadow, WINDOW* widget,
    WINDOW *textpad, const char *text, int rows, int cols, int *y, int *x,
    int *h, int *w, struct bar *b, struct buttons *bs)
{
	/* Important for decreasing screen */
	hide_dialog(*y, *x, *h, *w, conf->shadow);
	refresh();

	if (widget_size_position(conf, rows, cols, text, HBAR, MINBARWIDTH, bs,
	    y, x, h, w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (update_dialog(conf, shadow, widget, *y, *x, *h, *w, textpad, text, bs) != 0)
		return (BSDDIALOG_ERROR);
	pnoutrefresh(textpad, 0, 0, *y+1, *x+1+TEXTHMARGIN, *y+*h-7, *x+*w-1-TEXTHMARGIN);
	doupdate();

	wclear(b->win);
	b->w = *w - BORDERS - (2 * BARPADDING) - 2;
	wresize(b->win, 3, b->w + 2);
	mvwin(b->win, *y + *h - 6, *x + 1 + BARPADDING);
	draw_borders(conf, b->win, 3, b->w+2, RAISED);
	b->toupdate = true;

	return (0);
}

int
bsddialog_pause(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int sec)
{
	bool loop;
	int retval, y, x, h, w, tout;
	wint_t input;
	WINDOW *widget, *textpad, *shadow;
	struct bar b;
	struct buttons bs;

	get_buttons(conf, &bs, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if (widget_size_position(conf, rows, cols, text, HBAR, MINBARWIDTH, &bs,
	    &y, &x, &h, &w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);
	pnoutrefresh(textpad, 0, 0, y+1, x+1+TEXTHMARGIN, y+h-7, x+w-1-TEXTHMARGIN);
	doupdate();

	b.y = b.x = 1;
	b.w = w - BORDERS - (2 * BARPADDING) - 2;
	if ((b.win = new_boxed_window(conf, y + h - 6, x + 1 + BARPADDING, 3,
	    b.w + 2, RAISED)) == NULL)
		return (BSDDIALOG_ERROR);
	b.fmt = "%d";
	b.toupdate = true;

	tout = sec;
	nodelay(stdscr, TRUE);
	timeout(1000);
	loop = true;
	while (loop) {
		if (b.toupdate) {
			b.perc = (float)tout * 100 / sec;
			b.label = tout;
			draw_bar(&b);
		}

		if (get_wch(&input) == ERR) { /* timeout */
			tout--;
			if (tout < 0) {
				retval = BSDDIALOG_TIMEOUT;
				break;
			}
			else {
				b.toupdate = true;
				continue;
			}
		}
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			retval = BUTTONVALUE(bs);
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			DRAW_REFRESH_BUTTONS(widget, bs);
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				DRAW_REFRESH_BUTTONS(widget, bs);
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				DRAW_REFRESH_BUTTONS(widget, bs);
			}
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help_dialog(conf) != 0)
				return (BSDDIALOG_ERROR);
			if (pause_redraw(conf, shadow, widget, textpad, text,
			    rows, cols, &y, &x, &h, &w, &b, &bs) != 0)
				return (BSDDIALOG_ERROR);
			break;
		case KEY_RESIZE:
			if (pause_redraw(conf, shadow, widget, textpad, text,
			    rows, cols, &y, &x, &h, &w, &b, &bs) != 0)
				return (BSDDIALOG_ERROR);
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				retval = BUTTONVALUE(bs);
				loop = false;
			}
		}
	}
	nodelay(stdscr, FALSE);

	delwin(b.win);
	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}
