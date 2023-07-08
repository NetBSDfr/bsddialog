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
#include <stdlib.h>
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

#define MIN_HEIGHT  BORDERS + 6 /* 2 buttons 1 text 3 menu */

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

struct privateitem {
	bool on;
	/* "links" to api item */
	int group;
	int index;
	enum menumode type;
	struct bsddialog_menuitem *apiitem;
};

static void
set_on_output(struct bsddialog_conf *conf, int output, int ngroups,
    struct bsddialog_menugroup *groups, struct privateitem *pritems)
{
	int i, j, abs;

	if (output != BSDDIALOG_OK && !conf->menu.on_without_ok)
		return;

	for(i = abs = 0; i < ngroups; i++) {
		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			abs += groups[i].nitems;
			continue;
		}

		for(j = 0; j < (int)groups[i].nitems; j++) {
			groups[i].items[j].on = pritems[abs].on;
			abs++;
		}
	}
}

static int getprev(struct privateitem *pritems, int abs)
{
	int i;

	for (i = abs - 1; i >= 0; i--) {
		if (pritems[i].type == SEPARATORMODE)
			continue;
		return (i);
	}

	return (abs);
}

static int getnext(int npritems, struct privateitem *pritems, int abs)
{
	int i;

	for (i = abs + 1; i < npritems; i++) {
		if (pritems[i].type == SEPARATORMODE)
			continue;
		return (i);
	}

	return (abs);
}

static int
getfirst_with_default(int npritems, struct privateitem *pritems, int ngroups,
    struct bsddialog_menugroup *groups, int *focusgroup, int *focusitem)
{
	int i, abs;

	if ((abs =  getnext(npritems, pritems, -1)) < 0)
		return (abs);

	if (focusgroup == NULL || focusitem == NULL)
		return (abs);
	if (*focusgroup < 0 || *focusgroup >= ngroups)
		return (abs);
	if (groups[*focusgroup].type == BSDDIALOG_SEPARATOR)
		return (abs);
	if (*focusitem < 0 || *focusitem >= (int)groups[*focusgroup].nitems)
		return (abs);

	for (i = abs; i < npritems; i++) {
		if (pritems[i].group == *focusgroup &&
		    pritems[i].index == *focusitem)
			return (i);
	}

	return (abs);
}

static int
getfastnext(int menurows, int npritems, struct privateitem *pritems, int abs)
{
	int a, start, i;

	start = abs;
	i = menurows;
	do {
		a = abs;
		abs = getnext(npritems, pritems, abs);
		i--;
	} while (abs != a && abs < start + menurows && i > 0);

	return (abs);
}

static int
getfastprev(int menurows, struct privateitem *pritems, int abs)
{
	int a, start, i;

	start = abs;
	i = menurows;
	do {
		a = abs;
		abs = getprev(pritems, abs);
		i--;
	} while (abs != a && abs > start - menurows && i > 0);

	return (abs);
}

static int
getnextshortcut(struct bsddialog_conf *conf, int npritems,
    struct privateitem *pritems, int abs, wint_t key)
{
	int i, next;
	wchar_t wch;

	next = -1;
	for (i = 0; i < npritems; i++) {
		if (pritems[i].type == SEPARATORMODE)
			continue;

		if (conf->menu.no_name)
			mbtowc(&wch, pritems[i].apiitem->desc, MB_CUR_MAX);
		else
			mbtowc(&wch, pritems[i].apiitem->name, MB_CUR_MAX);

		if (wch == (wchar_t)key) {
			if (i > abs)
				return (i);

			if (i < abs && next == -1)
				next = i;
		}
	}

	return (next != -1 ? next : abs);
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
drawseparators(struct bsddialog_conf *conf, WINDOW *pad, int linelen,
    int nitems, struct privateitem *pritems)
{
	int i, linech, labellen;
	const char *desc, *name;

	for (i = 0; i < nitems; i++) {
		if (pritems[i].type != SEPARATORMODE)
			continue;
		if (conf->no_lines == false) {
			wattron(pad, t.menu.desccolor);
			linech = conf->ascii_lines ? '-' : ACS_HLINE;
			mvwhline(pad, i, 0, linech, linelen);
			wattroff(pad, t.menu.desccolor);
		}
		name = pritems[i].apiitem->name;
		desc = pritems[i].apiitem->desc;
		labellen = strcols(name) + strcols(desc) + 1;
		wmove(pad, i, labellen < linelen ? linelen/2 - labellen/2 : 0);
		wattron(pad, t.menu.namesepcolor);
		waddstr(pad, name);
		wattroff(pad, t.menu.namesepcolor);
		if (strcols(name) > 0 && strcols(desc) > 0)
			waddch(pad, ' ');
		wattron(pad, t.menu.descsepcolor);
		waddstr(pad, desc);
		wattroff(pad, t.menu.descsepcolor);
	}
}

static void
drawitem(struct bsddialog_conf *conf, WINDOW *pad, int y,
    struct lineposition pos, struct privateitem *pritem, bool focus)
{
	int colordesc, colorname, colorshortcut;
	wchar_t shortcut;
	struct bsddialog_menuitem *item;

	item = pritem->apiitem;

	/* prefix */
	if (item->prefix != NULL && item->prefix[0] != '\0')
		mvwaddstr(pad, y, 0, item->prefix);

	/* selector */
	wmove(pad, y, pos.xselector);
	wattron(pad, focus ? t.menu.f_selectorcolor : t.menu.selectorcolor);
	if (pritem->type == CHECKLISTMODE)
		wprintw(pad, "[%c]", pritem->on ? 'X' : ' ');
	if (pritem->type == RADIOLISTMODE)
		wprintw(pad, "(%c)", pritem->on ? '*' : ' ');
	wattroff(pad, focus ? t.menu.f_selectorcolor : t.menu.selectorcolor);

	/* name */
	colorname = focus ? t.menu.f_namecolor : t.menu.namecolor;
	if (conf->menu.no_name == false) {
		wattron(pad, colorname);
		mvwaddstr(pad, y, pos.xname + item->depth, item->name);
		wattroff(pad, colorname);
	}

	/* description */
	if (conf->menu.no_name)
		colordesc = focus ? t.menu.f_namecolor : t.menu.namecolor;
	else
		colordesc = focus ? t.menu.f_desccolor : t.menu.desccolor;

	if (conf->menu.no_desc == false) {
		wattron(pad, colordesc);
		if (conf->menu.no_name)
			mvwaddstr(pad, y, pos.xname + item->depth, item->desc);
		else
			mvwaddstr(pad, y, pos.xdesc, item->desc);
		wattroff(pad, colordesc);
	}

	/* shortcut */
	if (conf->menu.shortcut_buttons == false) {
		colorshortcut = focus ?
		    t.menu.f_shortcutcolor : t.menu.shortcutcolor;
		wattron(pad, colorshortcut);

		if (conf->menu.no_name)
			mbtowc(&shortcut, item->desc, MB_CUR_MAX);
		else
			mbtowc(&shortcut, item->name, MB_CUR_MAX);
		mvwaddwch(pad, y, pos.xname + item->depth, shortcut);
		wattroff(pad, colorshortcut);
	}

	/* bottom description */
	move(SCREENLINES - 1, 2);
	clrtoeol();
	if (item->bottomdesc != NULL && focus) {
		attron(t.menu.bottomdesccolor);
		addstr(item->bottomdesc);
		attroff(t.menu.bottomdesccolor);
		refresh();
	}
}

/* the caller has to call prefresh(menupad, ymenupad, 0, ys, xs, ye, xe); */
static void
update_menuwin(struct bsddialog_conf *conf, WINDOW *menuwin, int h, int w,
    int totnitems, unsigned int menurows, int ymenupad)
{
	draw_borders(conf, menuwin, h, w, LOWERED);

	if (totnitems > (int)menurows) {
		wattron(menuwin, t.dialog.arrowcolor);
		if (ymenupad > 0)
			mvwhline(menuwin, 0, 2,
			    conf->ascii_lines ? '^' : ACS_UARROW, 3);

		if ((ymenupad + (int)menurows) < totnitems)
			mvwhline(menuwin, h-1, 2,
			    conf->ascii_lines ? 'v' : ACS_DARROW, 3);

		mvwprintw(menuwin, h-1, w-6, "%3d%%",
		    100 * (ymenupad + menurows) / totnitems);
		wattroff(menuwin, t.dialog.arrowcolor);
	}
}

int
menu_size_position(struct bsddialog_conf *conf, int rows, int cols, 
    const char *text, unsigned int *menurows, int nitems, int linelen,
    struct buttons *bs, int *y, int *x, int *h, int *w)
{
	int htext, hmenu;

	if (set_widget_size(conf, rows, cols, h, w) != 0)
		return (BSDDIALOG_ERROR);

	hmenu = (*menurows == BSDDIALOG_AUTOSIZE) ? nitems : (int)*menurows;
	hmenu += 2; /* menu borders */
	/* 
	 * algo 1: notext = 1 (grows vertically).
	 * algo 2: notext = hmenu (grows horizontally, better for little term).
	 */
	if (set_widget_autosize(conf, rows, cols, h, w, text, &htext, bs,
	    hmenu, linelen + 4) != 0)
		return (BSDDIALOG_ERROR);
	/* avoid menurows overflow and menurows becomes "at most menurows" */
	if (*h - BORDERS - htext - HBUTTONS <= 2 /* menuborders */)
		*menurows = (nitems > 0) ? 1 : 0; /* for widget_checksize() */
	else
		*menurows = MIN(*h - BORDERS - htext - HBUTTONS, hmenu) - 2;

	/*
	 * no minw=linelen to avoid big menu fault, then some col can be
	 * hidden (example portconfig www/apache24).
	 */
	if (widget_checksize(*h, *w, bs, 2/*bmenu*/ + MIN(*menurows,1), 0) != 0)
		return (BSDDIALOG_ERROR);

	if (set_widget_position(conf, y, x, *h, *w) != 0)
		return (BSDDIALOG_ERROR);

	return (0);
}

struct privatemenu {
	WINDOW *box;           /* only for borders */
	WINDOW *pad;           /* pad for the private items */
	int ypad;              /* start pad line */
	unsigned int menurows; /* real menurows after menu_size_position() */
	int nitems;            /* total nitems (all groups * all items) */
	int sel;               /* current focus item, can be -1 */
};

static int
do_mixedlist(struct bsddialog_conf *conf, const char *text, int rows, int cols,
    unsigned int menurows, enum menumode mode, unsigned int ngroups,
    struct bsddialog_menugroup *groups, int *focuslist, int *focusitem)
{
	bool loop, onetrue, movefocus;
	int i, j, y, x, h, w, retval;
	int ys, ye, xs, xe, next;
	wint_t input;
	WINDOW  *shadow, *widget, *textpad;
	struct buttons bs;
	struct lineposition pos = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	struct bsddialog_menuitem *item;
	struct privateitem *pritems;
	struct privatemenu m;

	CHECK_PTR_SIZE(focuslist, int);
	CHECK_PTR_SIZE(focusitem, int);

	m.nitems = 0;
	for (i = 0; i < (int)ngroups; i++) {
		if (getmode(mode, groups[i]) == RADIOLISTMODE ||
		    getmode(mode, groups[i]) == CHECKLISTMODE)
			pos.selectorlen = 3;

		for (j = 0; j < (int)groups[i].nitems; j++) {
			m.nitems++;
			item = &groups[i].items[j];

			if (groups[i].type == BSDDIALOG_SEPARATOR) {
				pos.maxsepstr = MAX(pos.maxsepstr,
				    strcols(item->name) + strcols(item->desc));
				continue;
			}

			pos.maxprefix = MAX(pos.maxprefix,strcols(item->prefix));
			pos.maxdepth  = MAX(pos.maxdepth, item->depth);
			pos.maxname   = MAX(pos.maxname, strcols(item->name));
			pos.maxdesc   = MAX(pos.maxdesc, strcols(item->desc));
		}
	}
	pos.maxname = conf->menu.no_name ? 0 : pos.maxname;
	pos.maxdesc = conf->menu.no_desc ? 0 : pos.maxdesc;

	pos.xselector = pos.maxprefix + (pos.maxprefix != 0 ? 1 : 0);
	pos.xname = pos.xselector + pos.selectorlen +
	    (pos.selectorlen > 0 ? 1 : 0);
	pos.xdesc = pos.maxdepth + pos.xname + pos.maxname;
	pos.xdesc += (pos.maxname != 0 ? 1 : 0);
	pos.line = MAX(pos.maxsepstr + 3, pos.xdesc + pos.maxdesc);

	m.menurows = menurows;
	get_buttons(conf, &bs, conf->menu.shortcut_buttons, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);
	if (menu_size_position(conf, rows, cols, text, &m.menurows, m.nitems,
	    pos.line, &bs, &y, &x, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs) != 0)
		return (BSDDIALOG_ERROR);

	doupdate();

	prefresh(textpad, 0, 0, y + 1, x + 1 + TEXTHMARGIN, y + h - m.menurows,
	    x + 1 + w - TEXTHMARGIN);

	m.box = new_boxed_window(conf, y + h - 5 - m.menurows, x + 2,
	    m.menurows + 2, w - 4, LOWERED);

	m.pad = newpad(m.nitems, pos.line);
	wbkgd(m.pad, t.dialog.color);

	if ((pritems = calloc(m.nitems, sizeof (struct privateitem))) == NULL)
		RETURN_ERROR("Cannot allocate memory for internal menu items");

	m.sel = 0;
	for (i = 0; i < (int)ngroups; i++) {
		onetrue = false;
		for (j = 0; j < (int)groups[i].nitems; j++) {
			item = &groups[i].items[j];

			if (getmode(mode, groups[i]) == MENUMODE) {
				pritems[m.sel].on = false;
			} else if (getmode(mode, groups[i]) == RADIOLISTMODE) {
				pritems[m.sel].on = onetrue ? false : item->on;
				if (pritems[m.sel].on)
					onetrue = true;
			} else {
				pritems[m.sel].on = item->on;
			}
			pritems[m.sel].group = i;
			pritems[m.sel].index = j;
			pritems[m.sel].type = getmode(mode, groups[i]);
			pritems[m.sel].apiitem = item;

			drawitem(conf, m.pad, m.sel, pos, &pritems[m.sel], false);
			m.sel++;
		}
	}
	drawseparators(conf, m.pad, MIN((int)pos.line, w-6), m.nitems,
	    pritems);
	m.sel = getfirst_with_default(m.nitems, pritems, ngroups, groups,
	    focuslist, focusitem);
	if (m.sel >= 0)
		drawitem(conf, m.pad, m.sel, pos, &pritems[m.sel], true);

	ys = y + h - 5 - m.menurows + 1;
	ye = y + h - 5 ;
	if (conf->menu.align_left || (int)pos.line > w - 6) {
		xs = x + 3;
		xe = xs + w - 7;
	} else { /* center */
		xs = x + 3 + (w-6)/2 - pos.line/2;
		xe = xs + w - 5;
	}

	m.ypad = 0;
	if ((int)(m.ypad + m.menurows) - 1 < m.sel)
		m.ypad = m.sel - m.menurows + 1;
	update_menuwin(conf, m.box, m.menurows+2, w-4, m.nitems, m.menurows, m.ypad);
	wrefresh(m.box);
	prefresh(m.pad, m.ypad, 0, ys, xs, ye, xe);

	movefocus = false;
	loop = true;
	while (loop) {
		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			retval = BUTTONVALUE(bs);
			if (m.sel >= 0 && pritems[m.sel].type == MENUMODE)
				pritems[m.sel].on = true;
			set_on_output(conf, retval, ngroups, groups, pritems);
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				if (m.sel >= 0 && pritems[m.sel].type == MENUMODE)
					pritems[m.sel].on = true;
				set_on_output(conf, retval, ngroups, groups,
				    pritems);
				loop = false;
			}
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			draw_buttons(widget, bs);
			wrefresh(widget);
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				draw_buttons(widget, bs);
				wrefresh(widget);
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				draw_buttons(widget, bs);
				wrefresh(widget);
			}
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

			m.menurows = menurows;
			if (menu_size_position(conf, rows, cols, text,
			    &m.menurows, m.nitems, pos.line, &bs, &y, &x, &h,
			    &w) != 0)
				return (BSDDIALOG_ERROR);
			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs) != 0)
				return (BSDDIALOG_ERROR);

			doupdate();

			prefresh(textpad, 0, 0, y + 1, x + 1 + TEXTHMARGIN,
			    y + h - m.menurows, x + 1 + w - TEXTHMARGIN);

			wclear(m.box);
			mvwin(m.box, y + h - 5 - m.menurows, x + 2);
			wresize(m.box, m.menurows+2, w-4);
			update_menuwin(conf, m.box, m.menurows+2, w-4,
			    m.nitems, m.menurows, m.ypad);
			wrefresh(m.box);

			ys = y + h - 5 - m.menurows + 1;
			ye = y + h - 5 ;
			if (conf->menu.align_left || (int)pos.line > w - 6) {
				xs = x + 3;
				xe = xs + w - 7;
			} else { /* center */
				xs = x + 3 + (w-6)/2 - pos.line/2;
				xe = xs + w - 5;
			}

			drawseparators(conf, m.pad, MIN((int)pos.line, w-6),
			    m.nitems, pritems);

			if ((int)(m.ypad + m.menurows) - 1 < m.sel)
				m.ypad = m.sel - m.menurows + 1;
			prefresh(m.pad, m.ypad, 0, ys, xs, ye, xe);

			refresh();

			break;
		}

		if (m.sel < 0)
			continue;
		switch(input) {
		case KEY_HOME:
			next = getnext(m.nitems, pritems, -1);
			movefocus = next != m.sel;
			break;
		case KEY_UP:
			next = getprev(pritems, m.sel);
			movefocus = next != m.sel;
			break;
		case KEY_PPAGE:
			next = getfastprev(m.menurows, pritems, m.sel);
			movefocus = next != m.sel;
			break;
		case KEY_END:
			next = getprev(pritems, m.nitems);
			movefocus = next != m.sel;
			break;
		case KEY_DOWN:
			next = getnext(m.nitems, pritems, m.sel);
			movefocus = next != m.sel;
			break;
		case KEY_NPAGE:
			next = getfastnext(m.menurows, m.nitems, pritems, m.sel);
			movefocus = next != m.sel;
			break;
		case ' ': /* Space */
			if (pritems[m.sel].type == MENUMODE) {
				retval = BUTTONVALUE(bs);
				pritems[m.sel].on = true;
				set_on_output(conf, retval, ngroups, groups,
				    pritems);
				loop = false;
			} else if (pritems[m.sel].type == CHECKLISTMODE) {
				pritems[m.sel].on = !pritems[m.sel].on;
			} else { /* RADIOLISTMODE */
				for (i = m.sel - pritems[m.sel].index;
				    i < m.nitems &&
				    pritems[i].group == pritems[m.sel].group;
				    i++) {
					if (i != m.sel && pritems[i].on) {
						pritems[i].on = false;
						drawitem(conf, m.pad, i, pos,
						    &pritems[i], false);
					}
				}
				pritems[m.sel].on = !pritems[m.sel].on;
			}
			drawitem(conf, m.pad, m.sel, pos, &pritems[m.sel], true);
			prefresh(m.pad, m.ypad, 0, ys, xs, ye, xe);
			break;
		default:
			if (conf->menu.shortcut_buttons) {
				if (shortcut_buttons(input, &bs)) {
					retval = BUTTONVALUE(bs);
					if (pritems[m.sel].type == MENUMODE)
						pritems[m.sel].on = true;
					set_on_output(conf, retval, ngroups,
					    groups, pritems);
					loop = false;
				}
				break;
			}

			/* shourtcut items */
			next = getnextshortcut(conf, m.nitems, pritems, m.sel,
			    input);
			movefocus = next != m.sel;
		} /* end switch handler */

		if (movefocus) {
			drawitem(conf, m.pad, m.sel, pos, &pritems[m.sel], false);
			m.sel = next;
			drawitem(conf, m.pad, m.sel, pos, &pritems[m.sel], true);
			if (m.ypad > m.sel && m.ypad > 0)
				m.ypad = m.sel;
			if ((int)(m.ypad + m.menurows) <= m.sel)
				m.ypad = m.sel - m.menurows + 1;
			update_menuwin(conf, m.box, m.menurows+2, w-4,
			    m.nitems, m.menurows, m.ypad);
			wrefresh(m.box);
			prefresh(m.pad, m.ypad, 0, ys, xs, ye, xe);
			movefocus = false;
		}
	} /* end while handler */

	if (focuslist != NULL)
		*focuslist = m.sel < 0 ? -1 : pritems[m.sel].group;
	if (focusitem !=NULL)
		*focusitem = m.sel < 0 ? -1 : pritems[m.sel].index;

	delwin(m.pad);
	delwin(m.box);
	end_dialog(conf, shadow, widget, textpad);
	free(pritems);

	return (retval);
}

/* API */
int
bsddialog_mixedlist(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, unsigned int ngroups,
    struct bsddialog_menugroup *groups, int *focuslist, int *focusitem)
{
	int retval;

	retval = do_mixedlist(conf, text, rows, cols, menurows, MIXEDLISTMODE,
	    ngroups, groups, focuslist, focusitem);

	return (retval);
}

int
bsddialog_checklist(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, unsigned int nitems,
    struct bsddialog_menuitem *items, int *focusitem)
{
	int retval, focuslist = 0;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_CHECKLIST /* unused */, nitems, items};

	retval = do_mixedlist(conf, text, rows, cols, menurows, CHECKLISTMODE,
	    1, &group, &focuslist, focusitem);

	return (retval);
}

int
bsddialog_menu(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, unsigned int nitems,
    struct bsddialog_menuitem *items, int *focusitem)
{
	int retval, focuslist = 0;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_CHECKLIST /* unused */, nitems, items};

	retval = do_mixedlist(conf, text, rows, cols, menurows, MENUMODE, 1,
	    &group, &focuslist, focusitem);

	return (retval);
}

int
bsddialog_radiolist(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int menurows, unsigned int nitems,
    struct bsddialog_menuitem *items, int *focusitem)
{
	int retval, focuslist = 0;
	struct bsddialog_menugroup group = {
	    BSDDIALOG_RADIOLIST /* unused */, nitems, items};

	retval = do_mixedlist(conf, text, rows, cols, menurows, RADIOLISTMODE,
	    1, &group, &focuslist, focusitem);

	return (retval);
}
