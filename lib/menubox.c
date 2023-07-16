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
#include <stdlib.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

enum menumode {
	CHECKLISTMODE,
	MENUMODE,
	MIXEDLISTMODE,
	RADIOLISTMODE,
	SEPARATORMODE
};

struct privateitem {
	struct bsddialog_menuitem *apiitem; // set on at end, ? 
	/* API */
	const char *prefix;
	//bool on;
	unsigned int depth;
	const char *name;
	const char *desc;
	const char *bottomdesc; //solo che può essere NULL
	/* menu fields */
	bool on;
	int group;//?
	int index;//?
	enum menumode type;
	//w_char shrtcut; to add
};

struct privatemenu {
	WINDOW *box;           /* only for borders */
	WINDOW *pad;           /* pad for the private items */
	int ypad;              /* start pad line */
	int ys, ye, xs, xe;    /* pad pos */
	unsigned int apimenurows;
	unsigned int menurows; /* real menurows after menu_size_position() */
	int nitems;            /* total nitems (all groups * all items) */
	struct privateitem *pritems;
	int sel;               /* current focus item, can be -1 */
	bool hasbottomdesc;
	/* immutable strings positions in pad, except SEPARATORMODE */
	unsigned int xselector; /* [ ] */
	unsigned int xname;     /* real x: xname + item.depth */
	unsigned int xdesc;     /* real x: xdesc + item.depth */
	unsigned int line;      /* wpad: prefix [ ] depth name desc */
};


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

static int
build_privatemenu(struct bsddialog_conf *conf, struct privatemenu *m,
    enum menumode mode, unsigned int ngroups,
    struct bsddialog_menugroup *groups)
{
	bool onetrue;
	int i, j, abs;
	unsigned int maxsepstr, maxprefix, selectorlen, maxdepth;
	unsigned int maxname, maxdesc;
	struct bsddialog_menuitem *item;

	/* nitems and fault checks */
	CHECK_ARRAY(ngroups, groups);
	m->nitems = 0;
	for (i = 0; i < (int)ngroups; i++) {
		CHECK_ARRAY(groups[i].nitems, groups[i].items);
		m->nitems += (int)groups[i].nitems;
	}

	/* alloc and set private items */ù
	m->pritems = calloc(m->nitems, sizeof (struct privateitem));
	if (m->pritems == NULL)
		RETURN_ERROR("Cannot allocate memory for internal menu items");
	m->hasbottomdesc = false;
	abs = 0;
	for (i = 0; i < (int)ngroups; i++) {
		onetrue = false;
		for (j = 0; j < (int)groups[i].nitems; j++) {
			item = &groups[i].items[j];

			if (getmode(mode, groups[i]) == MENUMODE) {
				m->pritems[abs].on = false;
			} else if (getmode(mode, groups[i]) == RADIOLISTMODE) {
				m->pritems[abs].on = onetrue ? false : item->on;
				if (m->pritems[abs].on)
					onetrue = true;
			} else { /* CHECKLISTMODE */
				m->pritems[abs].on = item->on;
			}
			m->pritems[abs].group = i;
			m->pritems[abs].index = j;
			m->pritems[abs].type = getmode(mode, groups[i]);
			m->pritems[abs].apiitem = item;

			m->pritems[abs].prefix = CHECK_STR(item->prefix);
			m->pritems[abs].depth = item->depth;
			m->pritems[abs].name = CHECK_STR(item->name);
			m->pritems[abs].desc = CHECK_STR(item->desc);
			m->pritems[abs].bottomdesc = CHECK_STR(item->bottomdesc);
			if (item->bottomdesc != NULL)
				m->hasbottomdesc = true;

			abs++;
		}
	}

	/* positions */
	m->xselector = m->xname = m->xdesc = m->line = 0;
	maxsepstr = maxprefix = selectorlen = maxdepth = maxname = maxdesc = 0;
	for (i = 0; i < m->nitems; i++) {
		if (m->pritems[i].type == RADIOLISTMODE ||
		    m->pritems[i].type == CHECKLISTMODE)
			selectorlen = 4;

		if (groups[i].type == BSDDIALOG_SEPARATOR) {
			maxsepstr = MAX(maxsepstr,
			    strcols(m->pritems[i].name) +
			    strcols(m->pritems[i].desc));
			continue;
		}

		maxprefix = MAX(maxprefix, strcols(m->pritems[i].prefix));
		maxdepth  = MAX(maxdepth, m->pritems[i].depth);
		maxname   = MAX(maxname, strcols(m->pritems[i].name));
		maxdesc   = MAX(maxdesc, strcols(m->pritems[i].desc));
	}
	maxname = conf->menu.no_name ? 0 : maxname;
	maxdesc = conf->menu.no_desc ? 0 : maxdesc;

	m->xselector = maxprefix + (maxprefix != 0 ? 1 : 0);
	m->xname = m->xselector + selectorlen;
	m->xdesc = maxdepth + m->xname + maxname;
	m->xdesc += (maxname != 0 ? 1 : 0);
	m->line = MAX(maxsepstr + 3, m->xdesc + maxdesc);

	return (0);
}

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
drawitem(struct bsddialog_conf *conf, struct privatemenu *m, int y, bool focus)
{
	int colordesc, colorname, colorshortcut;
	wchar_t shortcut;
	struct privateitem *pritem;

	pritem = &m->pritems[y];

	/* prefix */
	if (pritem->prefix != NULL && pritem->prefix[0] != '\0')
		mvwaddstr(m->pad, y, 0, pritem->prefix);

	/* selector */
	wmove(m->pad, y, m->xselector);
	wattron(m->pad, focus ? t.menu.f_selectorcolor : t.menu.selectorcolor);
	if (pritem->type == CHECKLISTMODE)
		wprintw(m->pad, "[%c]", pritem->on ? 'X' : ' ');
	if (pritem->type == RADIOLISTMODE)
		wprintw(m->pad, "(%c)", pritem->on ? '*' : ' ');
	wattroff(m->pad, focus ? t.menu.f_selectorcolor : t.menu.selectorcolor);

	/* name */
	colorname = focus ? t.menu.f_namecolor : t.menu.namecolor;
	if (conf->menu.no_name == false) {
		wattron(m->pad, colorname);
		mvwaddstr(m->pad, y, m->xname + pritem->depth, pritem->name);
		wattroff(m->pad, colorname);
	}

	/* description */
	if (conf->menu.no_name)
		colordesc = focus ? t.menu.f_namecolor : t.menu.namecolor;
	else
		colordesc = focus ? t.menu.f_desccolor : t.menu.desccolor;

	if (conf->menu.no_desc == false) {
		wattron(m->pad, colordesc);
		if (conf->menu.no_name)
			mvwaddstr(m->pad, y, m->xname + pritem->depth, pritem->desc);
		else
			mvwaddstr(m->pad, y, m->xdesc, pritem->desc);
		wattroff(m->pad, colordesc);
	}

	/* shortcut */
	if (conf->menu.shortcut_buttons == false) {
		colorshortcut = focus ?
		    t.menu.f_shortcutcolor : t.menu.shortcutcolor;
		wattron(m->pad, colorshortcut);

		if (conf->menu.no_name)
			mbtowc(&shortcut, pritem->desc, MB_CUR_MAX);
		else
			mbtowc(&shortcut, pritem->name, MB_CUR_MAX);
		mvwaddwch(m->pad, y, m->xname + pritem->depth, shortcut);
		wattroff(m->pad, colorshortcut);
	}

	/* bottom description */
	if (m->hasbottomdesc) {
		move(SCREENLINES - 1, 2);
		clrtoeol();
		if (focus) {
			attron(t.menu.bottomdesccolor);
			addstr(pritem->bottomdesc);
			attroff(t.menu.bottomdesccolor);
			refresh();
		}
	}
}

/* the caller has to call prefresh(menupad, ymenupad, 0, ys, xs, ye, xe); */
static void
update_menubox(struct bsddialog_conf *conf, WINDOW *menuwin, int h, int w,
    int totnitems, unsigned int menurows, int ymenupad)
{
	draw_borders(conf, menuwin, LOWERED);

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

static int
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

static int
mixedlist_redraw(struct dialog *d, struct privatemenu *m, struct privateitem *pritems)
{
	if (d->built) {
		hide_dialog(d);
		refresh(); /* Important for decreasing screen */
	}
	m->menurows = m->apimenurows;
	if (menu_size_position(d->conf, d->rows, d->cols, d->text,
	    &m->menurows, m->nitems, m->line, &d->bs, &d->y, &d->x, &d->h,
	    &d->w) != 0)
		return (BSDDIALOG_ERROR);
	if (draw_dialog(d) != 0)
		return (BSDDIALOG_ERROR);
	if (d->built)
		refresh(); /* Important to fix grey lines expanding screen */
	TEXTPAD(d, 2/*bmenu*/ + m->menurows + HBUTTONS);

	update_box(d->conf, m->box, d->y + d->h - 5 - m->menurows, d->x + 2,
	    m->menurows+2, d->w-4, LOWERED);
	update_menubox(d->conf, m->box, m->menurows+2, d->w-4, m->nitems,
	    m->menurows, m->ypad);
	wnoutrefresh(m->box);

	if (m->sel > 0)
		drawitem(d->conf, m, m->sel, true);
	drawseparators(d->conf, m->pad, MIN((int)m->line, d->w-6),
	    m->nitems, pritems);
	if ((int)(m->ypad + m->menurows) - 1 < m->sel)
		m->ypad = m->sel - m->menurows + 1;
	m->ys = d->y + d->h - 5 - m->menurows + 1;
	m->ye = d->y + d->h - 5 ;
	if (d->conf->menu.align_left || (int)m->line > d->w - 6) {
		m->xs = d->x + 3;
		m->xe = m->xs + d->w - 7;
	} else { /* center */
		m->xs = d->x + 3 + (d->w-6)/2 - m->line/2;
		m->xe = m->xs + d->w - 5;
	}
	pnoutrefresh(m->pad, m->ypad, 0, m->ys, m->xs, m->ye, m->xe);

	//refresh();//?

	return (0);
}

static int
do_mixedlist(struct bsddialog_conf *conf, const char *text, int rows, int cols,
    unsigned int menurows, enum menumode mode, unsigned int ngroups,
    struct bsddialog_menugroup *groups, int *focuslist, int *focusitem)
{
	bool loop, changeitem;
	int i, next, retval;
	wint_t input;
	struct privatemenu m;
	struct dialog d;

	if (prepare_dialog(conf, text, rows, cols, &d) != 0)
		return (BSDDIALOG_ERROR);
	set_buttons(&d, conf->menu.shortcut_buttons, OK_LABEL, CANCEL_LABEL);

	if (d.conf->menu.no_name && d.conf->menu.no_desc)
		RETURN_ERROR("Both conf.menu.no_name and conf.menu.no_desc");

	if (build_privatemenu(conf, &m, mode, ngroups, groups) != 0)
		return (BSDDIALOG_ERROR);

	if ((m.box = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW box menu");
	wbkgd(m.box, t.dialog.color);

	m.pad = newpad(m.nitems, m.line);
	wbkgd(m.pad, t.dialog.color);

	for (i = 0; i < m.nitems; i++)
		drawitem(conf, &m, i, false);
	m.sel = getfirst_with_default(m.nitems, m.pritems, ngroups, groups,
	    focuslist, focusitem);
	m.ypad = 0;
	m.apimenurows = menurows;
	if (mixedlist_redraw(&d, &m, m.pritems) != 0)
		return (BSDDIALOG_ERROR);

	changeitem = false;
	loop = true;
	while (loop) {
		doupdate(); //buttons and mixedmenu_redraw() for now
		if (get_wch(&input) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			retval = BUTTONVALUE(d.bs);
			if (m.sel >= 0 && m.pritems[m.sel].type == MENUMODE)
				m.pritems[m.sel].on = true;
			set_on_output(conf, retval, ngroups, groups, m.pritems);
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = BSDDIALOG_ESC;
				if (m.sel >= 0 && m.pritems[m.sel].type == MENUMODE)
					m.pritems[m.sel].on = true;
				set_on_output(conf, retval, ngroups, groups,
				    m.pritems);
				loop = false;
			}
			break;
		case '\t': /* TAB */
			d.bs.curr = (d.bs.curr + 1) % d.bs.nbuttons;
			DRAW_BUTTONS(d);
			break;
		case KEY_LEFT:
			if (d.bs.curr > 0) {
				d.bs.curr--;
				DRAW_BUTTONS(d);
			}
			break;
		case KEY_RIGHT:
			if (d.bs.curr < (int) d.bs.nbuttons - 1) {
				d.bs.curr++;
				DRAW_BUTTONS(d);
			}
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			if (f1help_dialog(conf) != 0)
				return (BSDDIALOG_ERROR);
			if (mixedlist_redraw(&d, &m, m.pritems) != 0)
				return (BSDDIALOG_ERROR);
			break;
		case KEY_RESIZE:
			if (mixedlist_redraw(&d, &m, m.pritems) != 0)
				return (BSDDIALOG_ERROR);
			break;
		}

		if (m.sel < 0)
			continue;
		switch(input) {
		case KEY_HOME:
			next = getnext(m.nitems, m.pritems, -1);
			changeitem = next != m.sel;
			break;
		case KEY_UP:
			next = getprev(m.pritems, m.sel);
			changeitem = next != m.sel;
			break;
		case KEY_PPAGE:
			next = getfastprev(m.menurows, m.pritems, m.sel);
			changeitem = next != m.sel;
			break;
		case KEY_END:
			next = getprev(m.pritems, m.nitems);
			changeitem = next != m.sel;
			break;
		case KEY_DOWN:
			next = getnext(m.nitems, m.pritems, m.sel);
			changeitem = next != m.sel;
			break;
		case KEY_NPAGE:
			next = getfastnext(m.menurows, m.nitems, m.pritems, m.sel);
			changeitem = next != m.sel;
			break;
		case ' ': /* Space */
			if (m.pritems[m.sel].type == MENUMODE) {
				retval = BUTTONVALUE(d.bs);
				m.pritems[m.sel].on = true;
				set_on_output(conf, retval, ngroups, groups,
				    m.pritems);
				loop = false;
			} else if (m.pritems[m.sel].type == CHECKLISTMODE) {
				m.pritems[m.sel].on = !m.pritems[m.sel].on;
			} else { /* RADIOLISTMODE */
				for (i = m.sel - m.pritems[m.sel].index;
				    i < m.nitems &&
				    m.pritems[i].group == m.pritems[m.sel].group;
				    i++) {
					if (i != m.sel && m.pritems[i].on) {
						m.pritems[i].on = false;
						drawitem(conf, &m, i, false);
					}
				}
				m.pritems[m.sel].on = !m.pritems[m.sel].on;
			}
			drawitem(conf, &m, m.sel, true);
			prefresh(m.pad, m.ypad, 0, m.ys, m.xs, m.ye, m.xe);
			break;
		default:
			if (conf->menu.shortcut_buttons) {
				if (shortcut_buttons(input, &d.bs)) {
					retval = BUTTONVALUE(d.bs);
					if (m.pritems[m.sel].type == MENUMODE)
						m.pritems[m.sel].on = true;
					set_on_output(conf, retval, ngroups,
					    groups, m.pritems);
					loop = false;
				}
				break;
			}

			/* shourtcut items */
			next = getnextshortcut(conf, m.nitems, m.pritems, m.sel,
			    input);
			changeitem = next != m.sel;
		} /* end switch get_wch() */

		if (changeitem) {
			drawitem(conf, &m, m.sel, false);
			m.sel = next;
			drawitem(conf, &m, m.sel, true);
			if (m.ypad > m.sel && m.ypad > 0)
				m.ypad = m.sel;
			if ((int)(m.ypad + m.menurows) <= m.sel)
				m.ypad = m.sel - m.menurows + 1;
			update_menubox(conf, m.box, m.menurows+2, d.w-4,
			    m.nitems, m.menurows, m.ypad);
			wrefresh(m.box);
			prefresh(m.pad, m.ypad, 0, m.ys, m.xs, m.ye, m.xe);
			changeitem = false;
		}
	} /* end while(loop) */

	if (focuslist != NULL)
		*focuslist = m.sel < 0 ? -1 : m.pritems[m.sel].group;
	if (focusitem !=NULL)
		*focusitem = m.sel < 0 ? -1 : m.pritems[m.sel].index;

	if (m.hasbottomdesc) {
		move(SCREENLINES - 1, 2);
		clrtoeol();
	}
	delwin(m.pad);
	delwin(m.box);
	end_dialog(&d);
	free(m.pritems);

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
