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
long long int bsddialog_total_progview;

static const char states[12][14] = {
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

struct bar {
	bool toupdate;
	WINDOW *win;
	int y;           /* bar y in win */
	int x;           /* bar x in win */
	int w;           /* width in win */
	int perc;        /* barlen = (w * perc) / 100 */
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
		wnoutrefresh(b->win);
	b->toupdate = false;
}

int
bsddialog_gauge(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int perc, int fd, const char *sep, const char *end)
{
	bool mainloop;
	int fd2;
	FILE *input;
	char inputbuf[2048], ntext[2048], *pntext;
	struct bar b;
	struct dialog d;

	if (prepare_dialog(conf, text, rows, cols, &d) != 0)
		return (BSDDIALOG_ERROR);
	if ((b.win = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW bar");
	b.y = b.x = 1;
	b.fmt = "%3d%%";

	input = NULL;
	if (fd >= 0) {
		CHECK_PTR(sep, const char);
		CHECK_ARRAY(1, sep, const char);
		CHECK_PTR(end, const char);
		CHECK_ARRAY(1, end, const char);

		fd2 = dup(fd);
		if ((input = fdopen(fd2, "r")) == NULL)
			RETURN_FMTERROR("Cannot build FILE* from fd %d", fd);
	}

	perc = MIN(perc, 100);
	mainloop = true;
	while (mainloop) {
		if (d.built) {
			hide_dialog(&d);
			refresh(); /* Important for decreasing screen */
		}
		if (dialog_size_position(&d, HBAR, MINBARWIDTH, NULL) != 0)
			return (BSDDIALOG_ERROR);
		if (draw_dialog(&d))
			return (BSDDIALOG_ERROR);
		if (d.built)
			refresh(); /* Important to fix grey lines expanding screen */
		TEXTPAD(&d, HBAR);
		update_box(conf, b.win, d.y+d.h-4, d.x+3, HBAR, d.w-6, RAISED);
		b.w = d.w - 8;
		b.perc = b.label = perc;
		b.toupdate = true;
		draw_bar(&b);
		doupdate();
		if (input == NULL) /* that is fd < 0 */
			break;

		while (true) {
			fscanf(input, "%s", inputbuf);
			if (strcmp(inputbuf, end) == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(inputbuf, sep) == 0)
				break;
		}
		if (mainloop == false)
			break;
		fscanf(input, "%d", &perc);
		perc = MIN(perc, 100);
		pntext = &ntext[0];
		ntext[0] = '\0';
		while (true) {
			fscanf(input, "%s", inputbuf);
			if (strcmp(inputbuf, end) == 0) {
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
		d.text = ntext;
	}

	if (input != NULL)
		fclose(input);
	delwin(b.win);
	end_dialog(&d);

	return (BSDDIALOG_OK);
}

/* Mixedgauge */
static int
do_mixedgauge(struct bsddialog_conf *conf, const char *text, int rows, int cols,
    unsigned int mainperc, unsigned int nminibars, const char **minilabels,
    int *minipercs, bool color)
{
	int i, miniperc, max_minbarlen;
	int ystext, htext;
	int minicolor, red, green;
	struct bar b;
	struct dialog d;

	red   = bsddialog_color(BSDDIALOG_WHITE,BSDDIALOG_RED,  BSDDIALOG_BOLD);
	green = bsddialog_color(BSDDIALOG_WHITE,BSDDIALOG_GREEN,BSDDIALOG_BOLD);

	max_minbarlen = 0;
	for (i = 0; i < (int)nminibars; i++)
		max_minbarlen = MAX(max_minbarlen, (int)strcols(minilabels[i]));
	max_minbarlen += 3 + 16; /* seps + [...] */
	max_minbarlen = MAX(max_minbarlen, MINMGBARWIDTH); /* mainbar */

	if (prepare_dialog(conf, text, rows, cols, &d) != 0)
		return (BSDDIALOG_ERROR);
	if (dialog_size_position(&d, nminibars + HBAR, max_minbarlen, &htext) != 0)
		return (BSDDIALOG_ERROR);
	if (draw_dialog(&d) != 0)
		return (BSDDIALOG_ERROR);

	/* mini bars */
	b.win = d.widget;
	b.x = 1 + d.w - 2 - 15;
	b.w = 13;
	b.fmt = "%3d%%";
	b.toupdate = false;
	for (i = 0; i < (int)nminibars; i++) {
		miniperc = minipercs[i];
		/* label */
		if (color && miniperc >= 0)
			wattron(d.widget, A_BOLD);
		mvwaddstr(d.widget, i+1, 2, minilabels[i]);
		if (color && miniperc >= 0)
			wattroff(d.widget, A_BOLD);
		/* perc */
		if (miniperc == BSDDIALOG_MG_BLANK)
			continue;
		mvwaddstr(d.widget, i+1, d.w-2-15, "[             ]");
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
			wattron(d.widget, minicolor);
			miniperc = abs(miniperc + 1);
			mvwaddstr(d.widget, i+1, 1+d.w-2-15, states[miniperc]);
			wattroff(d.widget, minicolor);
		}
	}
	wnoutrefresh(d.widget);

	/* text */
	ystext = MAX(d.h - BORDERS - htext - HBAR, (int)nminibars);
	YSTEXTPAD(&d, ystext, HBAR);

	/* main bar */
	if ((b.win = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW bar");
	update_box(conf, b.win, d.y+d.h - 4, d.x+3, HBAR, d.w-6, RAISED);
	wattron(b.win, t.bar.color);
	mvwaddstr(b.win, 0, 2, "Overall Progress");
	wattroff(b.win, t.bar.color);

	b.y = b.x = 1;
	b.w = d.w - 8;
	b.fmt = "%3d%%";
	b.perc = b.label = MIN(mainperc, 100);
	b.toupdate = true;
	draw_bar(&b);

	doupdate();
	/* getch(); to test with "alternate mode" */

	delwin(b.win);
	end_dialog(&d);

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

static int rangebox_redraw(struct dialog *d, struct bar *b, int *bigchange)
{
	if (d->built) {
		hide_dialog(d);
		refresh(); /* Important for decreasing screen */
	}
	if (dialog_size_position(d, HBAR, MINBARWIDTH, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (draw_dialog(d) != 0)
		return (BSDDIALOG_ERROR);
	if (d->built)
		refresh(); /* Important to fix grey lines expanding screen */
	TEXTPAD(d, HBAR + HBUTTONS);

	b->w = d->w - BORDERS - (2 * BARPADDING) - 2;
	*bigchange = MAX(1, b->w  / 10);
	update_box(d->conf, b->win, d->y + d->h - 6, d->x + 1 + BARPADDING,
	    HBAR, b->w + 2, RAISED);
	b->toupdate = true;

	return (0);
}

int
bsddialog_rangebox(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, int min, int max, int *value)
{
	bool loop;
	int currvalue, retval, bigchange, positions;
	wint_t input;
	struct bar b;
	struct dialog d;

	CHECK_PTR(value, int);
	if (min >= max)
		RETURN_FMTERROR("min (%d) >= max (%d)", min, max);
	if (*value < min)
		RETURN_FMTERROR("value (%d) < min (%d)", *value, min);
	if (*value > max)
		RETURN_FMTERROR("value (%d) > max (%d)", *value, max);

	currvalue = *value;
	positions = max - min + 1;

	if (prepare_dialog(conf, text, rows, cols, &d) != 0)
		return (BSDDIALOG_ERROR);
	set_buttons(&d, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if ((b.win = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW bar");
	b.y = b.x = 1;
	b.fmt = "%d";
	if (rangebox_redraw(&d, &b, &bigchange) != 0)
		return (BSDDIALOG_ERROR);

	loop = true;
	while (loop) {
		if (b.toupdate) {
			b.perc = ((float)(currvalue - min)*100) / (positions-1);
			b.label = currvalue;
			draw_bar(&b);
		}
		doupdate();
		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			retval = BUTTONVALUE(d.bs);
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
			d.bs.curr = (d.bs.curr + 1) % d.bs.nbuttons;
			DRAW_BUTTONS(d);
			break;
		case KEY_LEFT:
			if (d.bs.curr > 0) {
				d.bs.curr--;
				DRAW_BUTTONS(d);
			}
			break;
		case KEY_RIGHT:
			if (d.bs.curr < (int) d.bs.nbuttons - 1) {
				d.bs.curr++;
				DRAW_BUTTONS(d);
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
			if (rangebox_redraw(&d, &b, &bigchange) != 0)
				return (BSDDIALOG_ERROR);
			break;
		case KEY_RESIZE:
			if (rangebox_redraw(&d, &b, &bigchange) != 0)
				return (BSDDIALOG_ERROR);
			break;
		default:
			if (shortcut_buttons(input, &d.bs)) {
				retval = BUTTONVALUE(d.bs);
				loop = false;
			}
		}
	}

	delwin(b.win);
	end_dialog(&d);

	return (retval);
}

static int pause_redraw(struct dialog *d, struct bar *b)
{
	if (d->built) {
		hide_dialog(d);
		refresh(); /* Important for decreasing screen */
	}
	if (dialog_size_position(d, HBAR, MINBARWIDTH, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (draw_dialog(d) != 0)
		return (BSDDIALOG_ERROR);
	if (d->built)
		refresh(); /* Important to fix grey lines expanding screen */
	TEXTPAD(d, HBAR + HBUTTONS);

	b->w = d->w - BORDERS - (2 * BARPADDING) - 2;
	update_box(d->conf, b->win, d->y + d->h - 6, d->x + 1 + BARPADDING,
	    HBAR, b->w + 2, RAISED);
	b->toupdate = true;

	return (0);
}

int
bsddialog_pause(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int sec)
{
	bool loop;
	int retval, tout;
	wint_t input;
	struct bar b;
	struct dialog d;

	if (prepare_dialog(conf, text, rows, cols, &d) != 0)
		return (BSDDIALOG_ERROR);
	set_buttons(&d, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if ((b.win = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW bar");
	b.y = b.x = 1;
	b.fmt = "%d";
	if (pause_redraw(&d, &b) != 0)
		return (BSDDIALOG_ERROR);

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
		doupdate();
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
			retval = BUTTONVALUE(d.bs);
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			d.bs.curr = (d.bs.curr + 1) % d.bs.nbuttons;
			DRAW_BUTTONS(d);
			break;
		case KEY_LEFT:
			if (d.bs.curr > 0) {
				d.bs.curr--;
				DRAW_BUTTONS(d);
			}
			break;
		case KEY_RIGHT:
			if (d.bs.curr < (int) d.bs.nbuttons - 1) {
				d.bs.curr++;
				DRAW_BUTTONS(d);
			}
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help_dialog(conf) != 0)
				return (BSDDIALOG_ERROR);
			if (pause_redraw(&d, &b) != 0)
				return (BSDDIALOG_ERROR);
			break;
		case KEY_RESIZE:
			if (pause_redraw(&d, &b) != 0)
				return (BSDDIALOG_ERROR);
			break;
		default:
			if (shortcut_buttons(input, &d.bs)) {
				retval = BUTTONVALUE(d.bs);
				loop = false;
			}
		}
	}
	nodelay(stdscr, FALSE);

	delwin(b.win);
	end_dialog(&d);

	return (retval);
}
