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
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

static void
textupdate(WINDOW *widget, WINDOW *textpad, int ytextpad, int htextpad,
    bool htext)
{
	int printrows, y, x, h, w;

	getbegyx(widget, y, x);
	getmaxyx(widget, h, w);
	printrows = h - BORDER - HBUTTONS - BORDER;

	if (htext > 0 && htextpad > printrows) {
		wattron(widget, t.dialog.arrowcolor);
		mvwprintw(widget, h-HBUTTONS-BORDER, w-4-TEXTHMARGIN-BORDER,
		    "%3d%%", 100 * (ytextpad + printrows) / htextpad);
		wattroff(widget, t.dialog.arrowcolor);
		wnoutrefresh(widget);
	}

	pnoutrefresh(textpad, ytextpad, 0, y+1, x+2, y+h-4, x+w-2);
}

int
message_size_position(struct bsddialog_conf *conf, int rows, int cols,
    const char *text, struct buttons *bs, int *y, int *x, int *h, int *w,
    int *htext)
{
	int minw;

	if (set_widget_size(conf, rows, cols, h, w) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_autosize(conf, rows, cols, h, w, text,
	    (*htext < 0) ? htext : NULL, bs, 0, 0) != 0)
		return (BSDDIALOG_ERROR);
	minw = (*htext > 0) ? 1 + TEXTHMARGINS: 0 ;
	if (widget_checksize(*h, *w, bs, MIN(*htext, 1), minw) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, y, x, *h, *w) != 0)
		return (BSDDIALOG_ERROR);

	return (0);
}

static int
do_message(struct bsddialog_conf *conf, const char *text, int rows, int cols,
    struct buttons bs)
{
	bool loop;
	int y, x, h, w, retval, htext, ytextpad, htextpad, printrows, unused;
	WINDOW *widget, *textpad, *shadow;
	wint_t input;

	htext = -1;
	if (message_size_position(conf, rows, cols, text, &bs, &y, &x, &h, &w,
	    &htext) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);

	printrows = h - BORDER - HBUTTONS - BORDER;
	ytextpad = 0;
	getmaxyx(textpad, htextpad, unused);
	unused++; /* fix unused error */
	loop = true;
	while (loop) {
		textupdate(widget, textpad, ytextpad, htextpad, htext);
		doupdate();
		if (get_wch(&input) == ERR)
			continue;
		switch (input) {
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
			draw_buttons(widget, bs);
			wnoutrefresh(widget);
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				draw_buttons(widget, bs);
				wnoutrefresh(widget);
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int)bs.nbuttons - 1) {
				bs.curr++;
				draw_buttons(widget, bs);
				wnoutrefresh(widget);
			}
			break;
		case KEY_UP:
			if (ytextpad > 0)
				ytextpad--;
			break;
		case KEY_DOWN:
			if (ytextpad + printrows < htextpad)
				ytextpad++;
			break;
		case KEY_HOME:
			ytextpad = 0;
			break;
		case KEY_END:
			ytextpad = htextpad - printrows;
			ytextpad = ytextpad < 0 ? 0 : ytextpad;
			break;
		case KEY_PPAGE:
			ytextpad -= printrows;
			ytextpad = ytextpad < 0 ? 0 : ytextpad;
			break;
		case KEY_NPAGE:
			ytextpad += printrows;
			if (ytextpad + printrows > htextpad)
				ytextpad = htextpad - printrows;
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

			if (message_size_position(conf, rows, cols, text, &bs,
			    &y, &x, &h, &w, &htext) != 0)
				return (BSDDIALOG_ERROR);
			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs) != 0)
				return (BSDDIALOG_ERROR);

			printrows = h - BORDER - HBUTTONS - BORDER;
			getmaxyx(textpad, htextpad, unused);
			ytextpad = 0;
			textupdate(widget, textpad, ytextpad, htextpad, htext);

			/* Important to fix grey lines expanding screen */
			refresh();
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				retval = BUTTONVALUE(bs);
				loop = false;
			}
		}
	}

	end_dialog(conf, shadow, widget, textpad);

	return (retval);
}

/* API */
int
bsddialog_msgbox(struct bsddialog_conf *conf, const char *text, int rows,
    int cols)
{
	struct buttons bs;

	get_buttons(conf, &bs, true, BUTTON_OK_LABEL, NULL);

	return (do_message(conf, text, rows, cols, bs));
}

int
bsddialog_yesno(struct bsddialog_conf *conf, const char *text, int rows,
    int cols)
{
	struct buttons bs;

	get_buttons(conf, &bs, true, "Yes", "No");

	return (do_message(conf, text, rows, cols, bs));
}
