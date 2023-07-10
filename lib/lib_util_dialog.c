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
#include <unistd.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"
#include "lib_util_dialog.h"

/*
 * bsddialog_pause():
 * 	prepare_dialog()
 * 	set_buttons()
 * 	draw_pause()
 * 		dialog_set_size_position()
 * 		draw_dialog()
 * 			draw_box()
 * 			draw_buttons()
 * 			print_textpad()
 * 	while(getch())
 * 	destroy_dialog()
 */

#define draw2_buttons(d) draw_buttons(d->widget, d->bs)

static void
draw2_borders(struct bsddialog_conf *conf, WINDOW *win, enum elevation elev);

void
draw_box(struct bsddialog_conf *conf, WINDOW *win, int y, int x, int h, int w,
    enum elevation elev)
{
	wclear(win);
	mvwin(win, y, x);
	wresize(win, h, w);
	draw2_borders(conf, win, elev);
	wnoutrefresh(win);
	
}

int dialog_size_position(struct dialog *d ,int *htext, int hnotext, int minw)
{
	if (set_widget_size(d->conf, d->rows, d->cols, &d->h, &d->w) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_autosize(d->conf, d->rows, d->cols, &d->h, &d->w,
	    d->text, htext, &d->bs, hnotext, minw) != 0)
		return (BSDDIALOG_ERROR);
	if (widget_checksize(d->h, d->w, &d->bs, hnotext, minw) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(d->conf, &d->y, &d->x, d->h, d->w) != 0)
		return (BSDDIALOG_ERROR);

	return (0);
}

/* Destroy */
int hide2_dialog(struct dialog *d)
{
	WINDOW *clear;

	if ((clear = newwin(d->h, d->w, d->y, d->x)) == NULL)
		RETURN_ERROR("Cannot hide the widget");
	wbkgd(clear, t.screen.color);
	wrefresh(clear);

	if (d->conf->shadow) {
		mvwin(clear, d->y + t.shadow.y, d->x + t.shadow.x);
		wrefresh(clear);
	}

	delwin(clear);

	return (0);
}

void destroy_dialog(struct dialog *d)
{
	if (d->conf->sleep > 0)
		sleep(d->conf->sleep);

	delwin(d->textpad);
	delwin(d->widget);
	if (d->conf->shadow)
		delwin(d->shadow);

	if (d->conf->clear)
		hide2_dialog(d);

	if (d->conf->get_height != NULL)
		*d->conf->get_height = d->h;
	if (d->conf->get_width != NULL)
		*d->conf->get_width = d->w;
}

/* Update and draw */
static void
draw2_borders(struct bsddialog_conf *conf, WINDOW *win, enum elevation elev)
{
	int h, w;
	int leftcolor, rightcolor;
	int ls, rs, ts, bs, tl, tr, bl, br, ltee, rtee;

	if (conf->no_lines)
		return;

	getmaxyx(win, h, w);

	if (conf->ascii_lines) {
		ls = rs = '|';
		ts = bs = '-';
		tl = tr = bl = br = ltee = rtee = '+';
	} else {
		ls = rs = ACS_VLINE;
		ts = bs = ACS_HLINE;
		tl = ACS_ULCORNER;
		tr = ACS_URCORNER;
		bl = ACS_LLCORNER;
		br = ACS_LRCORNER;
		ltee = ACS_LTEE;
		rtee = ACS_RTEE;
	}

	leftcolor = (elev == RAISED) ?
	    t.dialog.lineraisecolor : t.dialog.linelowercolor;
	rightcolor = (elev == RAISED) ?
	    t.dialog.linelowercolor : t.dialog.lineraisecolor;
	wattron(win, leftcolor);
	wborder(win, ls, rs, ts, bs, tl, tr, bl, br);
	wattroff(win, leftcolor);

	wattron(win, rightcolor);
	mvwaddch(win, 0, h-1, tr);
	mvwvline(win, 1, h-1, rs, w-2);
	mvwaddch(win, w-1, h-1, br);
	mvwhline(win, w-1, 1, bs, h-2);
	wattroff(win, rightcolor);
}

/*
 * caller had to call dialog_set_size_position()
 * draw_dialog()
 * caller has to call pnoutrefresh(textpad) and doupdate() */
int draw_dialog(struct dialog *d)
{
	int wtitle, wbottomtitle, ts, ltee, rtee;

	ts = d->conf->ascii_lines ? '-' : ACS_HLINE;
	ltee = d->conf->ascii_lines ? '+' : ACS_LTEE;
	rtee = d->conf->ascii_lines ? '+' : ACS_RTEE;

	if (d->conf->shadow) {
		wclear(d->shadow);
		mvwin(d->shadow, d->y + t.shadow.y, d->x + t.shadow.x);
		wresize(d->shadow, d->h, d->w);
		wnoutrefresh(d->shadow);
	}

	wclear(d->widget);
	mvwin(d->widget, d->y, d->x);
	wresize(d->widget, d->h, d->w);
	draw2_borders(d->conf, d->widget, RAISED);

	if (d->conf->title != NULL) {
		if ((wtitle = strcols(d->conf->title)) < 0)
			return (BSDDIALOG_ERROR);
		if (t.dialog.delimtitle && d->conf->no_lines == false) {
			wattron(d->widget, t.dialog.lineraisecolor);
			mvwaddch(d->widget, 0, d->w/2 - wtitle/2 -1, rtee);
			wattroff(d->widget, t.dialog.lineraisecolor);
		}
		wattron(d->widget, t.dialog.titlecolor);
		mvwaddstr(d->widget, 0, d->w/2 - wtitle/2, d->conf->title);
		wattroff(d->widget, t.dialog.titlecolor);
		if (t.dialog.delimtitle && d->conf->no_lines == false)
			waddch(d->widget, ltee | t.dialog.lineraisecolor);
	}

	if (d->bs.nbuttons > 0) {
		if (d->conf->no_lines == false) {
			wattron(d->widget, t.dialog.lineraisecolor);
			mvwaddch(d->widget, d->h-3, 0, ltee);
			mvwhline(d->widget, d->h-3, 1, ts, d->w-2);
			wattroff(d->widget, t.dialog.lineraisecolor);

			wattron(d->widget, t.dialog.linelowercolor);
			mvwaddch(d->widget, d->h-3, d->w-1, rtee);
			wattroff(d->widget, t.dialog.linelowercolor);
		}
		draw2_buttons(d);
	}

	if (d->conf->bottomtitle != NULL) {
		if ((wbottomtitle = strcols(d->conf->bottomtitle)) < 0)
			return (BSDDIALOG_ERROR);
		wattron(d->widget, t.dialog.bottomtitlecolor);
		wmove(d->widget, d->h - 1, d->w/2 - wbottomtitle/2 - 1);
		waddch(d->widget, ' ');
		waddstr(d->widget, d->conf->bottomtitle);
		waddch(d->widget, ' ');
		wattroff(d->widget, t.dialog.bottomtitlecolor);
	}

	wnoutrefresh(d->widget);

	wclear(d->textpad);
	wresize(d->textpad, 1, d->w - BORDERS - TEXTHMARGINS); /*MIN(1) for infobox 0 w*/
	if (print_textpad(d->conf, d->textpad, d->text) != 0)
		return (BSDDIALOG_ERROR);

	d->built = true;

	return (0);
}

/* Init and alloc */
int
prepare_dialog(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, struct dialog *d)
{
	CHECK_PTR(conf, struct bsddialog_conf);
	CHECK_PTR_SIZE(conf->get_height, int);
	CHECK_PTR_SIZE(conf->get_width, int);

	d->built = false;
	d->conf = conf;
	d->rows = rows;
	d->cols = cols;
	d->text = (text == NULL) ? "" : text;
	// htext? bool withhtext, mixedgauge
	d->bs.nbuttons = 0; /* caller has to set_buttons() */

	if (d->conf->shadow) {
		if ((d->shadow = newwin(1, 1, 1, 1)) == NULL)
			RETURN_ERROR("Cannot build WINDOW shadow");
		wbkgd(d->shadow, t.shadow.color);
	}

	if ((d->widget = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW widget");
	wbkgd(d->widget, t.dialog.color);

	/* fake for textbox */
	if ((d->textpad = newpad(1, 1)) == NULL)
		RETURN_ERROR("Cannot build the pad window for text");
	wbkgd(d->textpad, t.dialog.color);

	return (0);
}