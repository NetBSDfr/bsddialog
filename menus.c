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
	MIXEDLISTMODE,
	RADIOLISTMODE,
	SEPARATORMODE,
	TREEVIEWMODE
};

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

int checkradiolist(int nitems, struct bsddialog_menuitem *items)
{
	int i, error;

	error = 0;
	for (i=0; i<nitems; i++) {
		if (error > 0)
			items[i].on = false;

		if (items[i].on == true)
			error++;
	}

	return (error == 0 ? 0 : -1);
}

int checkmenu(int nitems, struct bsddialog_menuitem *items) // can change
{
	int i, error;

	error = 0;
	for (i=0; i<nitems; i++) {
		if (items[i].on == true)
			error++;

		items[i].on = false;
	}

	return (error == 0 ? 0 : -1);
}

void
getfirst(int ngroups, struct bsddialog_menugroup *groups, int *abs, int *group,
    int *rel)
{
	int i, a;

	*abs = *rel = *group = -1;
	a = 0;
	for (i=0; i<ngroups; i++) {
		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			a += groups[i].nitems;
			continue;
		}
		if (groups[i].nitems != 0) {
			*group = i;
			*abs = a + *abs + 1;
			*rel = 0;
			break;
		}
	}
}

void
getnext(int ngroups, struct bsddialog_menugroup *groups, int *abs, int *group,
    int *rel)
{
	int i, a;

	if (*abs < 0 || *group < 0 || *rel < 0)
		return;

	if (*rel + 1 < groups[*group].nitems) {
		*rel = *rel + 1;
		*abs = *abs + 1;
		return;
	}

	if (*group + 1 > ngroups)
		return;

	a = *abs;
	for (i = *group + 1; i < ngroups; i++) {
		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			a += groups[i].nitems;
			continue;
		}
		if (groups[i].nitems != 0) {
			*group = i;
			*abs = a + 1;
			*rel = 0;
			break;
		}
	}
}
void
getprev(int ngroups, struct bsddialog_menugroup *groups, int *abs, int *group,
    int *rel)
{
	int i, a;

	if (*abs < 0 || *group < 0 || *rel < 0)
		return;

	if (*rel > 0) {
		*rel = *rel - 1;
		*abs = *abs - 1;
		return;
	}

	if (*group - 1 < 0)
		return;

	a = *abs;
	for (i = *group - 1; i >= 0; i--) {
		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			a -= groups[i].nitems;
			continue;
		}
		if (groups[i].nitems != 0) {
			*group = i;
			*abs = a - 1;
			*rel = groups[i].nitems - 1;
			break;
		}
	}
}

enum menumode getmode(enum menumode mode, struct bsddialog_menugroup group)
{

	if (mode == MIXEDLISTMODE) {
		if (group.type == BSDDIALOG_SEPARATOR)
			mode = SEPARATORMODE;
		else if (group.type == BSDDIALOG_RADIOLIST)
			mode = RADIOLISTMODE;
		else if (group.type == BSDDIALOG_CHECKLIST)
			mode = CHECKLISTMODE;
	}

	return mode;
}

void
draw_myitem(struct config conf, WINDOW *pad, int y,
    struct bsddialog_menuitem item, enum menumode mode, struct lineposition pos,
    bool curr)
{
	int color, colorname, linech;

	color = curr ? t.curritemcolor : t.itemcolor;
	colorname = curr ? t.currtagcolor : t.tagcolor;

	if (mode == SEPARATORMODE) {
		if (conf.no_lines == false) {
			wattron(pad, t.itemcolor);
			linech = conf.ascii_lines ? '-' : ACS_HLINE;
			mvwhline(pad, y, 0, linech, pos.line);
			wattroff(pad, t.itemcolor);
		}
		wmove(pad, y, pos.line/2 - (strlen(item.name)+strlen(item.desc))/2);
		wattron(pad, t.namesepcolor);
		waddstr(pad, item.name);
		wattroff(pad, t.namesepcolor);
		if (strlen(item.name) > 0 && strlen(item.desc) > 0)
			waddch(pad, ' ');
		wattron(pad, t.descsepcolor);
		waddstr(pad, item.desc);
		wattroff(pad, t.descsepcolor);
		return;
	}

	/* prefix */
	if (conf.item_prefix == true)
		mvwaddstr(pad, y, 0, item.bottomdesc);

	/* selector */
	wmove(pad, y, pos.xselector);
	wattron(pad, color);
	if (mode == CHECKLISTMODE)
		wprintw(pad, "[%c]", item.on ? 'X' : ' ');
	if (mode == RADIOLISTMODE || mode == TREEVIEWMODE)
		wprintw(pad, "(%c)", item.on ? '*' : ' ');
	wattroff(pad, color);

	/* name */
	if (mode != BUILDLISTMODE && mode != TREEVIEWMODE) {
		wattron(pad, colorname);
		mvwaddstr(pad, y, pos.xname, item.name);
		wattroff(pad, colorname);
	}

	/* description */
	if ((mode == BUILDLISTMODE || mode == TREEVIEWMODE) && curr == false)
		color = item.on ? t.tagcolor : t.itemcolor;
	wattron(pad, color);
	if (mode == TREEVIEWMODE)
		pos.xdesc = pos.xname + item.depth * TREESPACE;
	mvwaddstr(pad, y, pos.xdesc, item.desc);
	wattroff(pad, color);

	/* bottom desc (item help) */
	if (curr && conf.item_help == true) {
		move(LINES-2, 5);
		clrtoeol();
		addstr(item.bottomdesc);

		refresh();
	}
}

void
print_selected_list(struct config conf, int output, enum menumode mode,
    int ngroups, struct bsddialog_menugroup *groups, int g, int rel)
{
	int i, j;
	bool sep;
	char *sepstr, quotech;
	enum menumode currmode;
	struct bsddialog_menuitem *item;

	item = &groups[g].items[rel];
	sep = false;

	if (output == BSDDIALOG_HELP) {
		dprintf(conf.output_fd, "HELP %s", item->name);
		sep = true;
	}

	quotech = conf.single_quoted ? '\'' : '"';
	sepstr = conf.separate_output ? "\n" : " ";

	if (output != BSDDIALOG_YESOK && conf.help_status == false)
		return;

	currmode = getmode(mode, groups[g]);

	if (currmode == MENUMODE) {
		dprintf(conf.output_fd, "%s", item->name);
		return;
	}
	/* else Lists */
	for (i=0; i < ngroups; i++) {
		for (j=0; j < groups[i].nitems; j++) {
			item = &groups[i].items[j];
			if (groups[i].type == BSDDIALOG_SEPARATOR)
				dprintf(conf.output_fd, "--%s %s--\n",
				    item->name, item->desc);
			if (item->on == true) {
				if (sep == true)
					dprintf(conf.output_fd, "%s", sepstr);
				sep = true;
				if (strchr(item->name, ' ') != NULL)
					dprintf(conf.output_fd, "%c", quotech);
				dprintf(conf.output_fd, "%s",item->name);
				if (strchr(item->name, ' ') != NULL)
					dprintf(conf.output_fd, "%c", quotech);
			}
		}
	}
}

int
do_mixedlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, char *namewidget, enum menumode mode, int ngroups,
    struct bsddialog_menugroup *groups)
{
	WINDOW *widget, *button, *menuwin, *menupad, *shadow;
	int i, j, output, input;
	int y, x, ys, ye, xs, xe, abs, g, rel, totnitems;
	bool loop, buttupdate;
	struct buttons bs;
	struct bsddialog_menuitem *item;
	enum menumode currmode;
	struct lineposition pos = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true, &button) <0)
		return -1;

	menuwin = new_window(y + rows - 5 - menurows, x + 2, menurows+2, cols-4,
	    NULL, NULL, conf.no_lines ? NOLINES : LOWERED,
	    conf.ascii_lines, false);


	totnitems = 0;
	for (i=0; i < ngroups; i++) {
		currmode = getmode(mode, groups[i]);
		if (currmode == RADIOLISTMODE || currmode == TREEVIEWMODE)
			checkradiolist(groups[i].nitems, groups[i].items);

		if (currmode == MENUMODE)
			checkmenu(groups[i].nitems, groups[i].items);

		if (currmode == RADIOLISTMODE || currmode == CHECKLISTMODE || currmode == TREEVIEWMODE)
			pos.selectorlen = 3;

		for (j=0; j < groups[i].nitems; j++) {
			totnitems++;
			item = &groups[i].items[j];

			if (groups[i].type == BSDDIALOG_SEPARATOR) {
				pos.maxsepstr = MAX(pos.maxsepstr, strlen(item->name) + strlen(item->desc));
				continue;
			}

			if (currmode == TREEVIEWMODE) {
				pos.maxdepth = MAX(pos.maxdepth, item->depth);
			}

			if (conf.item_prefix && item->bottomdesc != NULL)
				pos.maxprefix = MAX(pos.maxprefix, strlen(item->bottomdesc));

			pos.maxname = MAX(pos.maxname, strlen(item->name));
			pos.maxdesc = MAX(pos.maxdesc, strlen(item->desc));
		}
	}

	pos.xselector = pos.maxprefix + (pos.maxprefix != 0 ? 1 : 0);
	pos.xname = pos.xselector + pos.selectorlen + (pos.selectorlen != 0 ? 1 : 0);
	pos.xdesc = pos.xname + pos.maxname + (pos.maxname != 0 ? 1 : 0);
	if (currmode == TREEVIEWMODE)
		pos.line = MAX(pos.maxsepstr + 3, pos.xname + pos.maxdepth * TREESPACE + pos.maxdesc );
	else
		pos.line = MAX(pos.maxsepstr + 3, pos.xdesc + pos.maxdesc);

	menupad = newpad(totnitems, pos.line);
	wbkgd(menupad, t.widgetcolor);

	getfirst(ngroups, groups, &abs, &g, &rel);
	/*totnitems = 0;
	for (i=0; i<ngroups; i++) {
		currmode = getmode(mode, groups[i]);
		for (j=0; j<groups[i].nitems; j++) {
			item = &groups[i].items[j];
			if (conf.default_item != NULL)
				if (strcmp(item->name, conf.default_item) == 0)
					if (currmode != SEPARATORMODE) {
						abs = totnitems;
						i = groups[i].nitems;
						j = ngroups;
					}
			totnitems++;
		}
	}*/
	totnitems = 0;
	for (i=0; i<ngroups; i++) {
		currmode = getmode(mode, groups[i]);
		for (j=0; j<groups[i].nitems; j++) {
			item = &groups[i].items[j];
			draw_myitem(conf, menupad, totnitems, *item, currmode,
			    pos, totnitems == abs);
			totnitems++;
		}
	}

	ys = y + rows - 5 - menurows + 1;
	ye = ys + menurows + 2 -1;
	xs = (pos.line > cols - 6) ? (x + 2 + 1) : x + 3 + (cols-6)/2 - pos.line/2;
	xe = (pos.line > cols - 6) ? xs + cols - 7 : xs + cols - 4 -1;
	if (currmode == TREEVIEWMODE)
		xs = x + 2 + 1;

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	wrefresh(menuwin);
	prefresh(menupad, 0, 0, ys, xs, ye, xe);//delete?

	item = &groups[g].items[rel];
	currmode = getmode(mode, groups[g]);
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
			output = bs.value[bs.curr];
			if (currmode == MENUMODE)
				item->on = true;
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

		if (abs < 0)
			continue;

		switch(input) {
		case KEY_UP:
			draw_myitem(conf, menupad, abs, *item, currmode, pos, false);
			getprev(ngroups, groups, &abs, &g, &rel);
			item = &groups[g].items[rel];
			currmode= getmode(mode, groups[g]);
			draw_myitem(conf, menupad, abs, *item, currmode, pos, true);
			break;
		case KEY_DOWN:
			draw_myitem(conf, menupad, abs, *item, currmode, pos, false);
			getnext(ngroups, groups, &abs, &g, &rel);
			item = &groups[g].items[rel];
			currmode = getmode(mode, groups[g]);
			draw_myitem(conf, menupad, abs, *item, currmode, pos, true);
			break;
		case ' ': // Space
			if (currmode == MENUMODE)
				break;
			else if (currmode == CHECKLISTMODE)
				item->on = !item->on;
			else { //RADIOLISTMODE and TREEVIEWMODE
				if (item->on == true)
					break;
				for (i=0; i<groups[g].nitems; i++)
					if (groups[g].items[i].on == true) {
						groups[g].items[i].on = false;
						draw_myitem(conf, menupad,
						    abs - rel + i, groups[g].items[i],
						    currmode, pos, false);
					}
				item->on = true;
			}
			draw_myitem(conf, menupad, abs, *item, currmode, pos, true);
		}
	}

	if (abs >= 0)
		print_selected_list(conf, output, mode, ngroups, groups, g, rel);

	delwin(menupad);
	delwin(menuwin);
	widget_end(conf, namewidget, widget, rows, cols, shadow, button);

	return output;
}


int bsddialog_mixedlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int ngroups, struct bsddialog_menugroup *groups)
{
	int output;

	output = do_mixedlist(conf, text, rows, cols, menurows, "Mixedlist",
	    MIXEDLISTMODE, ngroups, groups);

	return output;
}

int
bsddialog_checklist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int output;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_CHECKLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, "Checklist",
	    CHECKLISTMODE, 1, &group);

	return output;
}

int
bsddialog_menu(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int output;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_CHECKLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, "Menu",
	    MENUMODE, 1, &group);

	return output;
}

int
bsddialog_radiolist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int output;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_RADIOLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, "Radiolist",
	    RADIOLISTMODE, 1, &group);

	return output;
}

int
bsddialog_treeview(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	int output;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_RADIOLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, "Treeview",
	    TREEVIEWMODE, 1, &group);

	return output;
}

int
bsddialog_buildlist(struct config conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items)
{
	WINDOW *widget, *button, *leftwin, *leftpad, *rightwin, *rightpad, *shadow;
	int output, i, x, y, input;
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
	leftpad  = newpad(nitems, pos.line);
	rightpad = newpad(nitems, pos.line);
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
					draw_myitem(conf, leftpad, nlefts, items[i],
					    BUILDLISTMODE, pos, curr == i);
					nlefts++;
				} else {
					if (currV == RIGHT && currH == nrights)
						curr = i;
					draw_myitem(conf, rightpad, nrights, items[i],
					    BUILDLISTMODE, pos, curr == i);
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
