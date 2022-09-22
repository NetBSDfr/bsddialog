/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Alfonso Sabato Siciliano
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
#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define MINHCAL     14
#define MINWCAL     38 /* 34 calendar, 4 margins */
#define MINYEAR   1900

#define ISLEAF(year) ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

static int
datetime_autosize(struct bsddialog_conf *conf, int rows, int cols, int *h,
    int *w, const char *text, struct buttons bs)
{
	int htext, wtext;

	if (cols == BSDDIALOG_AUTOSIZE || rows == BSDDIALOG_AUTOSIZE) {
		if (text_size(conf, rows, cols, text, &bs, MINHCAL, MINWCAL,
		    &htext, &wtext) != 0)
			return (BSDDIALOG_ERROR);
	}

	if (cols == BSDDIALOG_AUTOSIZE)
		*w = widget_min_width(conf, wtext, MINWCAL, &bs);

	if (rows == BSDDIALOG_AUTOSIZE)
		*h = widget_min_height(conf, htext, MINHCAL, true);

	return (0);
}

static int
datetime_checksize(int rows, int cols, struct buttons bs)
{
	int mincols;

	mincols = VBORDERS;
	mincols += buttons_min_width(bs);
	mincols = MAX(MINWCAL, mincols);

	if (cols < mincols)
		RETURN_ERROR("Few cols for this timebox/datebox");

	if (rows < MINHCAL + 2 + 2) /* cal + 2 button + 2 borders */
		RETURN_ERROR("Few rows for this timebox/datebox, at least 7");

	return (0);
}

static int month_days(int yy, int mm)
{
	int days;

	if (mm == 2)
		days = ISLEAF(yy) ? 29 : 28;
	else if (mm == 4 || mm == 6 || mm == 9 || mm == 11)
		days = 30;
	else
		days = 31;

	return days;

}

static int week_day(int yy, int mm, int dd)
{
	int Y, M, D, wd;

	Y = yy;
	D = dd;
	M = mm;
	wd  = (D += M < 3 ? Y-- : Y - 2, 23*M/9 + D + 4 + Y/4- Y/100 + Y/400)%7; 

	return (wd);
}

static void
print_calendar(struct bsddialog_conf *conf, WINDOW *win, int yy, int mm, int dd,
    bool active)
{
	int ndays, i, y, x, wd, h, w;
	int color;

	wclear(win);
	getmaxyx(win, h, w);
	draw_borders(conf, win, h, w, RAISED);
	mvwaddstr(win, 1, 5, "Sun Mon Tue Wed Thu Fri Sat");

	color = t.menu.f_namecolor;
	if (active == false)
		color |=  (A_REVERSE | A_BOLD);

	ndays = month_days(yy, mm);

	y = 2;
	wd = week_day(yy, mm, 1);
	for (i = 1; i <= ndays; i++) {
		x = 5 + (4 * wd); /* x has to be 6 with week number */
		wmove(win, y, x);
		mvwprintw(win, y, x, "%2d", i);
		if (i == dd) {
			wattron(win, color);
			mvwprintw(win, y, x, "%2d", i);
			wattroff(win, color);
		}
		wd++;
		if (wd > 6) {
			wd = 0;
			y++;
		}
	}

	wrefresh(win);
}

static void
drawsquare2(struct bsddialog_conf *conf, WINDOW *win, const char *fmt,
    const void *value, bool focus)
{
	int h, l, w;

	getmaxyx(win, h, w);
	draw_borders(conf, win, h, w, RAISED);
	if (focus) {
		l = 2 + w % 2;
		wattron(win, t.dialog.arrowcolor);
		mvwhline(win, 0, w/2 - l/2,
		    conf->ascii_lines ? '^' : ACS_UARROW, l);
		mvwhline(win, h-1, w/2 - l/2,
		    conf->ascii_lines ? 'v' : ACS_DARROW, l);
		wattroff(win, t.dialog.arrowcolor);
	}

	if (focus)
		wattron(win, t.menu.f_namecolor);
	if (strchr(fmt, 's') != NULL)
		mvwprintw(win, 1, 1, fmt, (const char*)value);
	else
		mvwprintw(win, 1, 1, fmt, *((const int*)value));
	if (focus)
		wattroff(win, t.menu.f_namecolor);

	wrefresh(win);
}

enum operation {
	UP_DAY,
	DOWN_DAY,
	LEFT_DAY,
	RIGHT_DAY,
	UP_MONTH,
	DOWN_MONTH,
	UP_YEAR,
	DOWN_YEAR
};

static void datectl(enum operation op, int *yy, int *mm, int *dd)
{
	int ndays;

	ndays = month_days(*yy, *mm);

	switch (op) {
	case UP_DAY:
		if (*dd > 7)
			*dd -= 7;
		else {
			if (*mm == 1) {
				*yy -= 1;
				*mm = 12;
			} else
				*mm -= 1;
			ndays = month_days(*yy, *mm);
			*dd = ndays - abs(7 - *dd);
		}
		break;
	case DOWN_DAY:
		if (*dd + 7 < ndays)
			*dd += 7;
		else {
			if (*mm == 12) {
				*yy += 1;
				*mm = 1;
			} else
				*mm += 1;
			*dd = *dd + 7 - ndays;
		}
		break;
	case LEFT_DAY:
		if (*dd > 1)
			*dd -= 1;
		else {
			if (*mm == 1) {
				*yy -= 1;
				*mm = 12;
			} else
				*mm -= 1;
			*dd = month_days(*yy, *mm);
		}
		break;
	case RIGHT_DAY:
		if (*dd < ndays)
			*dd += 1;
		else {
			if (*mm == 12) {
				*yy += 1;
				*mm = 1;
			} else
				*mm += 1;
			*dd = 1;
		}
		break;
	case UP_MONTH:
		if (*mm == 1) {
			*mm = 12;
			*yy -= 1;
		} else
			*mm -= 1;
		ndays = month_days(*yy, *mm);
		if (*dd > ndays)
			*dd = ndays;
		break;
	case DOWN_MONTH:
		break;
	case UP_YEAR:
		break;
	case DOWN_YEAR:
		break;
	}

	if (*yy < MINYEAR) {
		*yy = MINYEAR;
		*mm = 1;
		*dd = 1;
	}
	if (*yy > 9999) { // date.maxyy
		*yy = 9999;
		*mm = 12;
		*dd = 31;
	}
}

int
bsddialog_calendar(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int *yy, unsigned int *mm, unsigned int *dd)
{
	bool loop, focusbuttons;
	int retval, y, x, h, w, sel, ycal, xcal;
	wint_t input;
	int year, month, day;
	WINDOW *widget, *textpad, *shadow, *yearwin, *monthwin, *daywin;
	struct buttons bs;
	const char *m[12] = {
		"January", "February", "March", "April", "May", "June", "July",
		"August", "September", "October", "November", "December"
	};

	if (yy == NULL || mm == NULL || dd == NULL)
		RETURN_ERROR("yy / mm / dd cannot be NULL");

	year = *yy > 9999 ? 9999 : *yy;
	if (year < 1900)
		year = 1900;
	month = *mm > 12 ? 12 : *mm;
	if (month == 0)
		month = 1;
	day = *dd == 0 ? 1 : *dd;
	if(day > month_days(year, month))
		day = month_days(year, month);

	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (datetime_autosize(conf, rows, cols, &h, &w, text, bs) != 0)
		return (BSDDIALOG_ERROR);
	if (datetime_checksize(h, w, bs) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs,
	    true) != 0)
		return (BSDDIALOG_ERROR);

	pnoutrefresh(textpad, 0, 0, y+1, x+2, y+h-7, x+w-2);
	doupdate();

	ycal = y + h - 15;
	xcal = x + w/2 - 17;
	mvwaddstr(widget, h - 16, w/2 - 17, "Month");
	monthwin = new_boxed_window(conf, ycal, xcal, 3, 17, RAISED);
	mvwaddstr(widget, h - 16, w/2, "Year");
	yearwin = new_boxed_window(conf, ycal, xcal + 17, 3, 17, RAISED);
	daywin = new_boxed_window(conf, ycal + 3, xcal, 9, 34, RAISED);

	wrefresh(widget);

	sel = -1;
	loop = focusbuttons = true;
	while (loop) {
		drawsquare2(conf, monthwin, "%15s", m[month-1], sel == 0);
		drawsquare2(conf, yearwin, "%15d", &year, sel == 1);
		print_calendar(conf, daywin, year, month, day, sel == 2);

		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			if (focusbuttons || conf->button.always_active) {
				retval = bs.value[bs.curr];
				loop = false;
			}
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			if (focusbuttons) {
				bs.curr++;
				focusbuttons = bs.curr < (int)bs.nbuttons ?
				    true : false;
				if (focusbuttons == false) {
					sel = 0;
					bs.curr = conf->button.always_active ? 0 : -1;
				}
			} else {
				sel++;
				focusbuttons = sel > 2 ? true : false;
				if (focusbuttons) {
					bs.curr = 0;
				}
			}
			draw_buttons(widget, bs, true);
			wrefresh(widget);
			break;
		case KEY_RIGHT:
			if (focusbuttons) {
				bs.curr++;
				focusbuttons = bs.curr < (int)bs.nbuttons ?
				    true : false;
				if (focusbuttons == false) {
					sel = 0;
					bs.curr = conf->button.always_active ? 0 : -1;
				}
			} else if (sel == 2) {
				datectl(RIGHT_DAY, &year, &month, &day);
			} else {
				sel++;
				focusbuttons = sel > 2 ? true : false;
				if (focusbuttons) {
					bs.curr = 0;
				}
			}
			draw_buttons(widget, bs, true);
			wrefresh(widget);
			break;
		case KEY_LEFT:
			if (focusbuttons) {
				bs.curr--;
				focusbuttons = bs.curr < 0 ? false : true;
				if (focusbuttons == false) {
					sel = 2;
					bs.curr = conf->button.always_active ? 0 : -1;
				}
			} else if (sel == 2) {
				datectl(LEFT_DAY, &year, &month, &day);
			} else {
				sel--;
				focusbuttons = sel < 0 ? true : false;
				if (focusbuttons)
					bs.curr = (int)bs.nbuttons - 1;
			}
			draw_buttons(widget, bs, true);
			wrefresh(widget);
			break;
		case KEY_UP:
			if (focusbuttons) {
				sel = 2;
				focusbuttons = false;
				bs.curr = conf->button.always_active ? 0 : -1;
				draw_buttons(widget, bs, true);
				wrefresh(widget);
			} else
				datectl(UP_DAY, &year, &month, &day);
			break;
		case KEY_DOWN:
			if (focusbuttons)
				break;
			datectl(DOWN_DAY, &year, &month, &day);
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help(conf) != 0)
				return (BSDDIALOG_ERROR);
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_widget(y, x, h, w, conf->shadow);
			refresh();

			/*if (set_widget_size(conf, rows, cols, &h, &w) != 0)
				return (BSDDIALOG_ERROR);
			if (datetime_autosize(conf, rows, cols, &h, &w,
			    MINHCAL, MINWCAL, text, bs) != 0)
				return (BSDDIALOG_ERROR);
			if (datetime_checksize(h, w, MINWCAL, bs) != 0)
				return (BSDDIALOG_ERROR);
			if (set_widget_position(conf, &y, &x, h, w) != 0)
				return (BSDDIALOG_ERROR);

			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs, true) != 0)
				return (BSDDIALOG_ERROR);
			doupdate();

			mvwaddch(widget, h - 5, w/2 - 5, '/');
			mvwaddch(widget, h - 5, w/2 + 7, '/');
			wrefresh(widget);

			prefresh(textpad, 0, 0, y+1, x+2, y+h-7, x+w-2);

			wclear(c[0].win);
			mvwin(c[0].win, y + h - 6, x + w/2 - 11);
			wclear(c[1].win);
			mvwin(c[1].win, y + h - 6, x + w/2 - 4);
			wclear(c[2].win);
			mvwin(c[2].win, y + h - 6, x + w/2 + 8);*/

			/* Important to avoid grey lines expanding screen */
			refresh();
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				retval = bs.value[bs.curr];
				loop = false;
			}
		}
	}

	if (retval == BSDDIALOG_OK) {
		*yy = year;
		*mm = month;
		*dd = day;
	}

	delwin(yearwin);
	delwin(monthwin);
	delwin(daywin);
	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}