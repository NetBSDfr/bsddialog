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

struct privateitem {
	const char *label;      /* formitem.label */
	unsigned int ylabel;    /* formitem.ylabel */
	unsigned int xlabel;    /* formitem.xlabel */
	unsigned int yfield;    /* formitem.yfield */
	unsigned int xfield;    /* formitem.xfield */
	bool secure;            /* formitem.flags & BSDDIALOG_FIELDHIDDEN */
	bool readonly;          /* formitem.flags & BSDDIALOG_FIELDREADONLY */
	bool fieldnocolor;      /* formitem.flags & BSDDIALOG_FIELDNOCOLOR */
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

enum operation {
	MOVE_CURSOR_BEGIN,
	MOVE_CURSOR_RIGHT,
	MOVE_CURSOR_LEFT,
	DEL_LETTER
};

static bool fieldctl(struct privateitem *item, enum operation op)
{
	bool change;
	int width, oldwidth, nextwidth, cols;
	unsigned int i;

	/*BSDDIALOG_DEBUG(2,2,"|pos:%u, xposdraw:%u, xcursor:%u, "
	    "fieldcols:%u, nletters:%u, maxletters:%u|",
	    item->pos, item->xposdraw, item->xcursor, item->fieldcols, 
	    item->nletters, item->maxletters);*/
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
	/*BSDDIALOG_DEBUG(3,2,"|pos:%u, xposdraw:%u, xcursor:%u, "
	    "fieldcols:%u, nletters:%u, maxletters:%u|",
	    item->pos, item->xposdraw, item->xcursor, item->fieldcols, 
	    item->nletters, item->maxletters);*/

	return (change);
}

static void drawitem(WINDOW *w, struct privateitem *item, bool focus)
{
	int color;
	unsigned int n, cols;

	/* Label */
	mvwaddstr(w, item->ylabel, item->xlabel, item->label);

	/* Field */
	if (item->readonly)
		color = t.form.readonlycolor;
	else if (item->fieldnocolor)
		color = t.dialog.color;
	else
		color = focus ? t.form.f_fieldcolor : t.form.fieldcolor;
	wattron(w, color);
	/* can "fail", see trick in case KEY_DC */
	mvwhline(w, item->yfield, item->xfield, ' ', item->fieldcols);
	wrefresh(w); /* important for following multicolumn letters */
	n=0;
	cols = wcwidth(item->pubwbuf[item->xposdraw]);
	while (cols <= item->fieldcols && item->xposdraw + n <
	    wcslen(item->pubwbuf)) {
		n++;
		cols += wcwidth(item->pubwbuf[item->xposdraw + n]);
		
	}
	mvwaddnwstr(w, item->yfield, item->xfield,
	    &item->pubwbuf[item->xposdraw], n);
	wattroff(w, color);

	/* Bottom Desc */
	move(SCREENLINES - 1, 2);
	clrtoeol();
	if (item->bottomdesc != NULL && focus) {
		addstr(item->bottomdesc);
		refresh();
	}

	curs_set((focus && item->cursor) ? 1 : 0 );
	wmove(w, item->yfield, item->xfield + item->xcursor);
	wrefresh(w); /* to be sure after bottom desc addstr and refresh */
}

static bool insertch(struct privateitem *item, wchar_t wch, wchar_t securewch)
{
	int i;

	if (item->nletters >= item->maxletters)
		return (false);

	for (i = (int)item->nletters - 1; i >= (int)item->pos; i--) {
		item->privwbuf[i+1] = item->privwbuf[i];
		item->pubwbuf[i+1] = item->pubwbuf[i];
	}

	item->privwbuf[item->pos] = wch;
	item->pubwbuf[item->pos] = item->secure ? securewch : wch;
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
	bool switchfocus, changeitem, focusinform, insecurecursor, loop;
	int curritem, mbchsize, next, output, y, x, h, w, wchtype;
	unsigned int hformpad, wformpad;
	unsigned int i, j;
	wchar_t securewch, *winit;
	wint_t input;
	WINDOW *widget, *formpad, *formwin, *textpad, *shadow;
	struct privateitem *items, *item;
	struct buttons bs;

	/* disable form scrolling */
	if (formheight < nitems)
		formheight = nitems;

	for (i = 0; i < nitems; i++) {
		if (apiitems[i].maxvaluelen == 0)
			RETURN_ERROR("maxvaluelen cannot be zero");
		if (apiitems[i].fieldlen == 0)
			RETURN_ERROR("fieldlen cannot be zero");
	}

	insecurecursor = false;
	if (conf->form.securembch != NULL) {
		mbchsize = mblen(conf->form.securembch, MB_LEN_MAX);
		if(mbtowc(&securewch, conf->form.securembch, mbchsize) < 0)
			RETURN_ERROR("Cannot convert securembch to wchar_t");
		insecurecursor = true;
	} else if (conf->form.securech != '\0') {
		securewch = btowc(conf->form.securech);
		insecurecursor = true;
	} else {
		securewch = L' '; 
	}

	if ((items = malloc(nitems * sizeof(struct privateitem))) == NULL)
		RETURN_ERROR("Cannot allocate internal items");
	hformpad = wformpad = 0;
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
		item->xposdraw = 0;
		item->xcursor = 0;
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
				item->pubwbuf[j] = securewch;
		}

		item->pos = 0;
		if (apiitems[i].flags & BSDDIALOG_FIELDCURSOREND)
			while (fieldctl(item, MOVE_CURSOR_RIGHT))
				; /* shit to right */

		item->fieldcols = apiitems[i].fieldlen;

		hformpad = MAX(hformpad, items[i].ylabel);
		hformpad = MAX(hformpad, items[i].yfield);
		wformpad = MAX(wformpad, items[i].xlabel + strcols(items[i].label));
		wformpad = MAX(wformpad, items[i].xfield + items[i].fieldcols - 1);
	}

	get_buttons(conf, &bs, BUTTON_OK_LABEL, BUTTON_CANCEL_LABEL);

	if (set_widget_size(conf, rows, cols, &h, &w) != 0)
		return (BSDDIALOG_ERROR);
	if (form_autosize(conf, rows, cols, &h, &w, text, wformpad, &formheight,
	    nitems, bs) != 0)
		return (BSDDIALOG_ERROR);
	if (form_checksize(h, w, text, formheight, nitems, wformpad, bs) != 0)
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

	curritem = -1;
	for (i=0 ; i < nitems; i++) {
		drawitem(formwin, &items[i], false);
		if (curritem == -1 && item->readonly == false)
			curritem = i;
	}
	if (curritem != -1) {
		item = &items[curritem];
		drawitem(formwin, item, true);
		focusinform = true;
		redrawbuttons(widget, &bs, conf->form.focus_buttons, false);
	} else {
		item = NULL;
		focusinform = false;
	}

	changeitem = switchfocus = false;
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
				drawitem(formwin, item, false);
			} else {
				if (bs.curr + 1 < (int)bs.nbuttons) {
					bs.curr++;
				} else {
					bs.curr = 0;
					if (curritem != -1) {
						focusinform = true;
						drawitem(formwin, item, true);
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
			} else if (bs.curr > 0) {
				bs.curr--;
				draw_buttons(widget, bs, true);
				wrefresh(widget);
			} else if (curritem != -1) {
				switchfocus = true;
			}
			break;
		case KEY_RIGHT:
			if (focusinform) {
				if(fieldctl(item, MOVE_CURSOR_RIGHT))
					drawitem(formwin, item, true);
			} else if (bs.curr < (int) bs.nbuttons - 1) {
				bs.curr++;
				draw_buttons(widget, bs, true);
				wrefresh(widget);
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
			//curs_set(1); drawitem in KEY_RESIZE later
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

		if (switchfocus) {
			focusinform = !focusinform;
			drawitem(formwin, item, focusinform);
			bs.curr = 0;
			redrawbuttons(widget, &bs, conf->form.focus_buttons ||
			    !focusinform, !focusinform);
			switchfocus = false;
		}

		if (changeitem) {
			drawitem(formwin, item, false);
			curritem = next;
			item = &items[curritem];
			drawitem(formwin, item, true);
			//if (ymenupad > abs && ymenupad > 0)
			//	ymenupad = abs;
			//if ((int)(ymenupad + menurows) <= abs)
			//	ymenupad = abs - menurows + 1;
			//update_menuwin(conf, menuwin, menurows+2, w-4,
			//    totnitems, menurows, ymenupad);
			//wrefresh(menuwin);
			//prefresh(menupad, ymenupad, 0, ys, xs, ye, xe);
			changeitem = false;
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
