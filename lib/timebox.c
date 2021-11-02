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

#include <time.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"

/* "Time": timebox - calendar */

int bsddialog_timebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int hh, unsigned int mm, unsigned int ss)
{
	WINDOW *widget, *shadow;
	char stringtime[1024];
	int i, input, output, y, x, sel;
	struct buttons bs;
	bool loop, buttupdate;
	time_t clock;
	struct tm *localtm;
	struct myclockstruct {
		unsigned int max;
		unsigned int curr;
		WINDOW *win;
	} c[3] = { {23, hh, NULL}, {59, mm, NULL}, {59, ss, NULL} };

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	c[0].win = new_window(y + rows - 6, x + cols/2 - 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 - 3, ':');
	c[1].win = new_window(y + rows - 6, x + cols/2 - 2, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 + 2, ':');
	c[2].win = new_window(y + rows - 6, x + cols/2 + 3, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	sel=0;
	curs_set(2);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, rows-2, cols, bs, true);
			wrefresh(widget);
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
			output = bs.value[bs.curr]; // values -> outputs
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
		case KEY_UP:
			c[sel].curr = c[sel].curr < c[sel].max ? c[sel].curr + 1 : 0;
			break;
		case KEY_DOWN:
			c[sel].curr = c[sel].curr > 0 ? c[sel].curr - 1 : c[sel].max;
			break;
		}
	}

	curs_set(0);

	for (i=0; i<3; i++)
		delwin(c[i].win);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}

int bsddialog_calendar(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int yy, unsigned int mm, unsigned int dd)
{
	WINDOW *widget, *shadow;
	char stringtime[1024];
	int i, input, output, y, x, sel;
	struct buttons bs;
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

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	c[0].win = new_window(y + rows - 6, x + cols/2 - 12, 3, 6, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 - 6, '/');
	c[1].win = new_window(y + rows - 6, x + cols/2 - 5, 3, 11, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 + 6, '/');
	c[2].win = new_window(y + rows - 6, x + cols/2 + 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);

	wrefresh(widget);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	sel=2;
	curs_set(2);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, rows-2, cols, bs, true);
			wrefresh(widget);
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
			output = bs.value[bs.curr]; // values -> outputs
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
		case KEY_UP:
			c[sel].curr = c[sel].curr < c[sel].max ? c[sel].curr + 1 : 1;
			break;
		case KEY_DOWN:
			c[sel].curr = c[sel].curr > 1 ? c[sel].curr - 1 : c[sel].max;
			break;
		}
	}

	curs_set(0);

	for (i=0; i<3; i++)
		delwin(c[i].win);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}
