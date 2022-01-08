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
#include <ncurses.h>
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define DEPTHSPACE	4
#define MIN_HEIGHT	VBORDERS + 6 /* 2 buttons 1 text 3 menu */

extern struct bsddialog_theme t;

enum menumode {
	CHECKLISTMODE,
	MENUMODE,
	MIXEDLISTMODE,
	RADIOLISTMODE,
	SEPARATORMODE
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

static int checkradiolist(int nitems, struct bsddialog_menuitem *items)
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

static int checkmenu(int nitems, struct bsddialog_menuitem *items) // useful?
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

static void
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
			*abs = a;
			*rel = 0;
			break;
		}
	}
}

static void
getfirst_with_default(int ngroups, struct bsddialog_menugroup *groups, int *abs,
    int *group, int *rel, int *focusgroup, int *focusitem)
{
	int i;

	getfirst(ngroups, groups, abs, group, rel);
	if (*abs < 0)
		return;

	if (focusgroup == NULL || focusitem == NULL)
		return;
	if (*focusgroup < 0 || *focusgroup >= ngroups)
		return;
	if (groups[*focusgroup].type == BSDDIALOG_SEPARATOR)
		return;
	if (*focusitem < 0 || *focusitem >= groups[*focusgroup].nitems)
		return;

	for (i = *group; i < *focusgroup; i++)
		*abs += groups[i].nitems;

	*abs += *focusitem;
	*group = *focusgroup;
	*rel = *focusitem;
}

static void
getlast(int totnitems, int ngroups, struct bsddialog_menugroup *groups,
    int *abs, int *group, int *rel)
{
	int i, a;

	a = totnitems - 1;
	for (i = ngroups-1; i>=0; i--) {
		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			a -= groups[i].nitems;
			continue;
		}
		if (groups[i].nitems != 0) {
			*group = i;
			*abs = a;
			*rel = groups[i].nitems - 1;
			break;
		}
	}
}

static void
getnext(int ngroups, struct bsddialog_menugroup *groups, int *abs, int *group,
    int *rel)
{
	int i, a;

	if (*abs < 0 || *group < 0 || *rel < 0)
		return;

	if (*rel + 1 < (int) groups[*group].nitems) {
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

static void
getfastnext(int menurows, int ngroups, struct bsddialog_menugroup *groups,
    int *abs, int *group, int *rel)
{
	int a, start, i;

	start = *abs;
	i = menurows;
	do {
		a = *abs;
		getnext(ngroups, groups, abs, group, rel);
		i--;
	} while (*abs != a && *abs < start + menurows && i > 0);
}

static void
getprev(struct bsddialog_menugroup *groups, int *abs, int *group, int *rel)
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
			a -= (int) groups[i].nitems;
			continue;
		}
		if (groups[i].nitems != 0) {
			*group = i;
			*abs = a - 1;
			*rel = (int) groups[i].nitems - 1;
			break;
		}
	}
}

static void
getfastprev(int menurows, struct bsddialog_menugroup *groups, int *abs,
    int *group, int *rel)
{
	int a, start, i;

	start = *abs;
	i = menurows;
	do {
		a = *abs;
		getprev(groups, abs, group, rel);
		i--;
	} while (*abs != a && *abs > start - menurows && i > 0);
}

static bool
getnextshortcut(struct bsddialog_conf *conf, int ngroups,
    struct bsddialog_menugroup *groups, int *abs, int *group, int *rel,
    int key)
{
	int i, j, a, ch, ng, nr, na;
	bool mainloop;

	if (*abs < 0 || ngroups < 0 || *rel < 0)
		return (false);

	na = a = -1;
	mainloop = true;
	for (i = 0; i < ngroups && mainloop; i++) {
		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			a += groups[i].nitems;
			continue;
		}
		for (j = 0; j < (int)groups[i].nitems; j++) {
			a++;
			if (a == *abs)
				continue;

			if (conf->menu.no_name)
				ch = groups[i].items[j].desc[0];
			else
				ch = groups[i].items[j].name[0];
	
			if (ch == key) {
				if (a < *abs && na == -1) {
					na = a;
					ng = i;
					nr = j;
				}
				if (a > *abs) {
					na = a;
					ng = i;
					nr = j;
					mainloop = false;
					break;
				}
			}
		}
	}

	if (na != -1) {
		*abs = na;
		*group = ng;
		*rel = nr;
		return (true);
	}

	return (false);
}

static enum menumode
getmode(enum menumode mode, struct bsddialog_menugroup group)
{

	if (mode == MIXEDLISTMODE) {
		if (group.type == BSDDIALOG_SEPARATOR)
			mode = SEPARATORMODE;
		else if (group.type == BSDDIALOG_RADIOLIST)
			mode = RADIOLISTMODE;
		else if (group.type == BSDDIALOG_CHECKLIST)
			mode = CHECKLISTMODE;
	}

	return (mode);
}

static void
drawitem(struct bsddialog_conf *conf, WINDOW *pad, int y,
    struct bsddialog_menuitem item, enum menumode mode, struct lineposition pos,
    bool curr)
{
	int colordesc, colorname, colorshortcut, linech;
	const char *shortcut;

	if (mode == SEPARATORMODE) {
		if (conf->no_lines == false) {
			wattron(pad, t.menu.desccolor);
			linech = conf->ascii_lines ? '-' : ACS_HLINE;
			mvwhline(pad, y, 0, linech, pos.line);
			wattroff(pad, t.menu.desccolor);
		}
		wmove(pad, y,
		    pos.line/2 - (strlen(item.name) + strlen(item.desc)) / 2 );
		wattron(pad, t.menu.namesepcolor);
		waddstr(pad, item.name);
		wattroff(pad, t.menu.namesepcolor);
		if (strlen(item.name) > 0 && strlen(item.desc) > 0)
			waddch(pad, ' ');
		wattron(pad, t.menu.descsepcolor);
		waddstr(pad, item.desc);
		wattroff(pad, t.menu.descsepcolor);
		return;
	}

	/* prefix */
	if (item.prefix != NULL && item.prefix[0] != '\0')
		mvwaddstr(pad, y, 0, item.prefix);

	/* selector */
	wmove(pad, y, pos.xselector);
	wattron(pad, t.menu.selectorcolor);
	if (mode == CHECKLISTMODE)
		wprintw(pad, "[%c]", item.on ? 'X' : ' ');
	if (mode == RADIOLISTMODE)
		wprintw(pad, "(%c)", item.on ? '*' : ' ');
	wattroff(pad, t.menu.selectorcolor);

	/* name */
	colorname = curr ? t.menu.f_namecolor : t.menu.namecolor;
	if (conf->menu.no_name == false) {
		wattron(pad, colorname);
		mvwaddstr(pad, y, pos.xname + item.depth * DEPTHSPACE, item.name);
		wattroff(pad, colorname);
	}

	/* description */
	if (conf->menu.no_name)
		colordesc = curr ? t.menu.f_namecolor : t.menu.namecolor;
	else
		colordesc = curr ? t.menu.f_desccolor : t.menu.desccolor;

	if (conf->menu.no_desc == false) {
		wattron(pad, colordesc);
		if (conf->menu.no_name)
			mvwaddstr(pad, y, pos.xname + item.depth * DEPTHSPACE,
			    item.desc);
		else
			mvwaddstr(pad, y, pos.xdesc, item.desc);
		wattroff(pad, colordesc);
	}

	/* shortcut */
	if (conf->menu.shortcut_buttons == false) {
		colorshortcut = curr ?
		    t.menu.f_shortcutcolor : t.menu.shortcutcolor;
		wattron(pad, colorshortcut);

		if (conf->menu.no_name)
			shortcut = item.desc;
		else
			shortcut = item.name;
		wmove(pad, y, pos.xname + item.depth * DEPTHSPACE);
		if (shortcut != NULL && shortcut[0] != '\0')
			waddch(pad, shortcut[0]);
	wattroff(pad, colorshortcut);
	}

	/* bottom description */
	move(LINES-1, 2);
	clrtoeol();
	if (item.bottomdesc != NULL) {
		addstr(item.bottomdesc);
		refresh();
	}
}

static void
menu_autosize(struct bsddialog_conf *conf, int rows, int cols, int *h, int *w,
    const char *text, int linelen, unsigned int *menurows, int nitems,
    struct buttons bs)
{
	int textrow, menusize;

	if (cols == BSDDIALOG_AUTOSIZE)
		*w = widget_min_width(conf, &bs, linelen + 6);

	textrow = text != NULL && strlen(text) > 0 ? 1 : 0;
	if (rows == BSDDIALOG_AUTOSIZE) {
		if (*menurows == 0) {
			menusize = widget_max_height(conf) - HBORDERS -
			     2 /*buttons*/ - textrow;
			menusize = MIN(menusize, nitems + 2);
			*menurows = menusize - 2 < 0 ? 0 : menusize - 2;
		}
		else /* h autosize with fixed menurows */
			menusize = *menurows + 2;

		*h = widget_min_height(conf, true, menusize + textrow);
		/* 
		 * avoid menurows overflow and
		 * with rows=AUTOSIZE menurows!=0 becomes max-menurows
		 */
		*menurows = MIN(*h - 6 - textrow, (int)*menurows);
	} else {
		if (*menurows == 0)
			*menurows = MIN(rows-6-textrow, nitems);
	}
}

static int
menu_checksize(int rows, int cols, const char *text, int menurows, int nitems,
    struct buttons bs)
{
	int mincols, textrow, menusize;

	mincols = VBORDERS;
	/* buttons */
	mincols += bs.nbuttons * bs.sizebutton;
	mincols += bs.nbuttons > 0 ? (bs.nbuttons-1) * t.button.space : 0;
	/*
	 * linelen check, comment to allow some hidden col otherwise portconfig
	 * could not show big menus like www/apache24
	 */
	/* mincols = MAX(mincols, linelen); */

	if (cols < mincols)
		RETURN_ERROR("Few cols, width < size buttons or "
		    "name + descripion of the items");

	textrow = text != NULL && strlen(text) > 0 ? 1 : 0;

	if (nitems > 0 && menurows == 0)
		RETURN_ERROR("items > 0 but menurows == 0, probably terminal "
		    "too small");

	menusize = nitems > 0 ? 3 : 0;
	if (rows < 2  + 2 + menusize + textrow)
		RETURN_ERROR("Few lines for this menus");

	return (0);
}

/* the caller has to call prefresh(menupad, ymenupad, 0, ys, xs, ye, xe); */
static void
update_menuwin(struct bsddialog_conf *conf, WINDOW *menuwin, int h, int w,
    int totnitems, unsigned int menurows, int ymenupad)
{

	draw_borders(conf, menuwin, h, w, LOWERED);

	if (totnitems > (int) menurows) {
		wattron(menuwin, t.menu.arrowcolor);

		if (ymenupad > 0)
			mvwprintw(menuwin, 0, 2, "^^^");

		if ((int) (ymenupad + menurows) < totnitems)
			mvwprintw(menuwin, h-1, 2, "vvv");

		wattroff(menuwin, t.menu.arrowcolor);

		mvwprintw(menuwin, h-1, w-10, "%3d%%",
		    100 * (ymenupad + menurows) / totnitems);
	}
}

static int
do_mixedlist(struct bsddialog_conf *conf, const char *text, int rows, int cols,
    unsigned int menurows, enum menumode mode, int ngroups,
    struct bsddialog_menugroup *groups, int *focuslist, int *focusitem)
{
	WINDOW  *shadow, *widget, *textpad, *menuwin, *menupad;
	int i, j, y, x, h, w, output, input;
	int ymenupad, ys, ye, xs, xe, abs, g, rel, totnitems;
	bool loop, automenurows, shortcut_butts;
	struct buttons bs;
	struct bsddialog_menuitem *item;
	enum menumode currmode;
	struct lineposition pos = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	shortcut_butts = conf->menu.shortcut_buttons;

	automenurows = menurows == BSDDIALOG_AUTOSIZE ? true : false;

	totnitems = 0;
	for (i=0; i < ngroups; i++) {
		currmode = getmode(mode, groups[i]);
		if (currmode == RADIOLISTMODE)
			checkradiolist(groups[i].nitems, groups[i].items);

		if (currmode == MENUMODE)
			checkmenu(groups[i].nitems, groups[i].items);

		if (currmode == RADIOLISTMODE || currmode == CHECKLISTMODE)
			pos.selectorlen = 3;

		for (j=0; j < (int) groups[i].nitems; j++) {
			totnitems++;
			item = &groups[i].items[j];

			if (groups[i].type == BSDDIALOG_SEPARATOR) {
				pos.maxsepstr = MAX(pos.maxsepstr,
				    strlen(item->name) + strlen(item->desc));
				continue;
			}

			pos.maxprefix = MAX(pos.maxprefix, strlen(item->prefix));
			pos.maxdepth  = MAX(pos.maxdepth, item->depth);
			pos.maxname   = MAX(pos.maxname, strlen(item->name));
			pos.maxdesc   = MAX(pos.maxdesc, strlen(item->desc));
		}
	}
	pos.maxname = conf->menu.no_name ? 0 : pos.maxname;
	pos.maxdesc = conf->menu.no_desc ? 0 : pos.maxdesc;
	pos.maxdepth *= DEPTHSPACE;

	pos.xselector = pos.maxprefix + (pos.maxprefix != 0 ? 1 : 0);
	pos.xname = pos.xselector + pos.selectorlen +
	    (pos.selectorlen > 0 ? 1 : 0);
	pos.xdesc = pos.maxdepth + pos.xname + pos.maxname;
	pos.xdesc += (pos.maxname != 0 ? 1 : 0);
	pos.line = MAX(pos.maxsepstr + 3, pos.xdesc + pos.maxdesc);


	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	menu_autosize(conf, rows, cols, &h, &w, text, pos.line, &menurows,
	    totnitems, bs);
	if (menu_checksize(h, w, text, menurows, totnitems, bs) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs,
	     shortcut_butts) != 0)
		return (BSDDIALOG_ERROR);

	doupdate();

	prefresh(textpad, 0, 0, y + 1, x + 1 + t.text.hmargin,
	    y + h - menurows, x + 1 + w - t.text.hmargin);

	menuwin = new_boxed_window(conf, y + h - 5 - menurows, x + 2,
	    menurows+2, w-4, LOWERED);

	menupad = newpad(totnitems, pos.line);
	wbkgd(menupad, t.dialog.color);

	ymenupad = 0;
	for (i=0; i<ngroups; i++) {
		currmode = getmode(mode, groups[i]);
		for (j=0; j < (int) groups[i].nitems; j++) {
			item = &groups[i].items[j];
			drawitem(conf, menupad, ymenupad, *item, currmode, pos,
			    false);
			ymenupad++;
		}
	}
	getfirst_with_default(ngroups, groups, &abs, &g, &rel, focuslist, focusitem);
	if (abs >= 0) {
		currmode = getmode(mode, groups[g]);
		item = &groups[g].items[rel];
		drawitem(conf, menupad, abs, *item, currmode, pos, true);
	}

	ys = y + h - 5 - menurows + 1;
	ye = y + h - 5 ;
	if (conf->menu.align_left || (int)pos.line > w - 6) {
		xs = x + 3;
		xe = xs + w - 7;
	}
	else { /* center */
		xs = x + 3 + (w-6)/2 - pos.line/2;
		xe = xs + w - 5;
	}

	ymenupad = 0; /* now ymenupad is pminrow for prefresh() */
	if ((int)(ymenupad + menurows) - 1 < abs)
		ymenupad = abs - menurows + 1;
	update_menuwin(conf, menuwin, menurows+2, w-4, totnitems, menurows,
	    ymenupad);
	wrefresh(menuwin);
	prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);

	loop = true;
	while(loop) {
		input = getch();
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			output = bs.value[bs.curr];
			if (abs >= 0 && currmode == MENUMODE)
				item->on = true;
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				output = BSDDIALOG_ESC;
				if (abs >= 0 && currmode == MENUMODE)
					item->on = true;
				loop = false;
			}
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			draw_buttons(widget, bs, shortcut_butts);
			wrefresh(widget);
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				draw_buttons(widget, bs, shortcut_butts);
				wrefresh(widget);
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				draw_buttons(widget, bs, shortcut_butts);
				wrefresh(widget);
			}
			break;
		case KEY_F(1):
			if (conf->f1_file == NULL && conf->f1_message == NULL)
				break;
			if (f1help(conf) != 0)
				return (BSDDIALOG_ERROR);
			/* No break, screen size can change */
		case KEY_RESIZE:
			/* Important for decreasing screen */
			hide_widget(y, x, h, w, conf->shadow);
			refresh();

			if (set_widget_size(conf, rows, cols, &h, &w) != 0)
				return (BSDDIALOG_ERROR);
			menurows = automenurows ? 0 : menurows;
			menu_autosize(conf, rows, cols, &h, &w, text, pos.line,
			    &menurows, totnitems, bs);
			if (menu_checksize(h, w, text, menurows, totnitems,
			    bs) != 0)
				return (BSDDIALOG_ERROR);
			if (set_widget_position(conf, &y, &x, h, w) != 0)
				return (BSDDIALOG_ERROR);

			if(update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs, shortcut_butts) != 0)
				return (BSDDIALOG_ERROR);
			
			doupdate();

			prefresh(textpad, 0, 0, y + 1, x + 1 + t.text.hmargin,
			    y + h - menurows, x + 1 + w - t.text.hmargin);

			wclear(menuwin);
			mvwin(menuwin, y + h - 5 - menurows, x + 2);
			wresize(menuwin,menurows+2, w-4);
			update_menuwin(conf, menuwin, menurows+2, w-4,
			    totnitems, menurows, ymenupad);
			wrefresh(menuwin);
			
			ys = y + h - 5 - menurows + 1;
			ye = y + h - 5 ;
			if (conf->menu.align_left || (int)pos.line > w - 6) {
				xs = x + 3;
				xe = xs + w - 7;
			}
			else { /* center */
				xs = x + 3 + (w-6)/2 - pos.line/2;
				xe = xs + w - 5;
			}

			if ((int)(ymenupad + menurows) - 1 < abs)
				ymenupad = abs - menurows + 1;
			prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);

			refresh();

			break;
		}

		if (abs < 0)
			continue;
		switch(input) {
		case KEY_HOME:
		case KEY_UP:
		case KEY_PPAGE:
			if (abs == 0) /* useless, just to save cpu refresh */
				break;
			drawitem(conf, menupad, abs, *item, currmode, pos, false);
			if (input == KEY_HOME)
				getfirst(ngroups, groups, &abs, &g, &rel);
			else if (input == KEY_UP)
				getprev(groups, &abs, &g, &rel);
			else /* input == KEY_PPAGE*/
				getfastprev(menurows, groups, &abs, &g, &rel);
			item = &groups[g].items[rel];
			currmode= getmode(mode, groups[g]);
			drawitem(conf, menupad, abs, *item, currmode, pos, true);
			if (ymenupad > abs && ymenupad > 0)
				ymenupad = abs;
			update_menuwin(conf, menuwin, menurows+2, w-4, totnitems,
			    menurows, ymenupad);
			wrefresh(menuwin);
			prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);
			break;
		case KEY_END:
		case KEY_DOWN:
		case KEY_NPAGE:
			if (abs == totnitems -1)
				break; /* useless, just to save cpu refresh */
			drawitem(conf, menupad, abs, *item, currmode, pos, false);
			if (input == KEY_END)
				getlast(totnitems, ngroups, groups, &abs, &g,
				    &rel);
			else if (input == KEY_DOWN)
				getnext(ngroups, groups, &abs, &g, &rel);
			else /* input == KEY_NPAGE*/
				getfastnext(menurows, ngroups, groups, &abs, &g,
				    &rel);
			item = &groups[g].items[rel];
			currmode= getmode(mode, groups[g]);
			drawitem(conf, menupad, abs, *item, currmode, pos, true);
			if ((int)(ymenupad + menurows) <= abs)
				ymenupad = abs - menurows + 1;
			update_menuwin(conf, menuwin, menurows+2, w-4, totnitems,
			    menurows, ymenupad);
			wrefresh(menuwin);
			prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);
			break;
		case ' ': /* Space */
			if (currmode == MENUMODE)
				break;
			else if (currmode == CHECKLISTMODE)
				item->on = !item->on;
			else { /* RADIOLISTMODE */
				for (i=0; i < (int) groups[g].nitems; i++)
					if (groups[g].items[i].on && i != rel) {
						groups[g].items[i].on = false;
						drawitem(conf, menupad,
						    abs - rel + i,
						    groups[g].items[i],
						    currmode, pos, false);
					}
				item->on = !item->on;
			}
			drawitem(conf, menupad, abs, *item, currmode, pos, true);
			prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);
		default:
			if (shortcut_butts) {
				if (shortcut_buttons(input, &bs)) {
					output = bs.value[bs.curr];
					if (currmode == MENUMODE)
						item->on = true;
					loop = false;
				}
				break;
			}

			drawitem(conf, menupad, abs, *item, currmode, pos, false);
			getnextshortcut(conf, ngroups, groups, &abs, &g, &rel,
			    input);
			item = &groups[g].items[rel];
			currmode = getmode(mode, groups[g]);
			drawitem(conf, menupad, abs, *item, currmode, pos, true);
			if (ymenupad > abs && ymenupad > 0)
				ymenupad = abs;
			if ((int)(ymenupad + menurows) <= abs)
				ymenupad = abs - menurows + 1;
			update_menuwin(conf, menuwin, menurows+2, w-4,
			    totnitems, menurows, ymenupad);
			wrefresh(menuwin);
			prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);
		}
	}

	if (focuslist != NULL)
		*focuslist = g;
	if (focusitem !=NULL)
		*focusitem = rel;

	delwin(menupad);
	delwin(menuwin);
	end_dialog(conf, shadow, widget, textpad);

	return (output);
}

/* API */
int
bsddialog_mixedlist(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, int ngroups,
    struct bsddialog_menugroup *groups, int *focuslist, int *focusitem)
{
	int output;

	output = do_mixedlist(conf, text, rows, cols, menurows, MIXEDLISTMODE,
	    ngroups, groups, focuslist, focusitem);

	return (output);
}

int
bsddialog_checklist(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, int nitems,
    struct bsddialog_menuitem *items, int *focusitem)
{
	int output, focuslist = 0;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_CHECKLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, CHECKLISTMODE,
	    1, &group, &focuslist, focusitem);

	return (output);
}

int
bsddialog_menu(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, int nitems,
    struct bsddialog_menuitem *items, int *focusitem)
{
	int output, focuslist = 0;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_CHECKLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, MENUMODE, 1,
	    &group, &focuslist, focusitem);

	return (output);
}

int
bsddialog_radiolist(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, int nitems,
    struct bsddialog_menuitem *items, int *focusitem)
{
	int output, focuslist = 0;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_RADIOLIST /* unused */, nitems, items};

	output = do_mixedlist(conf, text, rows, cols, menurows, RADIOLISTMODE,
	    1, &group, &focuslist, focusitem);

	return (output);
}
