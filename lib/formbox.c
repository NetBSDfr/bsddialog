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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef PORTNCURSES
#include <ncurses/form.h>
#else
#include <form.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "bsddialog_theme.h"

/* "Form": inputbox - passwordbox - form - passwordform - mixedform */

extern struct bsddialog_theme t;

struct myfield {
	int len;
	char *buf;
	int pos;
	int size;
};

#define GETMYFIELD(field) ((struct myfield*)field_userptr(field))
#define GETMYFIELD2(form) ((struct myfield*)field_userptr(current_field(form)))

int bsddialog_inputmenu(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	text = "Inputbox unimplemented";
	bsddialog_msgbox(conf, text, rows, cols);
	RETURN_ERROR(text);
}

#define ISITEMHIDDEN(item) (item.flags & BSDDIALOG_ITEMHIDDEN)
#define ISITEMREADONLY(item) (item.flags & BSDDIALOG_ITEMREADONLY)

static void insertch(struct myfield *mf, int ch)
{
	int i;

	if (mf->len == mf->size)
		return;

	/* shift right */
	for (i=mf->len-1; i>=mf->pos; i--) {
		mf->buf[i+1] = mf->buf[i];
	}

	mf->buf[mf->pos] = ch;
	mf->pos += 1;
	mf->len += 1;
	mf->buf[mf->len] = '\0';
}

static void shiftleft(struct myfield *mf)
{
	int i, last;

	for (i=mf->pos; i<mf->len; i++) {
		mf->buf[i] = mf->buf[i+1];
	}

	last = mf->len > 0 ? mf->len -1 : 0;
	mf->buf[last] = '\0';
		mf->len = last;
}

static int
form_handler(WINDOW *widget, int y, int cols, struct buttons bs, WINDOW *entry,
    FORM *form, FIELD **field, int nitems, struct bsddialog_formitem *items)
{
	bool loop, buttupdate, inentry = true;
	int i, input, output;
	struct myfield *mf;

	curs_set(2);
	pos_form_cursor(form);
	loop = buttupdate = true;
	bs.curr = -1;
	form_driver(form, REQ_END_LINE);
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, y, cols, bs, !inentry);
			wrefresh(widget);
			buttupdate = false;
		}
		wrefresh(entry);
		input = getch();
		switch(input) {
		case KEY_ENTER:
		case 10: /* Enter */
			if (inentry)
				break;
			output = bs.value[bs.curr];
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
			for (i=0; i<nitems; i++) {
				items[i].newvalue1 = strdup(field_buffer(field[i], 0));
				mf = GETMYFIELD(field[i]);
				items[i].newvalue2 = mf->buf;
			}
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ESC;
			loop = false;
			break;
		case '\t': /* TAB */
			if (inentry) {
				bs.curr = 0;
				inentry = false;
				curs_set(0);
			} else {
				bs.curr++;
				inentry = bs.curr >= (int) bs.nbuttons ? true : false;
				if (inentry) {
					curs_set(2);
					pos_form_cursor(form);
				}
			}
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (inentry) {
				form_driver(form, REQ_PREV_CHAR);
				mf = GETMYFIELD2(form);
				if (mf->pos > 0)
					mf->pos -= 1;
				BSDDIALOG_DEBUG(1,1, "pos:%d, len:%d", mf->pos, mf->len);
			} else {
				if (bs.curr > 0) {
					bs.curr--;
					buttupdate = true;
				}
			}
			break;
		case KEY_RIGHT:
			if (inentry) {
				mf = GETMYFIELD2(form);
				if (mf->pos >= mf->len)
					break;
				mf->pos += 1;
				BSDDIALOG_DEBUG(1,1, "pos:%d, len:%d", mf->pos, mf->len);
				form_driver(form, REQ_NEXT_CHAR);
			} else {
				if (bs.curr < (int) bs.nbuttons - 1) {
					bs.curr++;
					buttupdate = true;
				}
			}
			break;
		case KEY_UP:
			if (nitems < 2)
				break;
			set_field_fore(current_field(form), t.form.fieldcolor);
			set_field_back(current_field(form), t.form.fieldcolor);
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), t.form.f_fieldcolor);
			set_field_back(current_field(form), t.form.f_fieldcolor);
			break;
		case KEY_DOWN:
			if (nitems < 2)
				break;
			set_field_fore(current_field(form), t.form.fieldcolor);
			set_field_back(current_field(form), t.form.fieldcolor);
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), t.form.f_fieldcolor);
			set_field_back(current_field(form), t.form.f_fieldcolor);
			break;
		case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
			mf = GETMYFIELD2(form);
			BSDDIALOG_DEBUG(3,1, "buf: |%s|", mf->buf);
			shiftleft(mf);
			BSDDIALOG_DEBUG(4, 1, "buf: |%s|", mf->buf);
			break;
		case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
			mf = GETMYFIELD2(form);
			BSDDIALOG_DEBUG(3,1, "buf: |%s|", mf->buf);
			shiftleft(mf);
			BSDDIALOG_DEBUG(4,1, "buf: |%s|", mf->buf);
			break;
		default:
			if (inentry) {
				
				form_driver(form, input);
				mf = GETMYFIELD2(form);
				BSDDIALOG_DEBUG(1,1, "pos:%d, len:%d", mf->pos, mf->len);
				BSDDIALOG_DEBUG(2,1, "buf: |%s|", mf->buf);
				insertch(mf, input);
				BSDDIALOG_DEBUG(3,1, "pos:%d, len:%d", mf->pos, mf->len);
				BSDDIALOG_DEBUG(4,1, "buf: |%s|", mf->buf);
			}
			else {
				for (i = 0; i < (int) bs.nbuttons; i++) {
					if (tolower(input) ==
					    tolower((bs.label[i])[0])) {
						output = bs.value[i];
						loop = false;
					}
				}
			}
			break;
		}
	}

	curs_set(0);

	return output;
}

int
bsddialog_form(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int nitems, struct bsddialog_formitem *items)
{
	WINDOW *widget, *entry, *shadow;
	int i, output, color, y, x;
	FIELD **field;
	FORM *form;
	struct buttons bs;

	if (new_widget(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	entry = new_boxed_window(conf, y + rows - 3 - formheight -2, x +1,
	    formheight+2, cols-2, LOWERED);

	get_buttons(conf, &bs, BUTTONLABEL(ok_label), BUTTONLABEL(extra_label),
	    BUTTONLABEL(cancel_label), BUTTONLABEL(help_label));

	struct myfield *myfields = malloc(nitems * sizeof(struct myfield));
	field = calloc(nitems + 1, sizeof(FIELD*));
	for (i=0; i < nitems; i++) {
		field[i] = new_field(1, items[i].formlen, items[i].yvalue-1, items[i].xvalue-1, 0, 0);
		field_opts_off(field[i], O_STATIC);
		set_max_field(field[i], items[i].valuelen);
		set_field_buffer(field[i], 0, items[i].init);

		myfields[i].pos = strlen(items[i].init);
		//if (myfields[i].pos < 0)
		//	myfields[i].pos = 0;
		myfields[i].len  = strlen(items[i].init);
		myfields[i].size  = items[i].valuelen;// + 1;
		myfields[i].buf = malloc(myfields[i].size);
		memset(myfields[i].buf, 0, myfields[i].size);
		strcpy(myfields[i].buf, items[i].init);
		set_field_userptr(field[i], &myfields[i]);

		field_opts_off(field[i], O_AUTOSKIP);
		field_opts_off(field[i], O_BLANK);
		//field_opts_off(field[i], O_BS_OVERLOAD);

		if (ISITEMHIDDEN(items[i]))
			field_opts_off(field[i], O_PUBLIC);

		if (ISITEMREADONLY(items[i])) {
			field_opts_off(field[i], O_EDIT);
			field_opts_off(field[i], O_ACTIVE);
			color = t.form.readonlycolor;
		} else {
			color = i == 0 ? t.form.f_fieldcolor : t.form.fieldcolor;
		}
		set_field_fore(field[i], color);
		set_field_back(field[i], color);
	}
	field[i] = NULL;

	 /* disable focus with 1 item (and inputbox or passwordbox) */
	if (nitems == 1) {
		set_field_fore(field[0], t.widgetcolor);
		set_field_back(field[0], t.widgetcolor);
	}

	form = new_form(field);
	set_form_win(form, entry);
	set_form_sub(form, derwin(entry, nitems, cols-4, 1, 1));
	post_form(form);

	for (i=0; i < nitems; i++)
		mvwaddstr(entry, items[i].ylabel, items[i].xlabel, items[i].label);

	wrefresh(entry);

	output = form_handler(widget, rows-2, cols, bs, entry, form, field,
	    nitems, items);

	unpost_form(form);
	free_form(form);
	for (i=0; i < nitems; i++)
		free_field(field[i]);
	free(field);

	delwin(entry);
	end_widget(conf, widget, rows, cols, shadow);

	return output;
}
