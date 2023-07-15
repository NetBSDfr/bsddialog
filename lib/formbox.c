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
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

struct privateitem {
	const char *label;      /* formitem.label */
	unsigned int ylabel;    /* formitem.ylabel */
	unsigned int xlabel;    /* formitem.xlabel */
	unsigned int yfield;    /* formitem.yfield */
	unsigned int xfield;    /* formitem.xfield */
	bool secure;            /* formitem.flags & BSDDIALOG_FIELDHIDDEN */
	bool readonly;          /* formitem.flags & BSDDIALOG_FIELDREADONLY */
	bool fieldnocolor;      /* formitem.flags & BSDDIALOG_FIELDNOCOLOR */
	bool extendfield;       /* formitem.flags & BSDDIALOG_FIELDEXTEND */
	bool fieldonebyte;      /* formitem.flags & BSDDIALOG_FIELDSINGLEBYTE */
	bool cursorend;         /* formitem.flags & BSDDIALOG_FIELDCURSOREND */
	bool cursor;            /* field cursor visibility */
	const char *bottomdesc; /* formitem.bottomdesc */

	wchar_t *privwbuf;       /* formitem.value */
	wchar_t *pubwbuf;        /* string for drawitem() */
	unsigned int maxletters; /* formitem.maxvaluelen, [priv|pub]wbuf size */
	unsigned int nletters;   /* letters in privwbuf and pubwbuf */
	unsigned int pos;        /* pos in privwbuf and pubwbuf */
	unsigned int fieldcols;  /* formitem.fieldlen */
	unsigned int xcursor;    /* position in fieldcols [0 - fieldcols-1] */
	unsigned int xposdraw;   /* first pubwbuf index to draw */
};

struct privateform {
	WINDOW *box;       /* window to draw borders */
	WINDOW *pad;
	unsigned int h;    /* only to create pad */
	unsigned int w;    /* only to create pad */
	unsigned int wmin; /* to refresh, w can change for FIELDEXTEND */
	unsigned int ys;   /* to refresh */
	unsigned int ye;   /* to refresh */
	unsigned int xs;   /* to refresh */
	unsigned int xe;   /* to refresh */
	unsigned int y;    /* changes moving focus around items */
	unsigned int formheight;  /* API formheight */
	unsigned int viewrows;    /* visible rows, real formheight */
	unsigned int minviewrows; /* min viewrows, ylabel != yfield */
	wchar_t securewch; /* wide char of conf.form.secure[mb]ch */
};

enum operation {
	MOVE_CURSOR_BEGIN,
	MOVE_CURSOR_END,
	MOVE_CURSOR_RIGHT,
	MOVE_CURSOR_LEFT,
	DEL_LETTER
};

static bool fieldctl(struct privateitem *item, enum operation op)
{
	bool change;
	int width, oldwidth, nextwidth, cols;
	unsigned int i;

	change = false;
	switch (op){
	case MOVE_CURSOR_BEGIN:
		if (item->pos == 0 && item->xcursor == 0)
			break;
		/* here the cursor is changed */
		change = true;
		item->pos = 0;
		item->xcursor = 0;
		item->xposdraw = 0;
		break;
	case MOVE_CURSOR_END:
		while (fieldctl(item, MOVE_CURSOR_RIGHT))
			change = true;
		break;
	case MOVE_CURSOR_LEFT:
		if (item->pos == 0)
			break;
		/* check redundant by item->pos == 0 because of 'while' below */
		if (item->xcursor == 0 && item->xposdraw == 0)
			break;
		/* here some letter to left */
		change = true;
		item->pos -= 1;
		width = wcwidth(item->pubwbuf[item->pos]);
		if (((int)item->xcursor) - width < 0) {
			item->xcursor = 0;
			item->xposdraw -= 1;
		} else
			item->xcursor -= width;

		while (true) {
			if (item->xposdraw == 0)
				break;
			if (item->xcursor >= item->fieldcols / 2)
				break;
			if (wcwidth(item->pubwbuf[item->xposdraw - 1]) +
			    item->xcursor + width > item->fieldcols)
				break;

			item->xposdraw -= 1;
			item->xcursor +=
			    wcwidth(item->pubwbuf[item->xposdraw]);
		}
		break;
	case DEL_LETTER:
		if (item->nletters == 0)
			break;
		if (item->pos == item->nletters)
			break;
		/* here a letter under the cursor */
		change = true;
		for (i = item->pos; i < item->nletters; i++) {
			item->privwbuf[i] = item->privwbuf[i+1];
			item->pubwbuf[i] = item->pubwbuf[i+1];
		}
		item->nletters -= 1;
		item->privwbuf[i] = L'\0';
		item->pubwbuf[i] = L'\0';
		break;
	case MOVE_CURSOR_RIGHT: /* used also by "insert", see handler loop */
		if (item->pos + 1 == item->maxletters)
			break;
		if (item->pos == item->nletters)
			break;
		/* here a change to right */
		change = true;
		oldwidth = wcwidth(item->pubwbuf[item->pos]);
		item->pos += 1;
		if (item->pos == item->nletters) { /* empty column */
			nextwidth = 1;
		} else { /* a letter to right */
			nextwidth = wcwidth(item->pubwbuf[item->pos]);
		}
		if (item->xcursor + oldwidth + nextwidth - 1 >= item->fieldcols) {
			cols = nextwidth;
			item->xposdraw = item->pos;
			while (item->xposdraw != 0) {
				cols += wcwidth(item->pubwbuf[item->xposdraw - 1]);
				if (cols > (int)item->fieldcols)
					break;
				item->xposdraw -= 1;
			}
			item->xcursor = 0;
			for (i = item->xposdraw; i < item->pos ; i++)
				item->xcursor += wcwidth(item->pubwbuf[i]);
		}
		else {
			item->xcursor += oldwidth;
		}

		break;
	}

	return (change);
}

static void
drawitem(struct privateform *form, struct privateitem *item, bool focus,
    bool refresh)
{
	int color;
	unsigned int n, cols;

	/* Label */
	wattron(form->pad, t.dialog.color);
	mvwaddstr(form->pad, item->ylabel, item->xlabel, item->label);
	wattroff(form->pad, t.dialog.color);

	/* Field */
	if (item->readonly)
		color = t.form.readonlycolor;
	else if (item->fieldnocolor)
		color = t.dialog.color;
	else
		color = focus ? t.form.f_fieldcolor : t.form.fieldcolor;
	wattron(form->pad, color);
	mvwhline(form->pad, item->yfield, item->xfield, ' ', item->fieldcols);
	n = 0;
	cols = wcwidth(item->pubwbuf[item->xposdraw]);
	while (cols <= item->fieldcols &&
	    item->xposdraw + n < wcslen(item->pubwbuf)) {
		n++;
		cols += wcwidth(item->pubwbuf[item->xposdraw + n]);

	}
	mvwaddnwstr(form->pad, item->yfield, item->xfield,
	    &item->pubwbuf[item->xposdraw], n);
	wattroff(form->pad, color);

	/* Bottom Desc */
	move(SCREENLINES - 1, 2);
	clrtoeol();
	if (item->bottomdesc != NULL && focus) {
		attron(t.form.bottomdesccolor);
		addstr(item->bottomdesc);
		attroff(t.form.bottomdesccolor);
		refresh();
	}

	/* Cursor */
	curs_set((focus && item->cursor) ? 1 : 0);
	wmove(form->pad, item->yfield, item->xfield + item->xcursor);

	if(refresh)
		prefresh(form->pad, form->y, 0, form->ys, form->xs, form->ye,
		    form->xe);
}

/*
 * Trick: draw 2 times an item switching focus.
 * Problem: curses tries to optimize the rendering but sometimes it misses some
 * updates or draws old stuff. libformw has a similar problem fixed by the
 * same trick.
 * Case 1: KEY_DC and KEY_BACKSPACE, deleted multicolumn letters are drawn
 * again. It seems fixed by new items pad and prefresh(), previously WINDOW.
 * Case2: some terminal, tmux and ssh does not show the cursor.
 */
#define DRAWITEM_TRICK(form, item, focus) do {                                 \
	drawitem(form, item, !focus, true);                                    \
	drawitem(form, item, focus, true);                                     \
} while (0)

static bool
insertch(struct privateform *form, struct privateitem *item, wchar_t wch)
{
	int i;

	if (item->nletters >= item->maxletters)
		return (false);

	for (i = (int)item->nletters - 1; i >= (int)item->pos; i--) {
		item->privwbuf[i+1] = item->privwbuf[i];
		item->pubwbuf[i+1] = item->pubwbuf[i];
	}

	item->privwbuf[item->pos] = wch;
	item->pubwbuf[item->pos] = item->secure ? form->securewch : wch;
	item->nletters += 1;
	item->privwbuf[item->nletters] = L'\0';
	item->pubwbuf[item->nletters] = L'\0';

	return (true);
}

static char* alloc_wstomb(wchar_t *wstr)
{
	int len, nbytes, i;
	char mbch[MB_LEN_MAX], *mbstr;

	nbytes = MB_LEN_MAX; /* to ensure a null terminated string */
	len = wcslen(wstr);
	for (i = 0; i < len; i++) {
		wctomb(mbch, wstr[i]);
		nbytes += mblen(mbch, MB_LEN_MAX);
	}
	if((mbstr = malloc(nbytes)) == NULL)
		return (NULL);

	wcstombs(mbstr,	wstr, nbytes);

	return (mbstr);
}

static int
return_values(struct bsddialog_conf *conf, int output, int nitems,
    struct bsddialog_formitem *apiitems, struct privateitem *items)
{
	int i;

	if (output != BSDDIALOG_OK && conf->form.value_without_ok == false)
		return (output);

	for (i = 0; i < nitems; i++) {
		if (conf->form.value_wchar) {
			apiitems[i].value = (char*)wcsdup(items[i].privwbuf);
		} else {
			apiitems[i].value = alloc_wstomb(items[i].privwbuf);
		}
		if (apiitems[i].value == NULL)
			RETURN_ERROR("Cannot allocate memory for form value");
	}

	return (output);
}

static unsigned int firstitem(unsigned int nitems, struct privateitem *items)
{
	int i;

	for (i = 0; i < (int)nitems; i++)
		if (items[i].readonly == false)
			break;

	return (i);
}

static unsigned int lastitem(unsigned int nitems, struct privateitem *items)
{
	int i;

	for (i = nitems - 1; i >= 0 ; i--)
		if (items[i].readonly == false)
			break;

	return (i);
}

static unsigned int
previtem(unsigned int nitems, struct privateitem *items, int curritem)
{
	int i;

	for (i = curritem - 1; i >= 0; i--)
		if (items[i].readonly == false)
			return(i);

	for (i = nitems - 1; i > curritem - 1; i--)
		if (items[i].readonly == false)
			return(i);

	return (curritem);
}

static unsigned int
nextitem(unsigned int nitems, struct privateitem *items, int curritem)
{
	int i;

	for (i = curritem + 1; i < (int)nitems; i++)
		if (items[i].readonly == false)
			return(i);

	for (i = 0; i < curritem; i++)
		if (items[i].readonly == false)
			return(i);

	return (curritem);
}

static void redrawbuttons(struct dialog *d, bool focus, bool shortcut)
{
	int selected;

	selected = d->bs.curr;
	if (focus == false)
		d->bs.curr = -1;
	d->bs.shortcut = shortcut;
	draw_buttons(d);
	wrefresh(d->widget);
	d->bs.curr = selected;
}

static void
update_formbox(struct bsddialog_conf *conf, struct privateform *form)
{
	int h, w;

	getmaxyx(form->box, h, w);
	draw_borders(conf, form->box, LOWERED);

	if (form->viewrows < form->h) {
		wattron(form->box, t.dialog.arrowcolor);
		if (form->y > 0)
			mvwhline(form->box, 0, (w / 2) - 2,
			    conf->ascii_lines ? '^' : ACS_UARROW, 5);

		if (form->y + form->viewrows < form->h)
			mvwhline(form->box, h-1, (w / 2) - 2,
			    conf->ascii_lines ? 'v' : ACS_DARROW, 5);
		wattroff(form->box, t.dialog.arrowcolor);
		wrefresh(form->box);
	}
}

static void curriteminview(struct privateform *form, struct privateitem *item)
{
	unsigned int yup, ydown;

	yup = MIN(item->ylabel, item->yfield);
	ydown = MAX(item->ylabel, item->yfield);

	if (form->y > yup && form->y > 0)
		form->y = yup;
	if ((int)(form->y + form->viewrows) - 1 < (int)ydown)
		form->y = ydown - form->viewrows + 1;
}

static int
form_size_position(struct bsddialog_conf *conf, int rows, int cols,
    const char *text, struct privateform *f, struct buttons *bs, int *y, int *x,
    int *h, int *w)
{
	int htext, hform;

	if (set_widget_size(conf, rows, cols, h, w) != 0)
		return (BSDDIALOG_ERROR);

	/* autosize */
	hform = (int) f->viewrows;
	if (f->viewrows == BSDDIALOG_AUTOSIZE)
		hform = MAX(f->w, f->minviewrows);
	hform += 2; /* formborders */

	if (set_widget_autosize(conf, rows, cols, h, w, text, &htext, bs, hform,
	    f->w + 4) != 0)
		return (BSDDIALOG_ERROR);
	/* formheight: avoid overflow, "at most" and at least minviewrows */
	if (*h - BORDERS - htext - HBUTTONS < 2 + (int)f->minviewrows) {
		f->viewrows = f->minviewrows; /* for widget_checksize() */
	} else if (f->viewrows == BSDDIALOG_AUTOSIZE) {
		f->viewrows = MIN(*h - BORDERS - htext - HBUTTONS, hform) - 2;
		f->viewrows = MAX(f->viewrows, f->minviewrows);
	} else {
		f->viewrows = MIN(*h - BORDERS - htext - HBUTTONS, hform) - 2;
	}

	/* checksize */
	if (f->viewrows < f->minviewrows)
		RETURN_FMTERROR("formheight, current: %u needed at least %u",
		    f->viewrows, f->minviewrows);
	if (widget_checksize(*h, *w, bs, 2 /* borders */ + f->minviewrows,
	    f->w + 6) != 0)
		return (BSDDIALOG_ERROR);

	if (set_widget_position(conf, y, x, *h, *w) != 0)
		return (BSDDIALOG_ERROR);

	return (0);
}

static int
form_redraw(struct dialog *d, struct privateform *f, unsigned int nitems,
    struct privateitem *items, int curritem, bool focusinform,
    struct privateitem *item)
{
	unsigned int i;

	if (d->built) {
		hide_dialog(d);
		refresh(); /* Important for decreasing screen */
	}
	f->viewrows = f->formheight;
	f->w = f->wmin;
	if (form_size_position(d->conf, d->rows, d->cols, d->text, f, &d->bs,
	    &d->y, &d->x, &d->h, &d->w) != 0)
		return (BSDDIALOG_ERROR);
	d->bs.shortcut = true; /* to check if useful, redrawbuttons() after */
	if (draw_dialog(d) != 0)
		return (BSDDIALOG_ERROR);
	if (d->built)
		refresh(); /* Important to fix grey lines expanding screen */
	TEXTPAD(d, 2/*bform*/ + f->viewrows + HBUTTONS);

	doupdate();// ?

	update_box(d->conf, f->box, d->y + d->h - 5 - f->viewrows, d->x + 2,
	    f->viewrows + 2, d->w - 4, LOWERED);

	for (i = 0; i < nitems; i++) {
		fieldctl(&items[i], MOVE_CURSOR_BEGIN);
		if (items[i].extendfield) {
			f->w = d->w - 6;
			items[i].fieldcols = f->w - items[i].xfield;
		}
		if (items[i].cursorend)
			fieldctl(&items[i], MOVE_CURSOR_END);
		drawitem(f, &items[i], false, false);
	}

	wresize(f->pad, f->h, f->w);
	for (i = 0; i < nitems; i++)
		drawitem(f, &items[i], false, false);

	f->ys = d->y + d->h - 5 - f->viewrows + 1;
	f->ye = d->y + d->h - 5 ;
	if ((int)f->w >= d->w - 6) { /* left */
		f->xs = d->x + 3;
		f->xe = f->xs + d->w - 7;
	} else { /* center */
		f->xs = d->x + 3 + (d->w - 6)/2 - f->w/2;
		f->xe = f->xs + d->w - 5;
	}

	if (curritem != -1) {
		redrawbuttons(d,
		    d->conf->button.always_active || !focusinform,
		    !focusinform);
		curriteminview(f, item);
		update_formbox(d->conf, f);
		wrefresh(f->box);
		DRAWITEM_TRICK(f, item, focusinform);
	} else {
		wrefresh(f->box);
	}

	return (0);
}

/* API */
int
bsddialog_form(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int formheight, unsigned int nitems,
    struct bsddialog_formitem *apiitems)
{
	bool switchfocus, changeitem, focusinform, insecurecursor, loop;
	int curritem, mbchsize, next, retval, wchtype;
	unsigned int i, j, itemybeg, itemxbeg, tmp;
	wchar_t *winit;
	wint_t input;
	struct privateitem *items, *item;
	struct privateform form;
	struct dialog d;

	for (i = 0; i < nitems; i++) {
		if (apiitems[i].maxvaluelen == 0)
			RETURN_ERROR("maxvaluelen cannot be zero");
		if (apiitems[i].fieldlen == 0)
			RETURN_ERROR("fieldlen cannot be zero");
	}

	insecurecursor = false;
	if (conf->form.securembch != NULL) {
		mbchsize = mblen(conf->form.securembch, MB_LEN_MAX);
		if(mbtowc(&form.securewch, conf->form.securembch, mbchsize) < 0)
			RETURN_ERROR("Cannot convert securembch to wchar_t");
		insecurecursor = true;
	} else if (conf->form.securech != '\0') {
		form.securewch = btowc(conf->form.securech);
		insecurecursor = true;
	} else {
		form.securewch = L' ';
	}

	if ((items = malloc(nitems * sizeof(struct privateitem))) == NULL)
		RETURN_ERROR("Cannot allocate internal items");
	form.h = form.w = form.minviewrows = 0;
	for (i = 0; i < nitems; i++) {
		item = &items[i];
		item->label = apiitems[i].label;
		item->ylabel = apiitems[i].ylabel;
		item->xlabel = apiitems[i].xlabel;
		item->yfield = apiitems[i].yfield;
		item->xfield = apiitems[i].xfield;
		item->secure = apiitems[i].flags & BSDDIALOG_FIELDHIDDEN;
		item->readonly = apiitems[i].flags & BSDDIALOG_FIELDREADONLY;
		item->fieldnocolor = apiitems[i].flags & BSDDIALOG_FIELDNOCOLOR;
		item->extendfield = apiitems[i].flags & BSDDIALOG_FIELDEXTEND;
		item->fieldonebyte = apiitems[i].flags &
		    BSDDIALOG_FIELDSINGLEBYTE;
		item->cursorend = apiitems[i].flags & BSDDIALOG_FIELDCURSOREND;
		item->bottomdesc = apiitems[i].bottomdesc;
		if (item->readonly || (item->secure && !insecurecursor))
			item->cursor = false;
		else
			item->cursor = true;

		item->maxletters = apiitems[i].maxvaluelen;
		item->privwbuf = calloc(item->maxletters + 1, sizeof(wchar_t));
		if (item->privwbuf == NULL)
			RETURN_ERROR("Cannot allocate item private buffer");
		memset(item->privwbuf, 0, item->maxletters + 1);
		item->pubwbuf = calloc(item->maxletters + 1, sizeof(wchar_t));
		if (item->pubwbuf == NULL)
			RETURN_ERROR("Cannot allocate item private buffer");
		memset(item->pubwbuf, 0, item->maxletters + 1);

		if ((winit = alloc_mbstows(apiitems[i].init)) == NULL)
			RETURN_ERROR("Cannot allocate item.init in wchar_t*");
		wcsncpy(item->privwbuf, winit, item->maxletters);
		wcsncpy(item->pubwbuf, winit, item->maxletters);
		free(winit);
		item->nletters = wcslen(item->pubwbuf);
		if (item->secure) {
			for (j = 0; j < item->nletters; j++)
				item->pubwbuf[j] = form.securewch;
		}

		item->fieldcols = apiitems[i].fieldlen;
		item->xposdraw = 0;
		item->xcursor = 0;
		item->pos = 0;

		form.h = MAX(form.h, items[i].ylabel);
		form.h = MAX(form.h, items[i].yfield);
		form.w = MAX(form.w, items[i].xlabel + strcols(items[i].label));
		form.w = MAX(form.w, items[i].xfield + items[i].fieldcols);
		if (i == 0) {
			itemybeg = MIN(items[i].ylabel, items[i].yfield);
			itemxbeg = MIN(items[i].xlabel, items[i].xfield);
		} else {
			tmp = MIN(items[i].ylabel, items[i].yfield);
			itemybeg = MIN(itemybeg, tmp);
			tmp = MIN(items[i].xlabel, items[i].xfield);
			itemxbeg = MIN(itemxbeg, tmp);
		}
		tmp = abs((int)items[i].ylabel - (int)items[i].yfield);
		form.minviewrows = MAX(form.minviewrows, tmp);
	}
	if (nitems > 0) {
		form.h = form.h + 1 - itemybeg;
		form.w -= itemxbeg;
		form.minviewrows += 1;
	}
	form.wmin = form.w;
	for (i = 0; i < nitems; i++) {
		items[i].ylabel -= itemybeg;
		items[i].yfield -= itemybeg;
		items[i].xlabel -= itemxbeg;
		items[i].xfield -= itemxbeg;
	}

	if (prepare_dialog(conf, text, rows, cols, &d) != 0)
		return (BSDDIALOG_ERROR);
	set_buttons(&d, true, OK_LABEL, CANCEL_LABEL);

	if ((form.box = newwin(1, 1, 1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW menu box");
	wbkgd(form.box, t.dialog.color);

	if ((form.pad = newpad(1, 1)) == NULL)
		RETURN_ERROR("Cannot build WINDOW menu box");
	wbkgd(form.pad, t.dialog.color);

	curritem = -1;
	for (i=0 ; i < nitems; i++) {
		if (curritem == -1 && items[i].readonly == false)
			curritem = i;
	}

	if (curritem != -1) {
		focusinform = true;
		form.y = 0;
		item = &items[curritem];
	} else {
		item = NULL;
		focusinform = false;
	}

	form.formheight = formheight;
	if (form_redraw(&d, &form, nitems, items, curritem, focusinform,
	    item) != 0)
		return (BSDDIALOG_ERROR);

	changeitem = switchfocus = false;
	loop = true;
	while (loop) {
		if ((wchtype = get_wch(&input)) == ERR)
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			if (focusinform && conf->button.always_active == false)
				break;
			retval = return_values(conf, BUTTONVALUE(d.bs),
			    nitems, apiitems, items);
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				retval = return_values(conf, BSDDIALOG_ESC,
				    nitems, apiitems, items);
				loop = false;
			}
			break;
		case '\t': /* TAB */
			if (focusinform) {
				switchfocus = true;
			} else {
				if (d.bs.curr + 1 < (int)d.bs.nbuttons) {
					d.bs.curr++;
				} else {
					d.bs.curr = 0;
					if (curritem != -1) {
						switchfocus = true;
					}
				}
				redrawbuttons(&d, true, true);
				wrefresh(d.widget);
			}
			break;
		case KEY_LEFT:
			if (focusinform) {
				if(fieldctl(item, MOVE_CURSOR_LEFT))
					DRAWITEM_TRICK(&form, item, true);
			} else if (d.bs.curr > 0) {
				d.bs.curr--;
				redrawbuttons(&d, true, true);
				wrefresh(d.widget);
			} else if (curritem != -1) {
				switchfocus = true;
			}
			break;
		case KEY_RIGHT:
			if (focusinform) {
				if(fieldctl(item, MOVE_CURSOR_RIGHT))
					DRAWITEM_TRICK(&form, item, true);
			} else if (d.bs.curr < (int) d.bs.nbuttons - 1) {
				d.bs.curr++;
				redrawbuttons(&d, true, true);
				wrefresh(d.widget);
			} else if (curritem != -1) {
				switchfocus = true;
			}
			break;
		case KEY_UP:
			if (focusinform) {
				next = previtem(nitems, items, curritem);
				changeitem = curritem != next;
			} else if (curritem != -1) {
				switchfocus = true;
			}
			break;
		case KEY_DOWN:
			if (focusinform == false)
				break;
			if (nitems == 1) {
				switchfocus = true;
			} else {
				next = nextitem(nitems, items, curritem);
				changeitem = curritem != next;
			}
			break;
		case KEY_PPAGE:
			if (focusinform) {
				next = firstitem(nitems, items);
				changeitem = curritem != next;
			}
			break;
		case KEY_NPAGE:
			if (focusinform) {
				next = lastitem(nitems, items);
				changeitem = curritem != next;
			}
			break;
		case KEY_BACKSPACE:
		case 127: /* Backspace */
			if (focusinform == false)
				break;
			if(fieldctl(item, MOVE_CURSOR_LEFT))
				if(fieldctl(item, DEL_LETTER))
					DRAWITEM_TRICK(&form, item, true);
			break;
		case KEY_DC:
			if (focusinform == false)
				break;
			if(fieldctl(item, DEL_LETTER))
				DRAWITEM_TRICK(&form, item, true);
			break;
		case KEY_HOME:
			if (focusinform == false)
				break;
			if(fieldctl(item, MOVE_CURSOR_BEGIN))
				DRAWITEM_TRICK(&form, item, true);
			break;
		case KEY_END:
			if (focusinform == false)
				break;
			if (fieldctl(item, MOVE_CURSOR_END))
				DRAWITEM_TRICK(&form, item, true);
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			curs_set(0);
			if (f1help_dialog(conf) != 0) {
				retval = BSDDIALOG_ERROR;
				loop = false;
			}
			if (form_redraw(&d, &form, nitems, items, curritem,
			    focusinform, item) != 0)
				return (BSDDIALOG_ERROR);
			break;
		case KEY_RESIZE:
			if (form_redraw(&d, &form, nitems, items, curritem,
			    focusinform, item) != 0)
				return (BSDDIALOG_ERROR);
			break;
		default:
			if (wchtype == KEY_CODE_YES)
				break;
			if (focusinform) {
				if (item->fieldonebyte && wctob(input) == EOF)
					break;
				/*
				 * MOVE_CURSOR_RIGHT manages new positions
				 * because the cursor remains on the new letter,
				 * "if" and "while" update the positions.
				 */
				if(insertch(&form, item, input)) {
					fieldctl(item, MOVE_CURSOR_RIGHT);
					/*
					 * no if(fieldctl), update always
					 * because it fails with maxletters.
					 */
					DRAWITEM_TRICK(&form, item, true);
				}
			} else {
				if (shortcut_buttons(input, &d.bs)) {
					retval = return_values(conf,
					    BUTTONVALUE(d.bs), nitems, apiitems,
					    items);
					loop = false;
				}
			}
			break;
		} /* end switch get_wch() */

		if (switchfocus) {
			focusinform = !focusinform;
			d.bs.curr = 0;
			redrawbuttons(&d,
			    conf->button.always_active || !focusinform,
			    !focusinform);
			DRAWITEM_TRICK(&form, item, focusinform);
			switchfocus = false;
		}

		if (changeitem) {
			DRAWITEM_TRICK(&form, item, false);
			curritem = next;
			item = &items[curritem];
			curriteminview(&form, item);
			update_formbox(conf, &form);
			DRAWITEM_TRICK(&form, item, true);
			changeitem = false;
		}
	} /* end while(loop) */

	curs_set(0);

	for (i = 0; i < nitems; i++) {
		free(items[i].privwbuf);
		free(items[i].pubwbuf);
	}
	delwin(form.pad);
	delwin(form.box);
	end_dialog(&d);

	return (retval);
}
