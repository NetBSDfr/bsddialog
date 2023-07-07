/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022-2023 Alfonso Sabato Siciliano
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

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define MINHCAL   13
#define MINWCAL   36 /* 34 calendar, 1 + 1 margins */
#define MINYEAR   1900
#define MAXYEAR   999999999

#define MINWDATE   23 /* 3 windows and their borders */

#define ISLEAP(year) ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

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

static int month_days(int yy, int mm)
{
	int days;

	if (mm == 2)
		days = ISLEAP(yy) ? 29 : 28;
	else if (mm == 4 || mm == 6 || mm == 9 || mm == 11)
		days = 30;
	else
		days = 31;

	return (days);
}

static int week_day(int yy, int mm, int dd)
{
	int wd;

	dd += mm < 3 ? yy-- : yy - 2;
	wd = 23*mm/9 + dd + 4 + yy/4 - yy/100 + yy/400;
	wd %= 7;

	return (wd);
}

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
		if (*mm == 12) {
			*mm = 1;
			*yy += 1;
		} else
			*mm += 1;
		ndays = month_days(*yy, *mm);
		if (*dd > ndays)
			*dd = ndays;
		break;
	case UP_YEAR:
		*yy -= 1;
		ndays = month_days(*yy, *mm);
		if (*dd > ndays)
			*dd = ndays;
		break;
	case DOWN_YEAR:
		*yy += 1;
		ndays = month_days(*yy, *mm);
		if (*dd > ndays)
			*dd = ndays;
		break;
	}

	if (*yy < MINYEAR) {
		*yy = MINYEAR;
		*mm = 1;
		*dd = 1;
	}
	if (*yy > MAXYEAR) {
		*yy = MAXYEAR;
		*mm = 12;
		*dd = 31;
	}
}

static void
print_calendar(struct bsddialog_conf *conf, WINDOW *win, int yy, int mm, int dd,
    bool active)
{
	int ndays, i, y, x, wd, h, w;

	getmaxyx(win, h, w);
	wclear(win);
	draw_borders(conf, win, h, w, RAISED);
	if (active) {
		wattron(win, t.dialog.arrowcolor);
		mvwhline(win, 0, 15, conf->ascii_lines ? '^' : ACS_UARROW, 4);
		mvwhline(win, h-1, 15, conf->ascii_lines ? 'v' : ACS_DARROW, 4);
		mvwvline(win, 3, 0, conf->ascii_lines ? '<' : ACS_LARROW, 3);
		mvwvline(win, 3, w-1, conf->ascii_lines ? '>' : ACS_RARROW, 3);
		wattroff(win, t.dialog.arrowcolor);
	}

	mvwaddstr(win, 1, 5, "Sun Mon Tue Wed Thu Fri Sat");
	ndays = month_days(yy, mm);
	y = 2;
	wd = week_day(yy, mm, 1);
	for (i = 1; i <= ndays; i++) {
		x = 5 + (4 * wd); /* x has to be 6 with week number */
		wmove(win, y, x);
		mvwprintw(win, y, x, "%2d", i);
		if (i == dd) {
			wattron(win, t.menu.f_namecolor);
			mvwprintw(win, y, x, "%2d", i);
			wattroff(win, t.menu.f_namecolor);
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
drawsquare(struct bsddialog_conf *conf, WINDOW *win, enum elevation elev,
    const char *fmt, const void *value, bool focus)
{
	int h, l, w;

	getmaxyx(win, h, w);
	draw_borders(conf, win, h, w, elev);
	if (focus) {
		l = 2 + w%2;
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

int
bsddialog_calendar(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int *year, unsigned int *month, unsigned int *day)
{
	bool loop, focusbuttons;
	int retval, y, x, h, w, sel, ycal, xcal, yy, mm, dd;
	wint_t input;
	WINDOW *widget, *textpad, *shadow, *yearwin, *monthwin, *daywin;
	struct buttons bs;
	const char *m[12] = {
		"January", "February", "March", "April", "May", "June", "July",
		"August", "September", "October", "November", "December"
	};

	if (year == NULL || month == NULL || day == NULL)
		RETURN_ERROR("yy / mm / dd cannot be NULL");

	yy = *year > MAXYEAR ? MAXYEAR : *year;
	if (yy < MINYEAR)
		yy = MINYEAR;
	mm = *month > 12 ? 12 : *month;
	if (mm == 0)
		mm = 1;
	dd = *day == 0 ? 1 : *day;
	if(dd > month_days(yy, mm))
		dd = month_days(yy, mm);

	get_buttons(conf, &bs, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if (widget_size_position(conf, rows, cols, text, MINHCAL, MINWCAL, &bs,
	    &y, &x, &h, &w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);

	pnoutrefresh(textpad, 0, 0, y+1, x+2, y+h-17, x+w-2);
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
		drawsquare(conf, monthwin, RAISED, "%15s", m[mm - 1], sel == 0);
		drawsquare(conf, yearwin, RAISED, "%15d", &yy, sel == 1);
		print_calendar(conf, daywin, yy, mm, dd, sel == 2);

		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			if (focusbuttons || conf->button.always_active) {
				retval = BUTTONVALUE(bs);
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
				if (bs.curr >= (int)bs.nbuttons) {
					focusbuttons = false;
					sel = 0;
					bs.curr = conf->button.always_active ?
					    0 : -1;
				}
			} else {
				sel++;
				if (sel > 2) {
					focusbuttons = true;
					sel = -1;
					bs.curr = 0;
				}
			}
			draw_buttons(widget, bs);
			wrefresh(widget);
			break;
		case KEY_RIGHT:
			if (focusbuttons) {
				bs.curr++;
				if (bs.curr >= (int)bs.nbuttons) {
					focusbuttons = false;
					sel = 0;
					bs.curr = conf->button.always_active ?
					    0 : -1;
				}
			} else if (sel == 2) {
				datectl(RIGHT_DAY, &yy, &mm, &dd);
			} else { /* Month or Year*/
				sel++;
			}
			draw_buttons(widget, bs);
			wrefresh(widget);
			break;
		case KEY_LEFT:
			if (focusbuttons) {
				bs.curr--;
				if (bs.curr < 0) {
					focusbuttons = false;
					sel = 2;
					bs.curr = conf->button.always_active ?
					    0 : -1;
				}
			} else if (sel == 2) {
				datectl(LEFT_DAY, &yy, &mm, &dd);
			} else if (sel == 1) {
				sel = 0;
			} else { /* sel = 0, Month */
				focusbuttons = true;
				sel = -1;
				bs.curr = 0;
			}
			draw_buttons(widget, bs);
			wrefresh(widget);
			break;
		case KEY_UP:
			if (focusbuttons) {
				sel = 2;
				focusbuttons = false;
				bs.curr = conf->button.always_active ? 0 : -1;
				draw_buttons(widget, bs);
				wrefresh(widget);
			} else if (sel == 0) {
				datectl(UP_MONTH, &yy, &mm, &dd);
			} else if (sel == 1) {
				datectl(UP_YEAR, &yy, &mm, &dd);
			} else { /* sel = 2 */
				datectl(UP_DAY, &yy, &mm, &dd);
			}
			break;
		case KEY_DOWN:
			if (focusbuttons) {
				break;
			} else if (sel == 0) {
				datectl(DOWN_MONTH, &yy, &mm, &dd);
			} else if (sel == 1) {
				datectl(DOWN_YEAR, &yy, &mm, &dd);
			} else { /* sel = 2 */
				datectl(DOWN_DAY, &yy, &mm, &dd);
			}
			break;
		case KEY_HOME:
			datectl(UP_MONTH, &yy, &mm, &dd);
			break;
		case KEY_END:
			datectl(DOWN_MONTH, &yy, &mm, &dd);
			break;
		case KEY_PPAGE:
			datectl(UP_YEAR, &yy, &mm, &dd);
			break;
		case KEY_NPAGE:
			datectl(DOWN_YEAR, &yy, &mm, &dd);
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help_dialog(conf) != 0)
				return (BSDDIALOG_ERROR);
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_dialog(y, x, h, w, conf->shadow);
			refresh();

			if (widget_size_position(conf, rows, cols, text,
			    MINHCAL, MINWCAL, &bs, &y, &x, &h, &w, NULL) != 0)
				return (BSDDIALOG_ERROR);
			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs) != 0)
				return (BSDDIALOG_ERROR);
			pnoutrefresh(textpad, 0, 0, y+1, x+2, y+h-17, x+w-2);
			doupdate();

			ycal = y + h - 15;
			xcal = x + w/2 - 17;
			mvwaddstr(widget, h - 16, w/2 - 17, "Month");
			mvwin(monthwin, ycal, xcal);
			mvwaddstr(widget, h - 16, w/2, "Year");
			mvwin(yearwin, ycal, xcal + 17);
			mvwin(daywin, ycal + 3, xcal);
			wrefresh(widget);

			/* Important to avoid grey lines expanding screen */
			refresh();
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				retval = BUTTONVALUE(bs);
				loop = false;
			}
		}
	}

	if (retval == BSDDIALOG_OK) {
		*year  = yy;
		*month = mm;
		*day   = dd;
	}

	delwin(yearwin);
	delwin(monthwin);
	delwin(daywin);
	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}

int
bsddialog_datebox(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int *year, unsigned int *month, unsigned int *day)
{
	bool loop, focusbuttons;
	int i, retval, y, x, h, w, sel;
	wint_t input;
	WINDOW *widget, *textpad, *shadow;
	struct buttons bs;
	struct calendar {
		int max;
		int value;
		WINDOW *win;
		unsigned int x;
	};
	struct month {
		const char *name;
		unsigned int days;
	};

	if (year == NULL || month == NULL || day == NULL)
		RETURN_ERROR("yy / mm / dd cannot be NULL");

	struct calendar c[3] = {
		{9999, *year,  NULL, 4 },
		{12,   *month, NULL, 9 },
		{31,   *day,   NULL, 2 }
	};

	struct month m[12] = {
		{ "January", 31 }, { "February", 28 }, { "March",     31 },
		{ "April",   30 }, { "May",      31 }, { "June",      30 },
		{ "July",    31 }, { "August",   31 }, { "September", 30 },
		{ "October", 31 }, { "November", 30 }, { "December",  31 }
	};

	for (i = 0 ; i < 3; i++) {
		if (c[i].value > c[i].max)
			c[i].value = c[i].max;
		if (c[i].value < 1)
			c[i].value = 1;
	}
	c[2].max = m[c[1].value -1].days;
	if (c[1].value == 2 && ISLEAP(c[0].value))
		c[2].max = 29;
	if (c[2].value > c[2].max)
		c[2].value = c[2].max;

	get_buttons(conf, &bs, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if (widget_size_position(conf, rows, cols, text, 3 /*windows*/,
	    MINWDATE, &bs, &y, &x, &h, &w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);

	pnoutrefresh(textpad, 0, 0, y+1, x+2, y+h-7, x+w-2);
	doupdate();

	c[0].win = new_boxed_window(conf, y+h-6, x + w/2 - 11, 3, 6, LOWERED);
	mvwaddch(widget, h - 5, w/2 - 5, '/');
	c[1].win = new_boxed_window(conf, y+h-6, x + w/2 - 4, 3, 11, LOWERED);
	mvwaddch(widget, h - 5, w/2 + 7, '/');
	c[2].win = new_boxed_window(conf, y+h-6, x + w/2 + 8, 3, 4, LOWERED);

	wrefresh(widget);

	sel = -1;
	loop = focusbuttons = true;
	while (loop) {
		drawsquare(conf, c[0].win, LOWERED, "%4d", &c[0].value,
		    sel == 0);
		drawsquare(conf, c[1].win, LOWERED, "%9s", m[c[1].value-1].name,
		    sel == 1);
		drawsquare(conf, c[2].win, LOWERED, "%02d", &c[2].value,
		    sel == 2);

		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			if (focusbuttons || conf->button.always_active) {
				retval = BUTTONVALUE(bs);
				loop = false;
			}
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				loop = false;
			}
			break;
		case KEY_RIGHT:
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
			draw_buttons(widget, bs);
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
			} else {
				sel--;
				focusbuttons = sel < 0 ? true : false;
				if (focusbuttons)
					bs.curr = (int)bs.nbuttons - 1;
			}
			draw_buttons(widget, bs);
			wrefresh(widget);
			break;
		case KEY_UP:
			if (focusbuttons) {
				sel = 0;
				focusbuttons = false;
				bs.curr = conf->button.always_active ? 0 : -1;
				draw_buttons(widget, bs);
				wrefresh(widget);
			} else {
				c[sel].value = c[sel].value > 1 ?
				    c[sel].value - 1 : c[sel].max ;
				/* if mount change */
				c[2].max = m[c[1].value -1].days;
				/* if year change */
				if (c[1].value == 2 && ISLEAP(c[0].value))
					c[2].max = 29;
				/* set new day */
				if (c[2].value > c[2].max)
					c[2].value = c[2].max;
			}
			break;
		case KEY_DOWN:
			if (focusbuttons)
				break;
			c[sel].value = c[sel].value < c[sel].max ?
			    c[sel].value + 1 : 1;
			/* if mount change */
			c[2].max = m[c[1].value -1].days;
			/* if year change */
			if (c[1].value == 2 && ISLEAP(c[0].value))
				c[2].max = 29;
			/* set new day */
			if (c[2].value > c[2].max)
				c[2].value = c[2].max;
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help_dialog(conf) != 0)
				return (BSDDIALOG_ERROR);
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_dialog(y, x, h, w, conf->shadow);
			refresh();

			if (widget_size_position(conf, rows, cols, text,
			    3 /*windows*/, MINWDATE, &bs, &y, &x, &h, &w,
			    NULL) != 0)
				return (BSDDIALOG_ERROR);
			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs) != 0)
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
			mvwin(c[2].win, y + h - 6, x + w/2 + 8);

			/* Important to avoid grey lines expanding screen */
			refresh();
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				retval = BUTTONVALUE(bs);
				loop = false;
			}
		}
	}

	if (retval == BSDDIALOG_OK) {
		*year  = c[0].value;
		*month = c[1].value;
		*day   = c[2].value;
	}

	for (i = 0; i < 3; i++)
		delwin(c[i].win);
	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}