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
#include <string.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#include <ncurses/form.h>
#else
#include <curses.h>
#include <form.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "bsddialog_theme.h"

/* "Message": msgbox - yesno */

#define HBORDERS	2
#define VBORDERS	2U
#define AUTO_WIDTH	(COLS / 3U)
/*
 * Min height = 5: 2 up & down borders + 2 label & up border buttons + 1 line
 * for text, at least 1 line is important for widget_withtextpad_init() to avoid
 * "Cannot build the pad window for text".
 */
#define MIN_HEIGHT	5

extern struct bsddialog_theme t;

//lib_util in the future
static int
set_widget_size(struct bsddialog_conf conf, int rows, int cols, int *h, int *w)
{
	int maxheight, maxwidth;

	if ((maxheight = conf.shadow ? LINES - t.shadowrows : LINES) <=0)
		RETURN_ERROR("Terminal too small, rows - shadow <= 0");

	if (rows == BSDDIALOG_FULLSCREEN)
		*h = maxheight;
	else if (rows < BSDDIALOG_FULLSCREEN)
		RETURN_ERROR("Negative (less than -1) height");
	else if (rows > BSDDIALOG_AUTOSIZE) {
		if ((*h = rows) > maxheight)
			RETURN_ERROR("Height too big (> terminal height - "\
			    "shadow");
	}
	/* rows == AUTOSIZE: each widget has to set its size */

	if ((maxwidth = conf.shadow ? COLS - t.shadowcols : COLS) <= 0)
		RETURN_ERROR("Terminal too small, cols - shadow <= 0");

	if (cols == BSDDIALOG_FULLSCREEN)
		*w = cols;
	else if (cols < BSDDIALOG_FULLSCREEN)
		RETURN_ERROR("Negative (less than -1) width");
	else if (cols > BSDDIALOG_AUTOSIZE) {
		if ((*w = cols) > maxwidth)
			RETURN_ERROR("Width too big (> terminal width - shadow)");
	}
	/* cols == AUTOSIZE: each widget has to set its size */

	return 0;
}

static int
message_autosize(struct bsddialog_conf conf, int rows, int cols, int *h, int *w,
    char *text, struct buttons bs)
{
	int maxword, maxline, nlines, line;

	if (get_text_properties(conf, text, &maxword, &maxline, &nlines) != 0)
		return BSDDIALOG_ERROR;

	if (cols == BSDDIALOG_AUTOSIZE) {
		*w = VBORDERS;
		/* buttons size */
		*w += bs.nbuttons * bs.sizebutton;
		*w += bs.nbuttons > 0 ? (bs.nbuttons-1) * t.buttonspace : 0;
		/* text size */
		line = MIN(maxline + VBORDERS + t.texthmargin * 2, AUTO_WIDTH);
		line = MAX(line, (int) (maxword + VBORDERS + t.texthmargin * 2));
		*w = MAX(*w, line);
		/* avoid terminal overflow */
		*w = MIN(*w, conf.shadow ? COLS - (int) t.shadowcols : COLS);
	}

	if (rows == BSDDIALOG_AUTOSIZE) {
		*h = MIN_HEIGHT - 1;
		if (maxword > 0)
			*h += MAX(nlines, (*w / 9)); /* aspect ratio: add conf/theme? */
		*h = MAX(*h, MIN_HEIGHT);
		/* avoid terminal overflow */
		*h = MIN(*h, conf.shadow ? LINES - (int) t.shadowrows : LINES);
	}

	return 0;
}

static int message_checksize(int rows, int cols, struct buttons bs)
{
	int mincols;

	mincols = VBORDERS;
	mincols += bs.nbuttons * bs.sizebutton;
	mincols += bs.nbuttons > 0 ? (bs.nbuttons-1) * t.buttonspace : 0;

	if (cols < mincols)
		RETURN_ERROR("Few cols, Msgbox and Yesno need at least width "\
		    "for nbuttons size and margin + borders");

	if (rows < MIN_HEIGHT)
		RETURN_ERROR("Msgbox and Yesno need at least height 5");

	return 0;
}

//lib_util in the future
static int
set_widget_position(struct bsddialog_conf conf, int *y, int *x, int rows,
    int cols, int h, int w)
{

	if (rows == BSDDIALOG_FULLSCREEN)
		*y = 0;
	else if (conf.y == BSDDIALOG_CENTER)
		*y = LINES/2 - h/2;
	else if (conf.y < BSDDIALOG_CENTER)
		RETURN_ERROR("Negative begin y (less than -1)");
	else if (conf.y >= LINES)
		RETURN_ERROR("Begin Y under the terminal");
	else
		*y = conf.y;

	if ((*y + h + (conf.shadow ? (int) t.shadowrows : 0)) > LINES)
		RETURN_ERROR("The lower of the box under the terminal "\
		    "(begin Y + height (+ shadow) > terminal lines)");


	if (cols == BSDDIALOG_FULLSCREEN)
		*x = 0;
	else if (conf.x == BSDDIALOG_CENTER)
		*x = COLS/2 - w/2;
	else if (conf.x < BSDDIALOG_CENTER)
		RETURN_ERROR("Negative begin x (less than -1)");
	else if (conf.x >= COLS)
		RETURN_ERROR("Begin X over the right of the terminal");
	else
		*x = conf.x;

	if ((*x + w + (conf.shadow ? (int) t.shadowcols : 0)) > COLS)
		RETURN_ERROR("The right of the box over the terminal "\
		    "(begin X + width (+ shadow) > terminal cols)");

	return 0;
}

static void
buttonsupdate(WINDOW *widget, int h, int w, struct buttons bs, bool shortkey)
{
	draw_buttons(widget, h-2, w, bs, shortkey);
	wrefresh(widget);
}

static void
textupdate(WINDOW *widget, int y, int x, int h, int w, WINDOW *textpad,
    int htextpad, int textrow)
{

	if (htextpad > h - 4) {
		mvwprintw(widget, h-3, w-6, "%3d%%", (int)((100 * (textrow+h-4)) / htextpad));
		wrefresh(widget);
	}

	prefresh(textpad, textrow, 0, y+1, x+2, y+h-4, x+w-2);
}

static int
do_widget(struct bsddialog_conf conf, char *text, int rows, int cols, char *name,
    struct buttons bs, bool shortkey)
{
	WINDOW *widget, *textpad, *shadow;
	bool loop;
	int i, y, x, h, w, input, output, htextpad, textrow;

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return BSDDIALOG_ERROR;
	if (message_autosize(conf, rows, cols, &h, &w, text, bs) != 0)
		return BSDDIALOG_ERROR;
	if (message_checksize(h, w, bs) != 0)
		return BSDDIALOG_ERROR;
	if (set_widget_position(conf, &y, &x, rows, cols, h, w) != 0)
		return BSDDIALOG_ERROR;

	htextpad = h - 4;
	if (widget_withtextpad_init(conf, &shadow, &widget, y, x, h, w, RAISED,
	    &textpad, &htextpad, text, true) != 0)
		return BSDDIALOG_ERROR;

	textrow = 0;
	loop = true;
	buttonsupdate(widget, h, w, bs, shortkey);
	textupdate(widget, y, x, h, w, textpad, htextpad, textrow);
	while(loop) {
		input = getch();
		switch (input) {
		case 10: /* Enter */
			output = bs.value[bs.curr];
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			buttonsupdate(widget, h, w, bs, shortkey);
			break;
		case KEY_F(1):
			if (conf.hfile == NULL)
				break;
			if (f1help(conf) != 0)
				return BSDDIALOG_ERROR;
			/* No break! the terminal size can change */
		case KEY_RESIZE: //to improve
		case 'r':
			/*BSDDIALOG_DEBUG(4,1,"LINES: %d, COLS: %d", LINES, COLS);
			BSDDIALOG_DEBUG(5,1,"old y: %d, old x: %d, old h: %d, old w: %d", y,x,h,w);

			BSDDIALOG_DEBUG(6,1,"Hide and new size and position",1);*/
			hide_widget(y, x, h, w,conf.shadow);

			/*
			 * Unnecessary, but, when the columns decrease the
			 * following "refresh" seem not work
			 */
			refresh();

			if (set_widget_size(conf, rows, cols, &h, &w) != 0)
				return BSDDIALOG_ERROR;
			if (message_autosize(conf, rows, cols, &h, &w, text, bs) != 0)
				return BSDDIALOG_ERROR;
			if (message_checksize(h, w, bs) != 0)
				return BSDDIALOG_ERROR;
			if (set_widget_position(conf, &y, &x, rows, cols, h, w) != 0)
				return BSDDIALOG_ERROR;
			//BSDDIALOG_DEBUG(7,1,"y: %d, x: %d, h: %d, w: %d", y,x,h,w);

			mvwin(shadow, y + t.shadowrows, x + t.shadowcols);
			wrefresh(shadow);

			mvwin(widget, y, x); /* refreshed by the following funcs*/
			buttonsupdate(widget, h, w, bs, shortkey);
			textupdate(widget, y, x, h, w, textpad, htextpad, textrow);
			break;
		case KEY_UP:
			if (textrow == 0)
				break;
			textrow--;
			textupdate(widget, y, x, h, w, textpad, htextpad, textrow);
			break;
		case KEY_DOWN:
			if (textrow + h - 4 >= htextpad)
				break;
			textrow++;
			textupdate(widget, y, x, h, w, textpad, htextpad, textrow);
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				buttonsupdate(widget, h, w, bs, shortkey);
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				buttonsupdate(widget, h, w, bs, shortkey);
			}
			break;
		default:
			if (shortkey == false)
				break;

			for (i = 0; i < (int) bs.nbuttons; i++)
				if (tolower(input) == tolower((bs.label[i])[0])) {
					output = bs.value[i];
					loop = false;
			}
		}
	}

	widget_withtextpad_end(conf, name, widget, h, w, textpad, shadow);

	return output;
}

/* API */

int
bsddialog_msgbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	struct buttons bs;

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), false, NULL, false, conf.help_button,
	    BUTTONLABEL(help_label));

	return (do_widget(conf, text, rows, cols, "msgbox", bs, true));
}

int
bsddialog_yesno(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	struct buttons bs;

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(yes_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(no_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	return (do_widget(conf, text, rows, cols, "yesno", bs, true));
}
