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

#include <sys/param.h>

#include <ctype.h>
#ifdef PORTNCURSES
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bsddialog.h"
#include "bsddialog_progressview.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define BARMARGIN	3
#define MINBARWIDTH	10
#define MINWIDTH	(MINBARWIDTH + BARMARGIN * 2)
#define MINHEIGHT	7 /* without text */

bool bsddialog_interruptprogview;
bool bsddialog_abortprogview;
int  bsddialog_total_progview;

extern struct bsddialog_theme t;

static void
draw_bar(WINDOW *win, int y, int x, int size, int perc, bool withlabel, 
    int label)
{
	char labelstr[128];
	int i, blue_x, color;

	blue_x = (int)((perc*(size))/100);

	wmove(win, y, x);
	for (i = 0; i < size; i++) {
		color = (i <= blue_x) ? t.bar.f_color : t.bar.color;
		wattron(win, color);
		waddch(win, ' ');
		wattroff(win, color);
	}

	if (withlabel)
		sprintf(labelstr, "%d", label);
	else
		sprintf(labelstr, "%3d%%", perc);
	wmove(win, y, x + size/2 - 2);
	for (i=0; i < (int) strlen(labelstr); i++) {
		color = (blue_x + 1 <= size/2 - (int)strlen(labelstr)/2 + i ) ?
		    t.bar.color : t.bar.f_color;
		wattron(win, color);
		waddch(win, labelstr[i]);
		wattroff(win, color);
	}
}

static int
bar_autosize(struct bsddialog_conf *conf, int rows, int cols, int *h, int *w,
    char *text, struct buttons *bs)
{
	int maxword, maxline, nlines;

	if (get_text_properties(conf, text, &maxword, &maxline, &nlines) != 0)
		return (BSDDIALOG_ERROR);

	if (cols == BSDDIALOG_AUTOSIZE) {
		*w = MAX(MINWIDTH, (int)(maxline + t.text.hmargin * 2));
		*w = widget_min_width(conf, bs, *w);
	}

	if (rows == BSDDIALOG_AUTOSIZE) {
		*h = MINHEIGHT;
		if (maxword > 0)
			*h += 1;

		*h = widget_min_height(conf, bs != NULL, *h);
	}

	return (0);
}

static int
bar_checksize(char *text, int rows, int cols, struct buttons *bs)
{
	int minheight, minwidth;

	minwidth = 0;
	if (bs != NULL) { /* gauge has not buttons */
		minwidth = bs->nbuttons * bs->sizebutton;
		if (bs->nbuttons > 0)
			minwidth += (bs->nbuttons-1) * t.button.space;
	}
	minwidth = MAX(minwidth + VBORDERS, MINBARWIDTH);

	if (cols< minwidth)
		RETURN_ERROR("Few cols for this widget");

	minheight = MINHEIGHT + ((text != NULL && strlen(text) > 0) ? 1 : 0);
	if (rows < minheight)
		RETURN_ERROR("Few rows for this mixedgauge");

	return (0);
}

int
bsddialog_gauge(struct bsddialog_conf *conf, char* text, int rows, int cols,
    unsigned int perc)
{
	bool mainloop;
	int y, x, h, w, htextpad;
	WINDOW *widget, *textpad, *bar, *shadow;
	char input[2048], ntext[2048], *pntext;

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (bar_autosize(conf, rows, cols, &h, &w, text, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (bar_checksize(text, h, w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, NULL,
	    false) != 0)
		return (BSDDIALOG_ERROR);

	bar = new_boxed_window(conf, y+h-4, x+3, 3, w-6, RAISED);

	mainloop = true;
	while (mainloop) {
		wrefresh(widget);
		prefresh(textpad, 0, 0, y+1, x+1+t.text.hmargin, y+h-4,
		    x+w-1-t.text.hmargin);
		draw_bar(bar, 1, 1, w-8, perc, false, -1 /*unused*/);
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
		htextpad = 1;
		wclear(textpad);
		pntext = &ntext[0];
		ntext[0] = '\0';
		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
			pntext[0] = ' ';
			pntext++;
			strcpy(pntext, input);
			pntext += strlen(input);
		}
		print_textpad(conf, textpad, &htextpad, w-2-t.text.hmargin*2,
		    ntext);
	}

	delwin(bar);
	end_dialog(conf, shadow, widget, textpad);

	return (BSDDIALOG_OK);
}


/* Mixedgauge */
static int
mixedgauge(struct bsddialog_conf *conf, char* text, int rows, int cols,
    unsigned int mainperc, unsigned int nminibars, char **minilabels,
    int *minipercs, bool color)
{
	int i, output, miniperc, y, x, h, w, max_minbarlen;
	int maxword, maxline, nlines, ypad, htextpad;
	int colorperc, red, green;
	WINDOW *widget, *textpad, *bar, *shadow;
	char states[12][14] = {
	    "  Succeeded  ", /*  0  */
	    "   Failed    ", /*  1  */
	    "   Passed    ", /*  2  */
	    "  Completed  ", /*  3  */
	    "   Checked   ", /*  4  */
	    "    Done     ", /*  5  */
	    "   Skipped   ", /*  6  */
	    " In Progress ", /*  7  */
	    "(blank)      ", /*  8  */
	    "     N/A     ", /*  9  */
	    "   Pending   ", /* 10  */
	    "   UNKNOWN   ", /* 10+ */
	};

	red   = bsddialog_color(BSDDIALOG_WHITE,BSDDIALOG_RED,  BSDDIALOG_BOLD);
	green = bsddialog_color(BSDDIALOG_WHITE,BSDDIALOG_GREEN,BSDDIALOG_BOLD);

	max_minbarlen = 0;
	for (i=0; i < (int)nminibars; i++)
		max_minbarlen = MAX(max_minbarlen, (int)strlen(minilabels[i]));
	max_minbarlen += 3 + 16 /* seps + [...] or mainbar */;

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);

	/* mixedgauge autosize */
	if (get_text_properties(conf, text, &maxword, &maxline, &nlines) != 0)
		return (BSDDIALOG_ERROR);

	if (cols == BSDDIALOG_AUTOSIZE) {
		w = MAX(max_minbarlen, (int)(maxline + t.text.hmargin * 2));
		w = widget_min_width(conf, NULL, w);
	}
	if (rows == BSDDIALOG_AUTOSIZE) {
		h = nminibars + 3 /* mainbar */ + (strlen(text) > 0 ? 3 : 0);
		h = widget_min_height(conf, false, h);
	}

	/* mixedgauge checksize */
	if (w < max_minbarlen + 2)
		RETURN_ERROR("Few cols for this mixedgauge");
	if (h < 5 + (int)nminibars + (strlen(text) > 0 ? 1 : 0))
		RETURN_ERROR("Few rows for this mixedgauge");

	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	output = new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text,
	    NULL, false);
	if (output == BSDDIALOG_ERROR)
		return (output);

	/* mini bars */
	for (i=0; i < (int)nminibars; i++) {
		miniperc = minipercs[i];
		if (miniperc == 8)
			continue;
		/* label */
		if (color && (miniperc == 7 || miniperc < 0))
			wattron(widget, A_BOLD);
		mvwaddstr(widget, i+1, 2, minilabels[i]);
			wattroff(widget, A_BOLD);
		/* perc */
		if (miniperc > 10)
			mvwaddstr(widget, i+1, w-2-15, states[11]);
		else if (miniperc >= 0 && miniperc <= 10) {
			mvwaddstr(widget, i+1, w-2-15, "[             ]");
			if (color && miniperc == 1) /* Failed */
				colorperc = red;
			if (color && miniperc == 5) /* Done */
				colorperc = green;
			if (color && (miniperc == 1 || miniperc == 5))
				wattron(widget, colorperc);
			mvwaddstr(widget, i+1, 1+w-2-15, states[miniperc]);
			if (color && (miniperc == 1 || miniperc == 5))
				wattroff(widget, colorperc);
		}
		else { /* miniperc < 0 */
			miniperc = abs(miniperc);
			mvwaddstr(widget, i+1, w-2-15, "[             ]");
			draw_bar(widget, i+1, 1+w-2-15, 13, miniperc, false,
			    -1 /*unused*/);
		}
	}

	wrefresh(widget);
	getmaxyx(textpad, htextpad, i /* unused */);
	ypad =  y + h - 5 - htextpad;
	ypad = ypad < y+(int)nminibars ? y+nminibars : ypad;
	prefresh(textpad, 0, 0, ypad, x+2, y+h-4, x+w-2);
	
	/* main bar */
	bar = new_boxed_window(conf, y+h -4, x+3, 3, w-6, RAISED);
	
	draw_bar(bar, 1, 1, w-8, mainperc, false, -1 /*unused*/);

	wattron(bar, t.bar.color);
	mvwaddstr(bar, 0, 2, "Overall Progress");
	wattroff(bar, t.bar.color);

	wrefresh(bar);

	/* getch(); port ncurses shows nothing */

	delwin(bar);
	end_dialog(conf, shadow, widget, textpad);

	return (BSDDIALOG_OK);
}

int
bsddialog_mixedgauge(struct bsddialog_conf *conf, char* text, int rows,
    int cols, unsigned int mainperc, unsigned int nminibars, char **minilabels,
    int *minipercs)
{
	int output;

	output = mixedgauge(conf, text, rows, cols, mainperc, nminibars,
	    minilabels, minipercs, false);

	return (output);
}

int
bsddialog_progressview (struct bsddialog_conf *conf, char * text, int rows,
    int cols, struct bsddialog_progviewconf *pvconf, unsigned int nminibar,
    struct bsddialog_fileminibar *minibar)
{
	int perc, output;
	int *minipercs;
	unsigned int i;
	char **minilabels;
	unsigned int mainperc, totaltodo;
	time_t tstart, told, tnew, refresh;
	bool update;
	float readforsec;

	if ((minilabels = calloc(nminibar, sizeof(char*))) == NULL)
		RETURN_ERROR("Cannot allocate memory for minilabels\n");
	if ((minipercs = calloc(nminibar, sizeof(int))) == NULL)
		RETURN_ERROR("Cannot allocate memory for minipercs\n");

	totaltodo = 0;
	for(i=0; i<nminibar; i++) {
		totaltodo     += minibar[i].size;
		minilabels[i] = minibar[i].label;
		minipercs[i]  = 10; /*Pending*/
	}

	refresh = pvconf->refresh == 0 ? 0 : pvconf->refresh - 1;
	output = BSDDIALOG_OK;
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
			output = mixedgauge(conf, text, rows, cols, mainperc,
			    nminibar, minilabels, minipercs, true);
			if (output == BSDDIALOG_ERROR)
				return (BSDDIALOG_ERROR);

			move(LINES-1, 2);
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
		if (minibar[i].status == 1) /* Failed*/
			break;

		perc = pvconf->callback(&minibar[i]);

		if (minibar[i].status == 5) {/* ||prec >= 100) Done */
			minipercs[i] = 5;
			update = true;
			i++;
		} else if (minibar[i].status == 1 || perc < 0) { /* Failed */
			minipercs[i] = 1;
			update = true;
		} else if (perc == 0)
			minipercs[i] = 7; /* In progress */
		else /* perc > 0 */
			minipercs[i] = -(perc);
	}

	free(minilabels);
	free(minipercs);
	return (output);
}

int
bsddialog_rangebox(struct bsddialog_conf *conf, char* text, int rows, int cols,
    int min, int max, int *value)
{
	WINDOW *widget, *textpad, *bar, *shadow;
	int y, x, h, w;
	bool loop, buttupdate, barupdate;
	int input, currvalue, output, sizebar, bigchange, positions;
	float perc;
	struct buttons bs;

	if (value == NULL)
		RETURN_ERROR("*value cannot be NULL");

	if (min >= max)
		RETURN_ERROR("min >= max");

	currvalue = *value;
	positions = max - min + 1;

	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (bar_autosize(conf, rows, cols, &h, &w, text, &bs) != 0)
		return (BSDDIALOG_ERROR);
	if (bar_checksize(text, h, w, &bs) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs,
	    true) != 0)
		return (BSDDIALOG_ERROR);

	doupdate();

	prefresh(textpad, 0, 0, y+1, x+1+t.text.hmargin, y+h-7, 
	    x+w-1-t.text.hmargin);

	sizebar = w - HBORDERS - 2 - BARMARGIN * 2;
	bigchange = MAX(1, sizebar/10);

	bar = new_boxed_window(conf, y + h - 6, x + 1 + BARMARGIN, 3,
	    sizebar + 2, RAISED);

	loop = buttupdate = barupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}
		if (barupdate) {
			perc = ((float)(currvalue - min)*100) / (positions-1);
			draw_bar(bar, 1, 1, sizebar, perc, true, currvalue);
			barupdate = false;
			wrefresh(bar);
		}

		input = getch();
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			output = bs.value[bs.curr];
			*value = currvalue;
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				output = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				buttupdate = true;
			}
			break;
		case KEY_HOME:
			currvalue = max;
			barupdate = true;
			break;
		case KEY_END:
			currvalue = min;
			barupdate = true;
			break;
		case KEY_NPAGE:
			currvalue -= bigchange;
			if (currvalue < min)
				currvalue = min;
			barupdate = true;
			break;
		case KEY_PPAGE:
			currvalue += bigchange;
			if (currvalue > max)
				currvalue = max;
			barupdate = true;
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
		case KEY_F(1):
			if (conf->f1_file == NULL && conf->f1_message == NULL)
				break;
			if (f1help(conf) != 0)
				return (BSDDIALOG_ERROR);
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_widget(y, x, h, w, conf->shadow);
			refresh();
			
			if (set_widget_size(conf, rows, cols, &h, &w) != 0)
				return (BSDDIALOG_ERROR);
			if (bar_autosize(conf, rows, cols, &h, &w, text,
			    &bs) != 0)
				return (BSDDIALOG_ERROR);
			if (bar_checksize(text, h, w, &bs) != 0)
				return (BSDDIALOG_ERROR);
			if (set_widget_position(conf, &y, &x, h, w) != 0)
				return (BSDDIALOG_ERROR);

			if(update_dialog(conf, shadow, widget,y, x, h, w,
			    textpad, text, &bs, true) != 0)
				return (BSDDIALOG_ERROR);

			doupdate();

			sizebar = w - HBORDERS - 2 - BARMARGIN * 2;
			bigchange = MAX(1, sizebar/10);
			wclear(bar);
			mvwin(bar, y + h - 6, x + 1 + BARMARGIN);
			wresize(bar, 3, sizebar + 2);
			draw_borders(conf, bar, 3, sizebar+2, RAISED);

			prefresh(textpad, 0, 0, y+1, x+1+t.text.hmargin, y+h-7, 
			    x+w-1-t.text.hmargin);

			barupdate = true;
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				output = bs.value[bs.curr];
				loop = false;
			}
		}
	}

	delwin(bar);
	end_dialog(conf, shadow, widget, textpad);

	return (output);
}

int
bsddialog_pause(struct bsddialog_conf *conf, char* text, int rows, int cols,
    unsigned int sec)
{
	WINDOW *widget, *textpad, *bar, *shadow;
	int output, y, x, h, w;
	bool loop, buttupdate, barupdate;
	int input, tout, sizebar;
	float perc;
	struct buttons bs;

	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (bar_autosize(conf, rows, cols, &h, &w, text, &bs) != 0)
		return (BSDDIALOG_ERROR);
	if (bar_checksize(text, h, w, &bs) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs,
	    true) != 0)
		return (BSDDIALOG_ERROR);

	doupdate();
	
	prefresh(textpad, 0, 0, y+1, x+1+t.text.hmargin, y+h-7, 
	    x+w-1-t.text.hmargin);

	sizebar = w - HBORDERS - 2 - BARMARGIN * 2;
	bar = new_boxed_window(conf, y + h - 6, x + 1 + BARMARGIN, 3,
	    sizebar + 2, RAISED);

	tout = sec;
	nodelay(stdscr, TRUE);
	timeout(1000);
	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			perc = (float)tout * 100 / sec;
			draw_bar(bar, 1, 1, sizebar, perc, true, tout);
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(widget, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}

		input = getch();
		if(input < 0) { /* timeout */
			tout--;
			if (tout < 0) {
				output = BSDDIALOG_TIMEOUT;
				break;
			}
			else {
				barupdate = true;
				continue;
			}
		}
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			output = bs.value[bs.curr];
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				output = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				buttupdate = true;
			}
			break;
		case KEY_F(1):
			if (conf->f1_file == NULL && conf->f1_message == NULL)
				break;
			if (f1help(conf) != 0)
				return (BSDDIALOG_ERROR);
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_widget(y, x, h, w, conf->shadow);
			refresh();
			
			if (set_widget_size(conf, rows, cols, &h, &w) != 0)
				return (BSDDIALOG_ERROR);
			if (bar_autosize(conf, rows, cols, &h, &w, text,
			    &bs) != 0)
				return (BSDDIALOG_ERROR);
			if (bar_checksize(text, h, w, &bs) != 0)
				return (BSDDIALOG_ERROR);
			if (set_widget_position(conf, &y, &x, h, w) != 0)
				return (BSDDIALOG_ERROR);

			if(update_dialog(conf, shadow, widget,y, x, h, w,
			    textpad, text, &bs, true) != 0)
				return (BSDDIALOG_ERROR);

			doupdate();

			sizebar = w - HBORDERS - 2 - BARMARGIN * 2;
			wclear(bar);
			mvwin(bar, y + h - 6, x + 1 + BARMARGIN);
			wresize(bar, 3, sizebar + 2);
			draw_borders(conf, bar, 3, sizebar+2, LOWERED);

			prefresh(textpad, 0, 0, y+1, x+1+t.text.hmargin, y+h-7, 
			    x+w-1-t.text.hmargin);

			barupdate = true;
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				output = bs.value[bs.curr];
				loop = false;
			}
		}
	}

	nodelay(stdscr, FALSE);

	delwin(bar);
	end_dialog(conf, shadow, widget, textpad);

	return (output);
}
