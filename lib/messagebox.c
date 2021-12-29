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

#ifdef PORTNCURSES
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define AUTO_WIDTH	(COLS / 3U)
#define MIN_HEIGHT	(HBORDERS + 2 /*buttons*/ + 1 /*text*/)

extern struct bsddialog_theme t;

static int
message_autosize(struct bsddialog_conf *conf, int rows, int cols, int *h,
    int *w, char *text, struct buttons bs)
{
	int maxword, maxline, nlines, line;

	if (get_text_properties(conf, text, &maxword, &maxline, &nlines) != 0)
		return BSDDIALOG_ERROR;

	if (cols == BSDDIALOG_AUTOSIZE) {
		line = MIN(maxline + t.text.hmargin * 2, AUTO_WIDTH);
		line = MAX(line, (int)(maxword + t.text.hmargin * 2));
		*w = widget_min_width(conf, &bs, line);
	}

	if (rows == BSDDIALOG_AUTOSIZE) {
		*h = 1;
		if (maxword > 0)
			*h = MAX(nlines, (int)(*w / GET_ASPECT_RATIO(conf)));
		*h = widget_min_height(conf, true, *h);
	}

	return 0;
}

static int message_checksize(int rows, int cols, struct buttons bs)
{
	int mincols;

	mincols = VBORDERS;
	mincols += bs.nbuttons * bs.sizebutton;
	mincols += bs.nbuttons > 0 ? (bs.nbuttons-1) * t.button.space : 0;

	if (cols < mincols)
		RETURN_ERROR("Few cols, Msgbox and Yesno need at least width "
		    "for borders, buttons and spaces between buttons");

	if (rows < MIN_HEIGHT)
		RETURN_ERROR("Msgbox and Yesno need at least height 5");

	return 0;
}

static void
textupdate(WINDOW *widget, WINDOW *textpad, int htextpad, int ytextpad)
{
	int y, x, h, w;

	getbegyx(widget, y, x);
	getmaxyx(widget, h, w);

	if (htextpad > h - 4) {
		mvwprintw(widget, h-3, w-6, "%3d%%",
		    100 * (ytextpad+h-4)/ htextpad);
		wnoutrefresh(widget);
	}

	pnoutrefresh(textpad, ytextpad, 0, y+1, x+2, y+h-4, x+w-2);
}

static int
do_message(struct bsddialog_conf *conf, char *text, int rows, int cols,
    struct buttons bs)
{
	bool loop;
	int y, x, h, w, input, output, ytextpad, htextpad, unused;
	WINDOW *widget, *textpad, *shadow;

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return BSDDIALOG_ERROR;
	if (message_autosize(conf, rows, cols, &h, &w, text, bs) != 0)
		return BSDDIALOG_ERROR;
	if (message_checksize(h, w, bs) != 0)
		return BSDDIALOG_ERROR;
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return BSDDIALOG_ERROR;

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs,
	    true) != 0)
		return BSDDIALOG_ERROR;

	ytextpad = 0;
	getmaxyx(textpad, htextpad, unused);
	unused++; /* fix unused error */
	textupdate(widget, textpad, htextpad, ytextpad);
	loop = true;
	while(loop) {
		doupdate();
		input = getch();
		switch (input) {
		case KEY_ENTER:
		case 10: /* Enter */
			output = bs.value[bs.curr];
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ESC;
			loop = false;
			break;
		case KEY_LEFT:
		case KEY_RIGHT:
		case '\t': /* TAB */
			if (move_button(input, &bs)) {
				draw_buttons(widget, bs, true);
				wnoutrefresh(widget);
			}
			break;
		case KEY_F(1):
			if (conf->f1_file == NULL && conf->f1_message == NULL)
				break;
			if (f1help(conf) != 0)
				return BSDDIALOG_ERROR;
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_widget(y, x, h, w, conf->shadow);
			refresh();

			if (set_widget_size(conf, rows, cols, &h, &w) != 0)
				return BSDDIALOG_ERROR;
			if (message_autosize(conf, rows, cols, &h, &w, text,
			    bs) != 0)
				return BSDDIALOG_ERROR;
			if (message_checksize(h, w, bs) != 0)
				return BSDDIALOG_ERROR;
			if (set_widget_position(conf, &y, &x, h, w) != 0)
				return BSDDIALOG_ERROR;

			if(update_dialog(conf, shadow, widget, y, x, h, w, 
			    textpad, text, &bs, true) != 0)
				return BSDDIALOG_ERROR;

			getmaxyx(textpad, htextpad, unused);
			textupdate(widget, textpad, htextpad, ytextpad);

			/* Important to fix grey lines expanding screen */
			refresh();
			break;
		case KEY_UP:
			if (ytextpad == 0)
				break;
			ytextpad--;
			textupdate(widget, textpad, htextpad, ytextpad);
			break;
		case KEY_DOWN:
			if (ytextpad + h - 4 >= htextpad)
				break;
			ytextpad++;
			textupdate(widget, textpad, htextpad, ytextpad);
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				output = bs.value[bs.curr];
				loop = false;
			}
		}
	}

	end_dialog(conf, shadow, widget, textpad);

	return output;
}

/* API */
int
bsddialog_msgbox(struct bsddialog_conf *conf, char* text, int rows, int cols)
{
	struct buttons bs;

	get_buttons(conf, &bs, BUTTON_OK_LABEL, NULL);

	return (do_message(conf, text, rows, cols, bs));
}

int
bsddialog_yesno(struct bsddialog_conf *conf, char* text, int rows, int cols)
{
	struct buttons bs;

	get_buttons(conf, &bs, BUTTON_YES_LABEL, BUTTON_NO_LABEL);

	return (do_message(conf, text, rows, cols, bs));
}
