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

#include <string.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "theme.h"

#define TREESPACE 4

extern struct bsddialog_theme t;

enum menumode {
	BUILDLISTMODE,
	CHECKLISTMODE,
	MENUMODE,
	RADIOLISTMODE,
	TREEVIEWMODE
};

struct positionlen {
	unsigned int prefixlen;
	unsigned int selectorlen;
	unsigned int maxdepth;
	unsigned int namelen;
	unsigned int desclen;
};

void
draw_myitem(WINDOW *pad, int y, struct bsddialog_menuitem item, enum menumode mode,
    int xdesc, bool curr, bool bottomdesc)
{
	int color, colorname;

	color = curr ? t.curritemcolor : t.itemcolor;
	colorname = curr ? t.currtagcolor : t.tagcolor;

	wmove(pad, y, 0);
	wattron(pad, color);
	if (mode == CHECKLISTMODE)
		wprintw(pad, "[%c]", item.on ? 'X' : ' ');
	if (mode == RADIOLISTMODE || mode == TREEVIEWMODE)
		wprintw(pad, "(%c)", item.on ? '*' : ' ');
	wattroff(pad, color);

	if (mode != BUILDLISTMODE && mode != TREEVIEWMODE) {
		wattron(pad, colorname);
		if (mode != MENUMODE)
			wmove(pad, y, 4);
		waddstr(pad, item.name);
		wattroff(pad, colorname);
	}

	if ((mode == BUILDLISTMODE || mode == TREEVIEWMODE) && curr == false)
		color = item.on ? t.tagcolor : t.itemcolor;
	wattron(pad, color);
	if (mode == CHECKLISTMODE || mode == RADIOLISTMODE || mode == TREEVIEWMODE)
		xdesc +=4;
	if (mode == TREEVIEWMODE)
		xdesc = xdesc + item.depth * TREESPACE;
	mvwaddstr(pad, y, xdesc, item.desc);
	wattroff(pad, color);

	if (curr && bottomdesc == true) {
		move(LINES-2, 5);
		clrtoeol();
		addstr(item.bottomdesc);
		
		refresh();
	}
}

int
do_menu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int line, int xdesc, enum menumode mode,
    int nitems, struct bsddialog_menuitem *items)
{
	WINDOW *widget, *button, *menuwin, *menupad, *shadow;
	char *sepstr, quotech;
	int i, output, y, x, input, curr;
	int ys, ye, xs, xe;
	bool loop, buttupdate, sep;
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true, &button) <0)
		return -1;

	menuwin = new_window(y + rows - 5 - menurows, x + 2, menurows+2, cols-4,
	    NULL, NULL, conf.no_lines ? NOLINES : LOWERED,
	    conf.ascii_lines, false);

	menupad = newpad(nitems, line);
	wbkgd(menupad, t.widgetcolor);

	curr = -1;
	if (conf.default_item != NULL) {
		for (i=0; i<nitems; i++) {
			if (strcmp(items[i].name, conf.default_item) == 0) {
				curr = i;
				break;
			}
		}
	}
	curr = curr < 0 ? 0 : curr;
	for (i=0; i<nitems; i++)
		draw_myitem(menupad, i, items[i], mode, xdesc, i == curr, conf.item_help);

	ys = y + rows - 5 - menurows + 1;
	ye = ys + menurows + 2 -1;
	xs = (line > cols - 6) ? (x + 2 + 1) : x + 3 + (cols-6)/2 - line/2;
	xe = (line > cols - 6) ? xs + cols - 7 : xs + cols - 4 -1;
	if (mode == TREEVIEWMODE)
		xs = x + 2 + 1;

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	wrefresh(menuwin);
	prefresh(menupad, 0, 0, ys, xs, ye, xe);//delete?

	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, bs, true);
			wrefresh(button);
			buttupdate = false;
		}
		//wrefresh(menuwin);
		prefresh(menupad, 0, 0, ys, xs, ye, xe);

		input = getch();
		switch(input) {
		case 10: // Enter
			output = bs.value[bs.curr]; // -> buttvalues[selbutton]
			loop = false;
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
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
			if (bs.curr < bs.nbuttons - 1) {
				bs.curr++;
				buttupdate = true;
			}
			break;
		}

		if (nitems <= 0)
			continue;

		switch(input) {
		case KEY_UP:
			draw_myitem(menupad, curr, items[curr], mode, xdesc, false, conf.item_help);
			curr = (curr > 0) ? curr - 1 : 0;
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true, conf.item_help);
			break;
		case KEY_DOWN:
			draw_myitem(menupad, curr, items[curr], mode, xdesc, false, conf.item_help);
			curr = (curr < nitems-1) ? curr +1 : nitems-1;
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true, conf.item_help);
			break;
		case ' ': /* Space */
			if (mode == MENUMODE)
				break;
			else if (mode == CHECKLISTMODE)
				items[curr].on = ! items[curr].on;
			else { //RADIOLISTMODE and TREEVIEWMODE
				if (items[curr].on == true)
					break;
				for (i=0; i<nitems; i++)
					if (items[i].on == true) {
						items[i].on = false;
						draw_myitem(menupad, i, items[i],
						    mode, xdesc, false, conf.item_help);
					}
				items[curr].on = true;
			}
			draw_myitem(menupad, curr, items[curr], mode, xdesc, true, conf.item_help);
			break;
		default:
			
			break;
		}
	}

	sep = false;
	quotech = conf.single_quoted ? '\'' : '"';

	if (output == BSDDIALOG_HELP && nitems >0) {
		dprintf(conf.output_fd, "HELP %s", items[curr].name);
		sep = true;
	}

	sepstr = conf.separate_output ? "\n" : " ";

	if ((output == BSDDIALOG_YESOK || conf.help_status == true) && nitems > 0) {
		if (mode == MENUMODE) {
			dprintf(conf.output_fd, "%s", items[curr].name);
			items[curr].on = true; // for library
		} else { /* CHECKLIST or RADIOLIST */
			for (i=0; i<nitems; i++) {
				if (items[i].on == true) {
					if (sep == true)
					    dprintf(conf.output_fd, "%s", sepstr);
					sep = true;
					dprintf(conf.output_fd, "%s",items[i].name);
				}
			}
		}
	}

	delwin(menupad);
	delwin(menuwin);
	widget_end(conf, "MenuToFix", widget, rows, cols, shadow, button);

	return output;
}

int
bsddialog_checklist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxname, maxdesc;

	line = maxname = maxdesc = 0;
	for (i=0; i<nitems; i++) {
		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc + 4);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    CHECKLISTMODE, nitems, items);

	return output;
}

int
bsddialog_menu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxname, maxdesc;

	line = maxname = maxdesc = 0;
	for (i=0; i<nitems; i++) {
		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    MENUMODE, nitems, items);

	return output;
}

int
bsddialog_radiolist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxname, maxdesc;
	bool on = false;

	line = maxname = maxdesc = 0;
	for (i=0; i<nitems; i++) {
		if (on == true)
			items[i].on = false;

		if (items[i].on == true)
			on = true;

		maxname = MAX(maxname, strlen(items[i].name) + 1);
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxname + maxdesc + 4);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, maxname,
	    RADIOLISTMODE, nitems, items);

	return output;
}

int
bsddialog_treeview(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line, maxdesc;
	bool on = false;

	line = maxdesc = 0;
	for (i=0; i<nitems; i++) {
		if (on == true)
			items[i].on = false;

		if (items[i].on == true)
			on = true;
		maxdesc = MAX(maxdesc, strlen(items[i].desc));
		line = MAX(line, maxdesc + 4 + items[i].depth * TREESPACE);
	}

	output = do_menu(conf, text, rows, cols, menurows, line, 0,
	    TREEVIEWMODE, nitems, items);

	return output;
}

int bsddialog_mixedmenu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int ngroups, struct bsddialog_menugroup *groups)
{
	int i, j;
	bool on;
	struct positionlen poslen = { 0, 0, 0, 0 };
	struct bsddialog_menuitem item;

	for (i=0; i<ngroups; i++) {
		on = false;
		if (groups[i].type == BSDDIALOG_RADIOLIST ||
		    groups[i].type == BSDDIALOG_CHECKLIST)
			poslen.selectorlen = 3;
		for (j=0; j<groups[i].nitems; j++) {
			item = groups[i].items[j];
			if (groups[i].type == BSDDIALOG_RADIOLIST) {
				if (on == true)
					item.on = false;

				if (item.on == true)
					on = true;
			}

			if (conf.item_prefix)
				poslen.prefixlen = MAX(poslen.prefixlen, strlen(item.bottomdesc));

			poslen.namelen = MAX(poslen.namelen, strlen(item.name) + 1);
			poslen.desclen = MAX(poslen.desclen, strlen(item.desc));
		}
	}

	return BSDDIALOG_ERROR;
}

int
do_buildlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int line, int nitems, struct bsddialog_menuitem *items,
    bool startleft)
{
	WINDOW *widget, *button, *leftwin, *leftpad, *rightwin, *rightpad, *shadow;
	int output, i, x, y, input;
	bool loop, buttupdate, padsupdate;
	int nlefts, nrights, leftwinx, rightwinx, winsy, padscols, curr;
	enum side {LEFT, RIGHT} currV;
	int currH;
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true, &button) <0)
		return -1;

	winsy = y + rows - 5 - menurows;
	leftwinx = x+2;
	leftwin = new_window(winsy, leftwinx, menurows+2, (cols-5)/2, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);
	rightwinx = x + cols - 2 -(cols-5)/2;
	rightwin = new_window(winsy, rightwinx, menurows+2, (cols-5)/2, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false);

	wrefresh(leftwin);
	wrefresh(rightwin);

	padscols = (cols-5)/2 - 2;
	leftpad  = newpad(nitems, line);
	rightpad = newpad(nitems, line);
	wbkgd(leftpad, t.widgetcolor);
	wbkgd(rightpad, t.widgetcolor);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	currH = 0;
	currV = startleft ? LEFT : RIGHT;
	loop = buttupdate = padsupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(button, cols, bs, true);
			wrefresh(button);
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
					draw_myitem(leftpad, nlefts, items[i],
					    BUILDLISTMODE, 0, curr == i, conf.item_help);
					nlefts++;
				} else {
					if (currV == RIGHT && currH == nrights)
						curr = i;
					draw_myitem(rightpad, nrights, items[i],
					    BUILDLISTMODE, 0, curr == i, conf.item_help);
					nrights++;
				}
			}
			prefresh(leftpad, 0, 0, winsy+1, leftwinx+1, winsy+1+menurows, leftwinx + 1 + padscols);
			prefresh(rightpad, 0, 0, winsy+1, rightwinx+1, winsy+1+menurows, rightwinx + 1 + padscols);
			padsupdate = false;
		}

		input = getch();
		switch(input) {
		case 10: // Enter
			output = bs.value[bs.curr]; // -> buttvalues[selbutton]
			loop = false;
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
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
		case ' ': // Space
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

			break;
		}
	}

	delwin(leftpad);
	delwin(leftwin);
	delwin(rightpad);
	delwin(rightwin);
	widget_end(conf, "Buildlist", widget, rows, cols, shadow, button);

	return output;
}

int
bsddialog_buildlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int i, output, line;
	bool startleft = false;

	line = 0;
	for (i=0; i<nitems; i++) {
		line = MAX(line, strlen(items[i].desc));
		if (items[i].on == false)
			startleft = true;
	}

	output = do_buildlist(conf, text, rows, cols, menurows, line, nitems,
	    items, startleft);

	return output;
}
