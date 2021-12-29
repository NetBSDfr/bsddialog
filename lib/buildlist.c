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
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define DEPTHSPACE	4

extern struct bsddialog_theme t;

struct lineposition {
	unsigned int maxsepstr;
	unsigned int maxprefix;
	unsigned int xselector;
	unsigned int selectorlen;
	unsigned int maxdepth;
	unsigned int xname;
	unsigned int maxname;
	unsigned int xdesc;
	unsigned int maxdesc;
	unsigned int line;
};

static void
drawitem(struct bsddialog_conf *conf, WINDOW *pad, int y, 
    struct bsddialog_menuitem item, struct lineposition pos, bool curr)
{
	int colordesc;
	/* prefix */
	if (item.prefix != NULL && item.prefix[0] != '\0')
		mvwaddstr(pad, y, 0, item.prefix);

	/* description */
	if (curr == false)
		colordesc = item.on ? t.menu.namecolor : t.menu.desccolor;
	else
		colordesc = t.menu.f_namecolor;

	if (conf->menu.no_desc == false) {
		wattron(pad, colordesc);
		if (conf->menu.no_name)
			mvwaddstr(pad, y, pos.xname + item.depth * DEPTHSPACE,
			    item.desc);
		else
			mvwaddstr(pad, y, pos.xdesc, item.desc);
		wattroff(pad, colordesc);
	}

	/* bottom description */
	move(LINES-1, 2);
	clrtoeol();
	if (item.bottomdesc != NULL) {
		addstr(item.bottomdesc);
		refresh();
	}
}

/* todo */
static int buildlist_autosize(int rows, int cols)
{

	if (cols == BSDDIALOG_AUTOSIZE)
		RETURN_ERROR("Unimplemented cols autosize for buildlist");

	if (rows == BSDDIALOG_AUTOSIZE)
		RETURN_ERROR("Unimplemented rows autosize for buildlist");

	return 0;
}

/* to improve */
static int
buildlist_checksize(int rows, int cols, char *text, int menurows, int nitems,
    struct buttons bs)
{
	int mincols, textrow, menusize;

	mincols = VBORDERS;
	/* buttons */
	mincols += bs.nbuttons * bs.sizebutton;
	mincols += bs.nbuttons > 0 ? (bs.nbuttons-1) * t.button.space : 0;
	/* line, comment to permet some cols hidden */
	/* mincols = MAX(mincols, linelen); */

	if (cols < mincols)
		RETURN_ERROR("Few cols, width < size buttons or "
		    "name+descripion of the items");

	textrow = text != NULL && strlen(text) > 0 ? 1 : 0;

	if (nitems > 0 && menurows == 0)
		RETURN_ERROR("items > 0 but menurows == 0, probably terminal "
		    "too small");

	menusize = nitems > 0 ? 3 : 0;
	if (rows < 2 + 2 + menusize + textrow)
		RETURN_ERROR("Few lines for this menus");

	return 0;
}

int
bsddialog_buildlist(struct bsddialog_conf *conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items,
    int *focusitem)
{
	WINDOW *widget, *textpad, *shadow;
	WINDOW *leftwin, *leftpad, *rightwin, *rightpad;
	int output, i, x, y, h, w, input;
	bool loop, buttupdate, padsupdate, startleft;
	int nlefts, nrights, leftwinx, rightwinx, winsy, padscols, curr;
	enum side {LEFT, RIGHT} currV;
	int currH;
	struct buttons bs;
	struct lineposition pos = {0,0,0,0,0,0,0,0,0,0};

	startleft = false;
	for (i=0; i<nitems; i++) {
		pos.line = MAX(pos.line, strlen(items[i].desc));
		if (items[i].on == false)
			startleft = true;
	}

	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return BSDDIALOG_ERROR;
	if (buildlist_autosize(rows, cols) != 0)
		return BSDDIALOG_ERROR;
	if (buildlist_checksize(h, w, text, menurows, nitems, bs) != 0)
		return BSDDIALOG_ERROR;
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return BSDDIALOG_ERROR;

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text,
	    &bs, true) != 0)
		return BSDDIALOG_ERROR;

	doupdate();

	prefresh(textpad, 0, 0, y + 1, x + 1 + t.text.hmargin,
	    y + h - menurows, x + 1 + w - t.text.hmargin);

	winsy = y + h - 5 - menurows;
	leftwinx = x+2;
	leftwin = new_boxed_window(conf, winsy, leftwinx, menurows+2, (w-5)/2,
	    LOWERED);
	rightwinx = x + w - 2 -(w-5)/2;
	rightwin = new_boxed_window(conf, winsy, rightwinx, menurows+2,
	    (w-5)/2, LOWERED);

	wrefresh(leftwin);
	wrefresh(rightwin);

	padscols = (w-5)/2 - 2;
	leftpad  = newpad(nitems, pos.line);
	rightpad = newpad(nitems, pos.line);
	wbkgd(leftpad, t.dialog.color);
	wbkgd(rightpad, t.dialog.color);

	currH = 0;
	currV = startleft ? LEFT : RIGHT;
	loop = buttupdate = padsupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}

		if (padsupdate) {
			werase(leftpad);
			werase(rightpad);
			curr = -1;
			nlefts = nrights = 0;
			for (i=0; i<nitems; i++) {
				if (items[i].on == false) {
					if (currV == LEFT && currH == nlefts)
						curr = i;
					drawitem(conf, leftpad, nlefts,
					    items[i], pos, curr == i);
					nlefts++;
				} else {
					if (currV == RIGHT && currH == nrights)
						curr = i;
					drawitem(conf, rightpad, nrights,
					    items[i], pos, curr == i);
					nrights++;
				}
			}
			prefresh(leftpad, 0, 0, winsy+1, leftwinx+1,
			    winsy+1+menurows, leftwinx + 1 + padscols);
			prefresh(rightpad, 0, 0, winsy+1, rightwinx+1,
			    winsy+1+menurows, rightwinx + 1 + padscols);
			padsupdate = false;
		}

		input = getch();
		switch(input) {
		case KEY_ENTER:
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
			buttupdate = true;
			break;
		}

		if (nitems <= 0)
			continue;

		switch(input) {
		case KEY_LEFT:
			if (currV == RIGHT && nrights > 0) {
				currV = LEFT;
				currH = 0;
				padsupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (currV == LEFT && nrights > 0) {
				currV = RIGHT;
				currH = 0;
				padsupdate = true;
			}
			break;
		case KEY_UP:
			currH = (currH > 0) ? currH - 1 : 0;
			padsupdate = true;
			break;
		case KEY_DOWN:
			if (currV == LEFT)
				currH = (currH < nlefts-1) ? currH +1 : currH;
			else
				currH = (currH < nrights-1)? currH +1 : currH;
			padsupdate = true;
			break;
		case ' ': /* Space */
			items[curr].on = ! items[curr].on;
			if (currV == LEFT) {
				if (nlefts > 1)
					currH = currH > 0 ? currH-1 : 0;
				else {
					currH = 0;
					currV = RIGHT;
				}
			} else {
				if (nrights > 1)
					currH = currH > 0 ? currH-1 : 0;
				else {
					currH = 0;
					currV = LEFT;
				}
			}
			padsupdate = true;
			break;
		default:
			if (shortcut_buttons(input, &bs)) {
				output = bs.value[bs.curr];
				loop = false;
			}
		}
	}

	if(focusitem != NULL)
		*focusitem = curr;

	delwin(leftpad);
	delwin(leftwin);
	delwin(rightpad);
	delwin(rightwin);
	end_dialog(conf, shadow, widget, textpad);

	return output;
}
