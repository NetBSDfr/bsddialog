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

#include <stdlib.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#include <ncurses/form.h>
#else
#include <curses.h>
#include <form.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "bsddialog_theme.h"

/* "Form": inputbox - passwordbox - form - passwordform - mixedform */

extern struct bsddialog_theme t;

int bsddialog_inputmenu(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	text = "Inputbox unimplemented";
	bsddialog_msgbox(conf, text, rows, cols);
	RETURN_ERROR(text);
}

#define ISITEMHIDDEN(item) (item.flags & BSDDIALOG_ITEMHIDDEN)
#define ISITEMREADONLY(item) (item.flags & BSDDIALOG_ITEMREADONLY)

static int
mixedform_handler(WINDOW *widget, int y, int cols, struct buttons bs,
    bool shortkey, WINDOW *entry, FORM *form, FIELD **field, int nitems
    /*struct formitem *items*/)
{
	bool loop, buttupdate, inentry = true;
	int input, output;

	curs_set(2);
	pos_form_cursor(form);
	loop = buttupdate = true;
	bs.curr = -1;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, y, cols, bs, shortkey);
			wrefresh(widget);
			buttupdate = false;
		}
		wrefresh(entry);
		input = getch();
		switch(input) {
		case 10: // Enter
			if (inentry)
				break;
			output = bs.value[bs.curr]; // values -> buttvalues
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
			/* add a struct for forms */
			/*for (i=0; i<nitems; i++) {
				bufp = field_buffer(field[i], 0);
				dprintf(fd, "\n+%s", bufp);
				bufp = field_buffer(field[i], 1);
				dprintf(fd, "-%s+", bufp);
			}*/
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ESC;
			loop = false;
			break;
		case '\t': // TAB
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
			} else {
				if (bs.curr > 0) {
					bs.curr--;
					buttupdate = true;
				}
			}
			break;
		case KEY_RIGHT:
			if (inentry) {
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
			set_field_fore(current_field(form), t.fieldcolor);
			set_field_back(current_field(form), t.fieldcolor);
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), t.currfieldcolor);
			set_field_back(current_field(form), t.currfieldcolor);
			break;
		case KEY_DOWN:
			if (nitems < 2)
				break;
			set_field_fore(current_field(form), t.fieldcolor);
			set_field_back(current_field(form), t.fieldcolor);
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_END_LINE);
			set_field_fore(current_field(form), t.currfieldcolor);
			set_field_back(current_field(form), t.currfieldcolor);
			break;
		case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
			break;
		case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
			break;
		default:
			if (inentry) {
				form_driver(form, input);
			}
			break;
		}
	}

	curs_set(0);

	return output;
}

static int
do_mixedform(struct bsddialog_conf conf, char* text, int rows, int cols,
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

	field = calloc(nitems + 1, sizeof(FIELD*));
	for (i=0; i < nitems; i++) {
		field[i] = new_field(1, items[i].formlen, items[i].yvalue-1, items[i].xvalue-1, 0, 1);
		field_opts_off(field[i], O_STATIC);
		set_max_field(field[i], items[i].valuelen);
		set_field_buffer(field[i], 0, items[i].value);
		set_field_buffer(field[i], 1, items[i].value);
		field_opts_off(field[i], O_AUTOSKIP);
		field_opts_off(field[i], O_BLANK);
		//field_opts_off(field[i], O_BS_OVERLOAD);

		if (ISITEMHIDDEN(items[i]))
			field_opts_off(field[i], O_PUBLIC);

		if (ISITEMREADONLY(items[i])) {
			field_opts_off(field[i], O_EDIT);
			field_opts_off(field[i], O_ACTIVE);
			color = t.fieldreadonlycolor;
		} else {
			color = i == 0 ? t.currfieldcolor : t.fieldcolor;
		}
		set_field_fore(field[i], color);
		set_field_back(field[i], color);
	}
	field[i] = NULL;

	if (nitems == 1) {// inputbox or passwordbox
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

	output = mixedform_handler(widget, rows-2, cols, bs, true, entry, form,
	    field, nitems /*,items*/);

	unpost_form(form);
	free_form(form);
	for (i=0; i < nitems; i++)
		free_field(field[i]);
	free(field);

	delwin(entry);
	end_widget(conf, widget, rows, cols, shadow);

	return output;
}

int
bsddialog_inputbox(struct bsddialog_conf conf, char* text, int rows, int cols,
    struct bsddialog_formitem *item)
{
	int output;

	output = do_mixedform(conf, text, rows, cols, 1, 1, item);

	return output;
}

int
bsddialog_passwordbox(struct bsddialog_conf conf, char* text, int rows, 
    int cols, struct bsddialog_formitem *item)
{
	int output;

	output = do_mixedform(conf, text, rows, cols, 1, 1, item);

	return output;
}

int
bsddialog_mixedform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int nitems, struct bsddialog_formitem *items)
{
	int output;

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

int
bsddialog_form(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int nitems, struct bsddialog_formitem *items)
{
	int output;

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

int
bsddialog_passwordform(struct bsddialog_conf conf, char* text, int rows,
    int cols, int formheight, int nitems, struct bsddialog_formitem *items)
{
	int output;

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

