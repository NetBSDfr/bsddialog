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
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define MINWTIME   14 /* 3 windows and their borders */

struct clock {
	unsigned int max;
	unsigned int value;
	WINDOW *win;
};

static void
drawsquare(struct bsddialog_conf *conf, WINDOW *win, unsigned int value,
    bool focus)
{
	int h, w;

	getmaxyx(win, h, w);
	draw_borders(conf, win, h, w, LOWERED);
	if (focus) {
		wattron(win, t.dialog.arrowcolor);
		mvwhline(win, 0, 1, conf->ascii_lines ? '^' : ACS_UARROW, 2);
		mvwhline(win, 2, 1, conf->ascii_lines ? 'v' : ACS_DARROW, 2);
		wattroff(win, t.dialog.arrowcolor);
	}

	if (focus)
		wattron(win, t.menu.f_namecolor);
	mvwprintw(win, 1, 1, "%02u", value);
	if (focus)
		wattroff(win, t.menu.f_namecolor);

	wrefresh(win);
}

int
bsddialog_timebox(struct bsddialog_conf *conf, const char* text, int rows,
    int cols, unsigned int *hh, unsigned int *mm, unsigned int *ss)
{
	bool loop, focusbuttons;
	int i, retval, y, x, h, w, sel;
	wint_t input;
	WINDOW *widget, *textpad, *shadow;
	struct buttons bs;
	struct clock c[3] = {
		{23, *hh, NULL},
		{59, *mm, NULL},
		{59, *ss, NULL}
	};

	CHECK_PTR(hh, unsigned int);
	CHECK_PTR(mm, unsigned int);
	CHECK_PTR(ss, unsigned int);

	for (i = 0 ; i < 3; i++) {
		if (c[i].value > c[i].max)
			c[i].value = c[i].max;
	}

	get_buttons(conf, &bs, true, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if (widget_size_position(conf, rows, cols, text, 3 /*windows*/,
	    MINWTIME, &bs, &y, &x, &h, &w, NULL) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);

	pnoutrefresh(textpad, 0, 0, y+1, x+2, y+h-7, x+w-2);
	doupdate();

	c[0].win = new_boxed_window(conf, y+h-6, x + w/2 - 7, 3, 4, LOWERED);
	mvwaddch(widget, h - 5, w/2 - 3, ':');
	c[1].win = new_boxed_window(conf, y+h-6, x + w/2 - 2, 3, 4, LOWERED);
	mvwaddch(widget, h - 5, w/2 + 2, ':');
	c[2].win = new_boxed_window(conf, y+h-6, x + w/2 + 3, 3, 4, LOWERED);

	wrefresh(widget);

	sel = -1;
	loop = focusbuttons = true;
	while (loop) {
		for (i = 0; i < 3; i++)
			drawsquare(conf, c[i].win, c[i].value, sel == i);

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
			} else { c[sel].value = c[sel].value > 0 ?
			    c[sel].value - 1 : c[sel].max;
			}
			break;
		case KEY_DOWN:
			if (focusbuttons)
				break;
			c[sel].value = c[sel].value < c[sel].max ?
			    c[sel].value + 1 : 0;
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
			    3 /*windows*/, MINWTIME, &bs, &y, &x, &h, &w,
			    NULL) != 0)
				return (BSDDIALOG_ERROR);
			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs) != 0)
				return (BSDDIALOG_ERROR);

			doupdate();

			mvwaddch(widget, h - 5, w/2 - 3, ':');
			mvwaddch(widget, h - 5, w/2 + 2, ':');
			wrefresh(widget);

			prefresh(textpad, 0, 0, y+1, x+2, y+h-7, x+w-2);

			wclear(c[0].win);
			mvwin(c[0].win, y + h - 6, x + w/2 - 7);
			wclear(c[1].win);
			mvwin(c[1].win, y + h - 6, x + w/2 - 2);
			wclear(c[2].win);
			mvwin(c[2].win, y + h - 6, x + w/2 + 3);

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
		*hh = c[0].value;
		*mm = c[1].value;
		*ss = c[2].value;
	}

	for (i = 0; i < 3; i++)
		delwin(c[i].win);
	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}
