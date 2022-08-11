/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021-2022 Alfonso Sabato Siciliano
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

#include <curses.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "bsddialog.h"
#include "bsddialog_theme.h"
#include "lib_util.h"

struct privitem {
	const char *label;      /* formitem.label */
	unsigned int ylabel;    /* formitem.ylabel */
	unsigned int xlabel;    /* formitem.xlabel */
	unsigned int yfield;    /* formitem.yfield */
	unsigned int xfield;    /* formitem.xfield */
	bool secure;            /* formitem.flags & BSDDIALOG_FIELDHIDDEN */
	bool readonly;          /* formitem.flags & BSDDIALOG_FIELDREADONLY */
	bool fieldnocolor;      /* formitem.flags & BSDDIALOG_FIELDNOCOLOR */
	const char *bottomdesc; /* formitem.bottomdesc */
	
	wchar_t *privwbuf;       /* formitem.value */
	wchar_t *pubwbuf;        /* string for drawitem() */
	unsigned int maxletters; /* formitem.maxvaluelen, [priv|pub]wbuf size */
	unsigned int nletters;   /* letters in privwbuf and pubwbuf */
	unsigned int pos;        /* pos in privwbuf and pubwbuf */
	unsigned int fieldcols;  /* formitem.fieldlen */
	unsigned int xcursor;    /* position in fieldcols [0 - fieldcols-1] */
	unsigned int xletterpubbuf; /* first position to draw in pubwbuf */
};

enum operation {
	MOVE_CURSOR_BEGIN,
	MOVE_CURSOR_RIGHT,
	MOVE_CURSOR_LEFT,
	DEL_LETTER
};

static bool fieldctl(struct privitem *item, enum operation op)
{
	bool change;
	int width, oldwidth, nextwidth, cols;
	unsigned int i;

//BSDDIALOG_DEBUG(2,2,"pos:%u, xletterpubbuf:%u, xcursor:%u, fieldcols:%u, nletters:%u, maxletters:%u|||||",
//    item->pos, item->xletterpubbuf, item->xcursor, item->fieldcols, item->nletters, item->maxletters);
	change = false;
	switch (op){
	case MOVE_CURSOR_BEGIN:
		if (item->pos == 0 && item->xcursor == 0)
			break;
		/* here the cursor is changed */
		change = true;
		item->pos = 0;
		item->xcursor = 0;
		item->xletterpubbuf = 0;
		break;
	case MOVE_CURSOR_LEFT:
		if (item->pos == 0)
			break;
		if (item->xcursor == 0 && item->xletterpubbuf == 0)
			break; /* useless by item->pos == 0 and 'while' below */
		/* here some letter to left */
		change = true;
		item->pos -= 1;
		width = wcwidth(item->pubwbuf[item->pos]);
		if (((int)item->xcursor) - width < 0) {
			item->xcursor = 0;
			item->xletterpubbuf -= 1;
		} else
			item->xcursor -= width;

		while (true){
			if (item->xletterpubbuf == 0)
				break;
			if (item->xcursor >= item->fieldcols / 2)
				break;
			if (wcwidth(item->pubwbuf[item->xletterpubbuf - 1]) +
			    item->xcursor + width > item->fieldcols)
				break;

			item->xletterpubbuf -= 1;
			item->xcursor +=
			    wcwidth(item->pubwbuf[item->xletterpubbuf]);
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
			item->xletterpubbuf = item->pos;
			while (item->xletterpubbuf != 0) {
				cols += wcwidth(item->pubwbuf[item->xletterpubbuf - 1]);
				if (cols > (int)item->fieldcols)
					break;
				item->xletterpubbuf -= 1;
			}
			item->xcursor = 0;
			for (i = item->xletterpubbuf; i < item->pos ; i++)
				item->xcursor += wcwidth(item->pubwbuf[i]);
		}
		else {
			item->xcursor += oldwidth;
		}

		break;
	}
//BSDDIALOG_DEBUG(3,2,"pos:%u, xletterpubbuf:%u, xcursor:%u, fieldcols:%u, nletters:%u, maxletters:%u|||||",
//    item->pos, item->xletterpubbuf, item->xcursor, item->fieldcols, item->nletters, item->maxletters);

	return (change);
}

static void drawitem(WINDOW *w, struct privitem *ni, bool focus)
{
	int color;
	unsigned int i, cols;

	/* Label */
	mvwaddstr(w, ni->ylabel, ni->xlabel, ni->label);

	/* Field */
	if (ni->readonly)
		color = t.form.readonlycolor;
	else if (ni->fieldnocolor)
		color = t.dialog.color;
	else
		color = focus ? t.form.f_fieldcolor : t.form.fieldcolor;
	wattron(w, color);
	wmove(w, ni->yfield, ni->xfield);
	for (i = 0; i < ni->fieldcols; i++)
		waddch(w, ' '); /* can "fail", see trick in case KEY_DC */
	wrefresh(w); /* important for following multicolumn letters */
	i=0;
	cols = wcwidth(ni->pubwbuf[ni->xletterpubbuf]);
	while (cols <= ni->fieldcols && ni->xletterpubbuf + i <
	    wcslen(ni->pubwbuf)) {
		mvwaddwch(w, ni->yfield, ni->xfield + i,
		    ni->pubwbuf[ni->xletterpubbuf + i]);
		i++;
		cols += wcwidth(ni->pubwbuf[ni->xletterpubbuf + i]);
		
	}
	wattroff(w, color);

	/* Bottom Desc */
	move(SCREENLINES - 1, 2);
	clrtoeol();
	if (ni->bottomdesc != NULL && focus) {
		addstr(ni->bottomdesc);
		refresh();
	}

	if (focus)
		wmove(w, ni->yfield, ni->xfield + ni->xcursor);
	wrefresh(w); /* to be sure after bottom desc addstr and refresh */
}

static bool insertch(struct privitem *mf, wchar_t wch, wchar_t securewch)
{
	int i;

	if (mf->nletters >= mf->maxletters)
		return (false);

	for (i = (int)mf->nletters - 1; i >= (int)mf->pos; i--) {
		mf->privwbuf[i+1] = mf->privwbuf[i];
		mf->pubwbuf[i+1] = mf->pubwbuf[i];
	}

	mf->privwbuf[mf->pos] = wch;
	mf->pubwbuf[mf->pos] = mf->secure ? securewch : wch;
	mf->nletters += 1;
	mf->privwbuf[mf->nletters] = L'\0';
	mf->pubwbuf[mf->nletters] = L'\0';

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
    struct bsddialog_formitem *items, struct privitem *pritems)
{
	int i;

	if (output != BSDDIALOG_OK && conf->form.value_without_ok == false)
		return (output);

	for (i = 0; i < nitems; i++) {
		if (conf->form.value_wchar) {
			items[i].value = (char*)wcsdup(pritems[i].privwbuf);
		} else {
			items[i].value = alloc_wstomb(pritems[i].privwbuf);
		}
		if (items[i].value == NULL)
			RETURN_ERROR("Cannot allocate memory for form value");
	}

	return (output);
}

static unsigned int firstitem(unsigned int nitems, struct privitem *items)
{
	int i;

	for (i = 0; i < (int)nitems; i++)
		if (items[i].readonly == false)
			break;

	return (i);
}

static unsigned int lastitem(unsigned int nitems, struct privitem *items)
{
	int i;

	for (i = nitems - 1; i >= 0 ; i--)
		if (items[i].readonly == false)
			break;

	return (i);
}

static unsigned int
previtem(unsigned int nitems, struct privitem *items, int curritem)
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
nextitem(unsigned int nitems, struct privitem *items, int curritem)
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

static void
redrawbuttons(WINDOW *window, struct buttons *bs, bool focus, bool shortcut)
{
	int selected;

	selected = bs->curr;
	if (focus == false)
		bs->curr = -1;
	draw_buttons(window, *bs, shortcut);
	wrefresh(window);
	bs->curr = selected;
}

static int
form_autosize(struct bsddialog_conf *conf, int rows, int cols, int *h, int *w,
    const char *text, int linelen, unsigned int *formheight, int nitems,
    struct buttons bs)
{
	int htext, wtext, menusize;

	if (cols == BSDDIALOG_AUTOSIZE || rows == BSDDIALOG_AUTOSIZE) {
		if (text_size(conf, rows, cols, text, &bs, *formheight + 2,
		    linelen + 2, &htext, &wtext) != 0)
			return (BSDDIALOG_ERROR);
	}

	if (cols == BSDDIALOG_AUTOSIZE)
		*w = widget_min_width(conf, wtext, linelen + 2, &bs);

	if (rows == BSDDIALOG_AUTOSIZE) {
		if (*formheight == 0) {
			menusize = widget_max_height(conf) - HBORDERS -
			     2 /*buttons*/ - htext;
			menusize = MIN(menusize, nitems + 2);
			*formheight = menusize - 2 < 0 ? 0 : menusize - 2;
		}
		else /* h autosize with fixed formheight */
			menusize = *formheight + 2;

		*h = widget_min_height(conf, htext, menusize, true);
	} else {
		if (*formheight == 0)
			*formheight = MIN(rows-6-htext, nitems);
	}

	return (0);
}

static int
form_checksize(int rows, int cols, const char *text, int formheight, int nitems,
    unsigned int linelen, struct buttons bs)
{
	int mincols, textrow, formrows;

	mincols = VBORDERS;
	/* buttons */
	mincols += buttons_width(bs);
	mincols = MAX(mincols, (int)linelen + 4);

	if (cols < mincols)
		RETURN_ERROR("Few cols, width < size buttons or "
		    "forms (label + field)");

	// XXX last strlen
	textrow = text != NULL && strlen(text) > 0 ? 1 : 0;

	if (nitems > 0 && formheight == 0)
		RETURN_ERROR("fields > 0 but formheight == 0, probably "
		    "terminal too small");

	formrows = nitems > 0 ? 3 : 0;
	if (rows < 2  + 2 + formrows + textrow)
		RETURN_ERROR("Few lines for this menus");

	return (0);
}

/* API */
int
bsddialog_form(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, unsigned int formheight, unsigned int nitems,
    struct bsddialog_formitem *apiitems)
{
	bool loop, focusinform;
	wchar_t securewch;
	int output, y, x, h, w, wchtype, curritem, mbchsize;
	unsigned int i, j;
	unsigned long maxline;
	wint_t input;
	struct buttons bs;
	struct privitem *items, *item;
	wchar_t *winit;
	WINDOW *widget, *formwin, *textpad, *shadow;

	/* disable form scrolling */
	if (formheight < nitems)
		formheight = nitems;

	for (i = 0; i < nitems; i++) {
		if (apiitems[i].maxvaluelen == 0)
			RETURN_ERROR("maxvaluelen cannot be zero");
		if (apiitems[i].fieldlen == 0)
			RETURN_ERROR("fieldlen cannot be zero");
	}

	if (conf->form.securembch != NULL) {
		mbchsize = mblen(conf->form.securembch, MB_LEN_MAX);
		if(mbtowc(&securewch, conf->form.securembch, mbchsize) < 0)
			RETURN_ERROR("Cannot convert securembch to wchar_t");
	} else if (conf->form.securech != '\0') {
		securewch = btowc(conf->form.securech);
	} else {
		securewch = L' '; 
	}

	if ((items = malloc(nitems * sizeof(struct privitem))) == NULL)
		RETURN_ERROR("Cannot allocate internal items");
	maxline = 0;
	curritem = -1;
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
		item->bottomdesc = apiitems[i].bottomdesc;
		item->xletterpubbuf = 0;
		item->xcursor = 0;

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
				item->pubwbuf[j] = securewch;
		}

		item->pos = 0;
		if (apiitems[i].flags & BSDDIALOG_FIELDCURSOREND)
			while (fieldctl(item, MOVE_CURSOR_RIGHT))
				; /* shit to right */

		item->fieldcols = apiitems[i].fieldlen;

		if (curritem == -1 && item->readonly == false)
			curritem = i;

		maxline = MAX(maxline, items[i].xlabel + strcols(items[i].label));
		maxline = MAX(maxline, items[i].xfield + items[i].fieldcols - 1);
	}

	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (form_autosize(conf, rows, cols, &h, &w, text, maxline, &formheight,
	    nitems, bs) != 0)
		return (BSDDIALOG_ERROR);
	if (form_checksize(h, w, text, formheight, nitems, maxline, bs) != 0)
		return (BSDDIALOG_ERROR);
	if (set_widget_position(conf, &y, &x, h, w) != 0)
		return (BSDDIALOG_ERROR);

	if (new_dialog(conf, &shadow, &widget, y, x, h, w, &textpad, text, &bs,
	    true) != 0)
		return (BSDDIALOG_ERROR);

	prefresh(textpad, 0, 0, y + 1, x + 1 + TEXTHMARGIN, y + h - formheight,
	    x + 1 + w - TEXTHMARGIN);

	formwin = new_boxed_window(conf, y + h - 3 - formheight - 2, x + 1,
	    formheight + 2, w - 2, LOWERED);
	
	for (i=0 ; i < nitems; i++)
		drawitem(formwin, &items[i], false);
	item = NULL;
	if (curritem != -1) {
		curs_set(1);
		item = &items[curritem];
		drawitem(formwin, item, true);
		focusinform = true;
		redrawbuttons(widget, &bs, conf->form.focus_buttons, false);
	} else {
		focusinform = false;
	}

	loop = true;
	while (loop) {
		wrefresh(formwin);
		wchtype = get_wch(&input);
		/* avoid wctob(input) == WEOF for IEEE Std 1003.1-2008 */
		if (conf->form.input_singlebyte && wchtype != KEY_CODE_YES &&
		    wctob(input) == EOF) 
			continue;
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			if (focusinform && conf->form.focus_buttons == false)
				break;
			output = return_values(conf, bs.value[bs.curr],
			    nitems, apiitems, items);
			loop = false;
			break;
		case 27: /* Esc */
			if (conf->key.enable_esc) {
				output = return_values(conf, BSDDIALOG_ESC,
				    nitems, apiitems, items);
				loop = false;
			}
			break;
		case '\t': /* TAB */
			if (focusinform) {
				bs.curr = 0;
				focusinform = false;
				curs_set(0);
			} else {
				if (bs.curr + 1 < (int)bs.nbuttons) {
					bs.curr++;
				} else {
					bs.curr = 0;
					if (curritem != -1) {
						focusinform = true;
						curs_set(1);
					}
				}
			}
			redrawbuttons(widget, &bs, conf->form.focus_buttons ||
			    !focusinform, !focusinform);
			break;
		case KEY_LEFT:
			if (focusinform) {
				if(fieldctl(item, MOVE_CURSOR_LEFT))
					drawitem(formwin, item, true);
			} else {
				if (bs.curr > 0) {
					bs.curr--;
					draw_buttons(widget, bs, true);
					wrefresh(widget);
				}//else items == 1 && curritem != -1 focusinform
			}
			break;
		case KEY_RIGHT:
			if (focusinform) {
				if(fieldctl(item, MOVE_CURSOR_RIGHT))
					drawitem(formwin, item, true);
			} else {
				if (bs.curr < (int) bs.nbuttons - 1) {
					bs.curr++;
					draw_buttons(widget, bs, true);
					wrefresh(widget);
				}//else items == 1 && curritem != -1 focusinform
			}
			break;
		case KEY_UP:
			if (focusinform == false)
				break;
			drawitem(formwin, item, false);
			curritem = previtem(nitems, items, curritem);
			item = &items[curritem];
			drawitem(formwin, item, true);
			break;
		case KEY_DOWN:
			if (focusinform == false)
				break;
			drawitem(formwin, item, false);
			curritem = nextitem(nitems, items, curritem);
			item = &items[curritem];
			drawitem(formwin, item, true);
			break;
		case KEY_PPAGE:
			if (focusinform == false)
				break;
			drawitem(formwin, item, false);
			curritem = firstitem(nitems, items);
			item = &items[curritem];
			drawitem(formwin, item, true);
			break;
		case KEY_NPAGE:
			if (focusinform == false)
				break;
			drawitem(formwin, item, false);
			curritem = lastitem(nitems, items);
			item = &items[curritem];
			drawitem(formwin, item, true);
			break;
		case KEY_BACKSPACE:
		case 127: /* Backspace */
			if (focusinform == false)
				break;
			if(fieldctl(item, MOVE_CURSOR_LEFT))
				if(fieldctl(item, DEL_LETTER)) {
					/* trick, see case KEY_DC */
					drawitem(formwin, item, false);
					drawitem(formwin, item, true);
				}
			break;
		case KEY_DC:
			if (focusinform == false)
				break;
			if(fieldctl(item, DEL_LETTER)) {
				/*
				 * trick to force curses to redraw the empty
				 * single column in the field otherwise the
				 * deleted multicolumn letters are drawn again.
				 * libformw has a similar problem and solution.
				 */
				drawitem(formwin, item, false);
				drawitem(formwin, item, true);
			}
			break;
		case KEY_HOME:
			if (focusinform == false)
				break;
			if(fieldctl(item, MOVE_CURSOR_BEGIN))
				drawitem(formwin, item, true);
			break;
		case KEY_END:
			if (focusinform == false)
				break;
			while (fieldctl(item, MOVE_CURSOR_RIGHT))
				; /* shit to right */
			drawitem(formwin, item, true);
			break;
		case KEY_F(1):
			if (conf->key.f1_file == NULL &&
			    conf->key.f1_message == NULL)
				break;
			curs_set(0);
			if (f1help(conf) != 0) {
				output = BSDDIALOG_ERROR;
				loop = false;
			}
			curs_set(1);
			/* No Break */
		case KEY_RESIZE:
			if (update_dialog(conf, shadow, widget, y, x, h, w,
			    textpad, text, &bs, true) != 0)
			return (BSDDIALOG_ERROR);

			doupdate();
			wrefresh(widget);

			prefresh(textpad, 0, 0, y + 1, x + 1 + TEXTHMARGIN,
			    y + h - formheight, x + 1 + w - TEXTHMARGIN);

			draw_borders(conf, formwin, formheight+2, w-2, LOWERED);
			if (curritem != -1)
				drawitem(formwin, item, true);
			wrefresh(formwin);

			refresh();
			break;
		default:
			if (wchtype == KEY_CODE_YES)
				break;
			if (focusinform) {
				/*
				 * MOVE_CURSOR_RIGHT manages new positions
				 * because the cursor remains on the new letter,
				 * "if" and "while" update the positions.
				 */
				if(insertch(item, input, securewch)) {
					fieldctl(item, MOVE_CURSOR_RIGHT);
					/* 
					 * no if(fieldctl), update always
					 * because it fails with maxletters.
					 */ 
					drawitem(formwin, item, true);
				}
			}
			else {
				if (shortcut_buttons(input, &bs)) {
					output = return_values(conf,
					    bs.value[bs.curr], nitems, apiitems,
					    items);
					loop = false;
				}
			}
			break;
		}
	}
	curs_set(0);

	delwin(formwin);
	for (i = 0; i < nitems; i++) {
		free(items[i].privwbuf);
		free(items[i].pubwbuf);
	}
	end_dialog(conf, shadow, widget, textpad);

	return (output);
}
