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
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#include <ncurses/form.h>
#else
#include <curses.h>
#include <form.h>
#endif

#include "bsddialog.h"
#include "lib_util.h"
#include "theme.h"

#define MAXINPUT 2048

extern struct bsddialog_theme t;

int bsddialog_init(void)
{
	int i, j, c = 1, error = OK;

	set_error_string("");

	if(initscr() == NULL)
		RETURN_ERROR("Cannot init ncurses (initscr)");

	error += keypad(stdscr, TRUE);
	nl();
	error += cbreak();
	error += noecho();
	curs_set(0);
	if(error != OK) {
		bsddialog_end();
		RETURN_ERROR("Cannot init ncurses (keypad and cursor)");
	}

	error += start_color();
	for (i=0; i<8; i++)
		for(j=0; j<8; j++) {
			error += init_pair(c, i, j);
			c++;
	}
	if(error != OK) {
		bsddialog_end();
		RETURN_ERROR("Cannot init ncurses (colors)");
	}

	bsddialog_settheme(BSDDIALOG_THEME_DIALOG);

	return error;
}

int bsddialog_end(void)
{

	if (endwin() != OK)
		RETURN_ERROR("Cannot end ncurses (endwin)");

	return 0;
}

int bsddialog_backtitle(struct bsddialog_conf conf, char *backtitle)
{

	mvaddstr(0, 1, backtitle);
	if (conf.no_lines != true)
		mvhline(1, 1, conf.ascii_lines ? '-' : ACS_HLINE, COLS-2);

	refresh();

	return 0;
}

const char *bsddialog_geterror(void)
{

	return get_error_string();
}

/*
 * SECTIONS
 *  1) "Info"    infobox
 *  2) "Message" msgbox - yesno
 *  3) "Menu"    buildlist - checklist - menu - mixedlist - radiolist - treeview
 *  4) "Form"    inputbox - passwordbox - form - passwordform - mixedform
 *  5) "Editor"  editbox(todo)
 *  6) "Bar"     gauge - mixedgauge - rangebox - pause
 *  7) "Time"    timebox - calendar
 *  8) "Command" prgbox - programbox(todo) - progressbox(todo)
 *  9) "Text"    tailbox - tailboxbg(todo) - textbox
 * 10) "File"    dselect(todo) - fselect(todo)
 */

/*
 * SECTION 1 "Info": infobox
 */
int
bsddialog_infobox(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	WINDOW *widget, *shadow;
	int y, x;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    false) <0)
		return -1;

	getch();

	widget_end(conf, "Infobox", widget, rows, cols, shadow);

	return (BSDDIALOG_YESOK);
}

/*
 * SECTION 2 "Message": msgbox - yesno
 * message.c
 */

/*
 * SECTION 3 "Menu": checklist - menu - radiolist - treeview - buildlist
 * menus.c
 */

/*
 * SECTIONS 4 "Form": inputbox - passwordbox - form - passwordform - mixedform
 */
int bsddialog_inputmenu(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	return 0;
}

#define ITEMHIDDEN 0x1
#define ISITEMHIDDEN(item) (item.itemflags & 0x1)
#define ITEMREADONLY 0x2
#define ISITEMREADONLY(item) (item.itemflags & 0x2)
struct formitem {
	char *label;
	unsigned int ylabel;
	unsigned int xlabel;
	char *item;
	unsigned int yitem;
	unsigned int xitem;
	int itemlen;
	unsigned int inputlen;
	unsigned int itemflags;
};

static int
mixedform_handler(WINDOW *widget, int y, int cols, struct buttons bs,
    bool shortkey, WINDOW *entry, FORM *form, FIELD **field, int nitems,
    struct formitem *items, int fd)
{
	bool loop, buttupdate, inentry = true;
	int i, input, output;
	char *bufp;

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
			for (i=0; i<nitems; i++) {
				bufp = field_buffer(field[i], 0);
				dprintf(fd, "\n+%s", bufp);
				bufp = field_buffer(field[i], 1);
				dprintf(fd, "-%s+", bufp);
			}
			loop = false;
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			if (inentry) {
				bs.curr = 0;
				inentry = false;
				curs_set(0);
			} else {
				bs.curr++;
				inentry = bs.curr >= bs.nbuttons ? true : false;
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
				if (bs.curr < bs.nbuttons - 1) {
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
    int formheight, int nitems, struct formitem *items)
{
	WINDOW *widget, *entry, *shadow;
	int i, output, color, y, x;
	FIELD **field;
	FORM *form;
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	entry = new_window(y + rows - 3 - formheight -2, x +1,
	    formheight+2, cols-2, NULL, NULL, conf.no_lines ? NOLINES : LOWERED,
	    conf.ascii_lines);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	field = calloc(nitems + 1, sizeof(FIELD*));
	for (i=0; i < nitems; i++) {
		field[i] = new_field(1, items[i].itemlen, items[i].yitem-1, items[i].xitem-1, 0, 1);
		field_opts_off(field[i], O_STATIC);
		set_max_field(field[i], items[i].inputlen);
		set_field_buffer(field[i], 0, items[i].item);
		set_field_buffer(field[i], 1, items[i].item);
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
	    field, nitems, items, conf.output_fd);

	unpost_form(form);
	free_form(form);
	for (i=0; i < nitems; i++)
		free_field(field[i]);
	free(field);

	delwin(entry);
	widget_end(conf, "Mixedform", widget, rows, cols, shadow);

	return output;
}

int bsddialog_inputbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	int output;
	struct formitem item;

	item.label	= "";
	item.ylabel	= 0;
	item.xlabel	= 0;
	item.item	= ""; // TODO add argv
	item.yitem	= 1;
	item.xitem	= 1;
	item.itemlen	= cols-4;
	item.inputlen	= 2048; // todo conf.sizeinput
	item.itemflags	= 0;

	output = do_mixedform(conf, text, rows, cols, 1, 1, &item);

	return output;
}

int bsddialog_passwordbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	int output;
	struct formitem item;

	item.label	= "";
	item.ylabel	= 0;
	item.xlabel	= 0;
	item.item	= ""; // TODO add argv
	item.yitem	= 1;
	item.xitem	= 1;
	item.itemlen	= cols-4;
	item.inputlen	= 2048; // todo conf.sizeinput
	item.itemflags	= ITEMHIDDEN;

	output = do_mixedform(conf, text, rows, cols, 1, 1, &item);

	return output;
}

int
bsddialog_mixedform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv)
{
	int i, output, nitems;
	struct formitem items[128];

	if ((argc % 9) != 0)
		return (-1);

	nitems = argc / 9;
	for (i=0; i<nitems; i++) {
		items[i].label	   = argv[9*i];
		items[i].ylabel	   = atoi(argv[9*i+1]);
		items[i].xlabel	   = atoi(argv[9*i+2]);
		items[i].item	   = argv[9*i+3];
		items[i].yitem	   = atoi(argv[9*i+4]);
		items[i].xitem	   = atoi(argv[9*i+5]);
		items[i].itemlen   = atoi(argv[9*i+6]);
		items[i].inputlen  = atoi(argv[9*i+7]);
		items[i].itemflags = atoi(argv[9*i+8]);
	}

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

int
bsddialog_form(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv)
{
	int i, output, nitems, itemlen, inputlen;
	unsigned int flags = 0;
	struct formitem items[128];

	if ((argc % 8) != 0)
		return (-1);

	nitems = argc / 8;
	for (i=0; i<nitems; i++) {
		items[i].label	   = argv[8*i];
		items[i].ylabel	   = atoi(argv[8*i+1]);
		items[i].xlabel	   = atoi(argv[8*i+2]);
		items[i].item	   = argv[8*i+3];
		items[i].yitem	   = atoi(argv[8*i+4]);
		items[i].xitem	   = atoi(argv[8*i+5]);

		itemlen = atoi(argv[8*i+6]);
		items[i].itemlen   = abs(itemlen);

		inputlen = atoi(argv[8*i+7]);
		items[i].inputlen = inputlen == 0 ? abs(itemlen) : inputlen;

		flags = flags | (itemlen < 0 ? ITEMREADONLY : 0);
		items[i].itemflags = flags;
	}

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

int
bsddialog_passwordform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv)
{
	int i, output, nitems, itemlen, inputlen;
	unsigned int flags = ITEMHIDDEN;
	struct formitem items[128];

	if ((argc % 8) != 0)
		return (-1);

	nitems = argc / 8;
	for (i=0; i<nitems; i++) {
		items[i].label	   = argv[8*i];
		items[i].ylabel	   = atoi(argv[8*i+1]);
		items[i].xlabel	   = atoi(argv[8*i+2]);
		items[i].item	   = argv[8*i+3];
		items[i].yitem	   = atoi(argv[8*i+4]);
		items[i].xitem	   = atoi(argv[8*i+5]);

		itemlen = atoi(argv[8*i+6]);
		items[i].itemlen   = abs(itemlen);

		inputlen = atoi(argv[8*i+7]);
		items[i].inputlen = inputlen == 0 ? abs(itemlen) : inputlen;

		flags = flags | (itemlen < 0 ? ITEMREADONLY : 0);
		items[i].itemflags = flags;
	}

	output = do_mixedform(conf, text, rows, cols, formheight, nitems, items);

	return output;
}

/*
 * SECTION 5 "Editor":  editbox(todo)
 */
int bsddialog_editbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{

	return (BSDDIALOG_ERROR);
}

/*
 * SECTION 6 "Bar": gauge - mixedgauge - rangebox - pause
 */
static void
draw_perc_bar(WINDOW *win, int y, int x, int size, int perc, bool withlabel, int label)
{
	char labelstr[128];
	int i, blue_x, color;

	blue_x = (int)((perc*(size))/100);

	wmove(win, y, x);
	for (i = 0; i < size; i++) {
		color = (i <= blue_x) ? t.currbarcolor : t.barcolor;
		wattron(win, color);
		waddch(win, ' ');
		wattroff(win, color);
	}

	if (withlabel)
		sprintf(labelstr, "%d", label);
	else
		sprintf(labelstr, "%3d%%", perc);
	wmove(win, y, x + size/2 - 2);
	for (i=0; i<strlen(labelstr); i++) {
		color = ( (blue_x + 1) <= (size/2 - strlen(labelstr)/2 + i) ) ?
		    t.barcolor : t.currbarcolor;
		wattron(win, color);
		waddch(win, labelstr[i]);
		wattroff(win, color);
	}
}

int bsddialog_gauge(struct bsddialog_conf conf, char* text, int rows, int cols, int perc)
{
	WINDOW *widget, *bar, *shadow;
	char input[2048];
	int i, y, x;
	bool mainloop = true;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    false) <0)
		return -1;

	bar = new_window(y+rows -4, x+3, 3, cols-6, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines);

	wrefresh(widget);
	wrefresh(bar);

	while (mainloop) {
		draw_perc_bar(bar, 1, 1, cols-8, perc, false, -1 /*unused*/);

		wrefresh(widget);
		wrefresh(bar);

		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
		}
		scanf("%d", &perc);
		perc = perc < 0 ? 0 : perc;
		perc = perc > 100 ? 100 : perc;
		i = 2;
		wmove(widget, 1, 1);
		wclrtoeol(widget);
		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
			//print_text(conf, widget, 1, 1, cols-2, input);
			mvwaddstr(widget, 1, i, input);
			i = i + strlen(input) + 1;
			wrefresh(widget);
		}
	}

	delwin(bar);
	widget_end(conf, "Gauge", widget, rows, cols, shadow);

	return BSDDIALOG_YESOK;
}

int bsddialog_mixedgauge(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int perc, int argc, char **argv)
{
	WINDOW *widget, *bar, *shadow;
	int i, miniperc, y, x;
	char states[11][16] = {
	    "[  Succeeded  ]",
	    "[   Failed    ]",
	    "[   Passed    ]",
	    "[  Completed  ]",
	    "[   Checked   ]",
	    "[    Done     ]",
	    "[   Skipped   ]",
	    "[ In Progress ]",
	    "!!!  BLANK  !!!",
	    "[     N/A     ]",
	    "[   UNKNOWN   ]",};

	if (widget_init(conf, &widget, &y, &x, NULL, &rows, &cols, &shadow,
	    false) <0)
		return -1;

	bar = new_window(y+rows -4, x+3, 3, cols-6, NULL, conf.hline,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines);

	/* mini bars */
	for (i=0; i < (argc/2); i++) {
		miniperc = atol(argv[i*2 + 1]);
		if (miniperc == 8)
			continue;
		mvwaddstr(widget, i+1, 2, argv[i*2]);
		if (miniperc > 9)
			mvwaddstr(widget, i+1, cols-2-15, states[10]);
		else if (miniperc >= 0 && miniperc <= 9)
			mvwaddstr(widget, i+1, cols-2-15, states[miniperc]);
		else { //miniperc < 0
			miniperc = abs(miniperc);
			mvwaddstr(widget, i+1, cols-2-15, "[             ]");
			draw_perc_bar(widget, i+1, 1+cols-2-15, 13, miniperc,
			    false, -1 /*unused*/);
		}
	}

	print_text(conf, widget, rows-6, 2, cols-2, text);

	/* main bar */
	draw_perc_bar(bar, 1, 1, cols-8, perc, false, -1 /*unused*/);

	wattron(bar, t.barcolor);
	mvwaddstr(bar, 0, 2, "Overall Progress");
	wattroff(bar, t.barcolor);

	wrefresh(widget);
	wrefresh(bar);

	getch();

	delwin(bar);
	widget_end(conf, "Mixedgaugebox", widget, rows, cols, shadow);

	return BSDDIALOG_YESOK;
}

int
bsddialog_rangebox(struct bsddialog_conf conf, char* text, int rows, int cols, int min,
    int max, int def)
{
	WINDOW *widget, *bar, *shadow;
	int y, x;
	bool loop, buttupdate, barupdate;
	int input, currvalue, output, sizebar;
	float perc;
	int positions = max - min + 1;
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	bar = new_window(y + rows - 6, x +7, 3, cols-14, NULL, NULL,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	currvalue = def;
	sizebar = cols - 16;
	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			perc = ((float)(currvalue - min)*100) / ((float)positions-1);
			draw_perc_bar(bar, 1, 1, sizebar, perc, true, currvalue);
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(widget, rows-2, cols, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}

		input = getch();
		switch(input) {
		case 10: // Enter
			output = bs.value[bs.curr]; // values -> outputs
			loop = false;
			dprintf(conf.output_fd, "%d", currvalue);
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
		case KEY_UP:
			if (currvalue < max) {
				currvalue++;
				barupdate = true;
			}
			break;
		case KEY_DOWN:
			if (currvalue > min) {
				currvalue--;
				barupdate = true;
			}
			break;
		}
	}

	delwin(bar);
	widget_end(conf, "Rangebox", widget, rows, cols, shadow);

	return output;
}

int bsddialog_pause(struct bsddialog_conf conf, char* text, int rows, int cols, int sec)
{
	WINDOW *widget, *bar, *shadow;
	int output, y, x;
	bool loop, buttupdate, barupdate;
	int input, currvalue, sizebar;
	float perc;
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	bar = new_window(y + rows - 6, x +7, 3, cols-14, NULL, NULL,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	currvalue = sec;
	sizebar = cols-16;
	nodelay(stdscr, TRUE);
	timeout(1000);
	//wtimeout(buttwin, 2);
	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			perc = ((float)(currvalue*100)) / ((float)sec);
			draw_perc_bar(bar, 1, 1, sizebar, perc, true, currvalue);
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(widget, rows-2, cols, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}

		input = getch();
		if(input < 0) {
			currvalue--;
			if (currvalue < 0) {
				output = BSDDIALOG_ERROR;
				break;
			}
			else {
				barupdate = true;
				continue;
			}
		}
		switch(input) {
		case 10: // Enter
			output = bs.value[bs.curr]; // values -> outputs
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
	}

	nodelay(stdscr, FALSE);

	delwin(bar);
	widget_end(conf, "Pause", widget, rows, cols, shadow);

	return output;
}

/*
 * SECTION 7 "Time": timebox - calendar
 */
int bsddialog_timebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int hh, unsigned int mm, unsigned int ss)
{
	WINDOW *widget, *shadow;
	char stringtime[1024];
	int i, input, output, y, x, sel;
	struct buttons bs;
	bool loop, buttupdate;
	time_t clock;
	struct tm *localtm;
	struct myclockstruct {
		unsigned int max;
		unsigned int curr;
		WINDOW *win;
	} c[3] = { {23, hh, NULL}, {59, mm, NULL}, {59, ss, NULL} };

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	c[0].win = new_window(y + rows - 6, x + cols/2 - 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 - 3, ':');
	c[1].win = new_window(y + rows - 6, x + cols/2 - 2, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 + 2, ':');
	c[2].win = new_window(y + rows - 6, x + cols/2 + 3, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	sel=0;
	curs_set(2);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, rows-2, cols, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}

		for (i=0; i<3; i++) {
			mvwprintw(c[i].win, 1, 1, "%2d", c[i].curr);
			wrefresh(c[i].win);
		}
		wmove(c[sel].win, 1, 2);
		wrefresh(c[sel].win);

		input = getch();
		switch(input) {
		case 10: // Enter
			output = bs.value[bs.curr]; // values -> outputs
			loop = false;
			if (conf.time_format == NULL) {
				dprintf(conf.output_fd, "%u:%u:%u", hh, mm, ss);
			} else {
				time(&clock);
				localtm = localtime(&clock);
				localtm->tm_hour = c[0].curr;
				localtm->tm_min  = c[1].curr;
				localtm->tm_sec  = c[2].curr;
				clock = mktime(localtm);
				localtm = localtime(&clock);
				strftime(stringtime, 1024, conf.time_format, localtm);
				dprintf(conf.output_fd, "%s", stringtime);
			}
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			sel = (sel + 1) % 3;
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
		case KEY_UP:
			c[sel].curr = c[sel].curr < c[sel].max ? c[sel].curr + 1 : 0;
			break;
		case KEY_DOWN:
			c[sel].curr = c[sel].curr > 0 ? c[sel].curr - 1 : c[sel].max;
			break;
		}
	}

	curs_set(0);

	for (i=0; i<3; i++)
		delwin(c[i].win);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}

int bsddialog_calendar(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int yy, unsigned int mm, unsigned int dd)
{
	WINDOW *widget, *shadow;
	char stringtime[1024];
	int i, input, output, y, x, sel;
	struct buttons bs;
	bool loop, buttupdate;
	time_t clock;
	struct tm *localtm;
	struct calendar {
		unsigned int max;
		unsigned int curr;
		WINDOW *win;
		unsigned int x;
	} c[3] = {{9999, yy, NULL, 4 }, {12, mm, NULL, 9 }, {31, dd, NULL, 2 }};
	struct month {
		char *name;
		unsigned int days;
	} m[12] = {
	    { "January", 30 }, { "February", 30 }, { "March",     30 },
	    { "April",   30 }, { "May",      30 }, { "June",      30 },
	    { "July",    30 }, { "August",   30 }, { "September", 30 },
	    { "October", 30 }, { "November", 30 }, { "December",  30 }
	};

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	c[0].win = new_window(y + rows - 6, x + cols/2 - 12, 3, 6, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 - 6, '/');
	c[1].win = new_window(y + rows - 6, x + cols/2 - 5, 3, 11, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);
	mvwaddch(widget, rows - 5, cols/2 + 6, '/');
	c[2].win = new_window(y + rows - 6, x + cols/2 + 7, 3, 4, NULL, NULL,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines);

	wrefresh(widget);

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	sel=2;
	curs_set(2);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(widget, rows-2, cols, bs, true);
			wrefresh(widget);
			buttupdate = false;
		}

		mvwprintw(c[0].win, 1, 1, "%4d", c[0].curr);
		mvwprintw(c[1].win, 1, 1, "%9s", m[c[1].curr-1].name);
		mvwprintw(c[2].win, 1, 1, "%2d", c[2].curr);
		for (i=0; i<3; i++) {
			wrefresh(c[i].win);
		}
		wmove(c[sel].win, 1, c[sel].x);
		wrefresh(c[sel].win);

		input = getch();
		switch(input) {
		case 10: // Enter
			output = bs.value[bs.curr]; // values -> outputs
			loop = false;
			if (conf.date_format == NULL) {
				dprintf(conf.output_fd, "%u/%u/%u",
				    c[0].curr, c[1].curr, c[2].curr);
			} else {
				time(&clock);
				localtm = localtime(&clock);
				localtm->tm_year = c[0].curr - 1900;
				localtm->tm_mon  = c[1].curr;
				localtm->tm_mday = c[2].curr;
				clock = mktime(localtm);
				localtm = localtime(&clock);
				strftime(stringtime, 1024, conf.date_format, localtm);
				dprintf(conf.output_fd, "%s", stringtime);
			}
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			sel = (sel + 1) % 3;
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
		case KEY_UP:
			c[sel].curr = c[sel].curr < c[sel].max ? c[sel].curr + 1 : 1;
			break;
		case KEY_DOWN:
			c[sel].curr = c[sel].curr > 1 ? c[sel].curr - 1 : c[sel].max;
			break;
		}
	}

	curs_set(0);

	for (i=0; i<3; i++)
		delwin(c[i].win);
	widget_end(conf, "Timebox", widget, rows, cols, shadow);

	return output;
}

/*
 * SECTION 8 "Command": prgbox - programbox(todo) - progressbox(todo)
 */
static int
command_handler(WINDOW *window, int y, int cols, struct buttons bs, bool shortkey)
{
	bool loop, update;
	int i, input;
	int output;

	loop = update = true;
	while(loop) {
		if (update) {
			draw_buttons(window, y, cols, bs, shortkey);
			update = false;
		}
		wrefresh(window);
		input = getch();
		switch (input) {
		case 10: /* Enter */
			output = bs.value[bs.curr];
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': /* TAB */
			bs.curr = (bs.curr + 1) % bs.nbuttons;
			update = true;
			break;
		case KEY_LEFT:
			if (bs.curr > 0) {
				bs.curr--;
				update = true;
			}
			break;
		case KEY_RIGHT:
			if (bs.curr < bs.nbuttons - 1) {
				bs.curr++;
				update = true;
			}
			break;
		default:
			if (shortkey) {
				for (i = 0; i < bs.nbuttons; i++)
					if (input == (bs.label[i])[0]) {
						output = bs.value[i];
						loop = false;
				}
			}
		}
	}

	return output;
}

int
bsddialog_prgbox(struct bsddialog_conf conf, char* text, int rows, int cols, char *command)
{
	char line[MAXINPUT];
	WINDOW *widget, *pad, *shadow;
	int i, y, x, padrows, padcols, ys, ye, xs, xe;
	int output;
	int pipefd[2];
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), false, NULL, false, conf.help_button,
	    BUTTONLABEL(help_label));

	if (text != NULL && conf.no_lines == false) {
		print_text(conf, widget, 1, 1, cols-2, text);
		mvwhline(widget, 2, 2, conf.ascii_lines ? '-' : ACS_HLINE, cols -4);
		wrefresh(widget);
	}

	padrows = text == NULL ? rows - 4 : rows - 6;
	padcols = cols - 2;
	ys = text == NULL ? y + 1 : y + 3;
	xs = x + 1;
	ye = ys + padrows;
	xe = xs + padcols;

	pad = newpad(padrows, padcols);
	wbkgd(pad, t.widgetcolor);

	pipe(pipefd);
	if (fork() == 0)
	{
		close(pipefd[0]);    // close reading

		dup2(pipefd[1], 1);  // send stdout to the pipe
		dup2(pipefd[1], 2);  // send stderr to the pipe

		close(pipefd[1]);    // this descriptor is no longer needed

		//const char *ls="/bin/ls";
		execl(command, command, NULL);
		return 0;
	}
	else
	{
		close(pipefd[1]);  // close write

		i = 0;
		while (read(pipefd[0], line, MAXINPUT) != 0) {
			mvwaddstr(pad, i, 0, line);
			prefresh(pad, 0, 0, ys, xs, ye, xe);
			i++;
		}
	}

	output = command_handler(widget, rows-2, cols, bs, true);

	return output;
}

int bsddialog_programbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	char line[MAXINPUT];
	WINDOW *widget, *pad, *shadow;
	int i, y, x, padrows, padcols, ys, ye, xs, xe, output;
	struct buttons bs;

	if (widget_init(conf, &widget, &y, &x, text, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	get_buttons(&bs, !conf.no_ok, BUTTONLABEL(ok_label), conf.extra_button,
	    BUTTONLABEL(extra_label), !conf.no_cancel, BUTTONLABEL(cancel_label),
	    conf.defaultno, conf.help_button, BUTTONLABEL(help_label));

	if (text != NULL && conf.no_lines == false) {
		mvwhline(widget, 2, 2, conf.ascii_lines ? '-' : ACS_HLINE, cols -4);
		wrefresh(widget);
	}

	padrows = text == NULL ? rows - 4 : rows - 6;
	padcols = cols - 2;
	ys = text == NULL ? y + 1 : y + 3;
	xs = x + 1;
	ye = ys + padrows;
	xe = xs + padcols;

	pad = newpad(padrows, padcols);

	i = 0;
	//while (fgets(line, MAXINPUT, stdin) != NULL) {
	while(getstr(line) != ERR){
		mvwaddstr(pad, i, 0, line);
		prefresh(pad, 0, 0, ys, xs, ye, xe);
		i++;
	}
	
	output = command_handler(widget, rows-2, cols, bs, true);

	return output;
}

int bsddialog_progressbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{
	return 0;
}

/*
 * SECTION 9 "Text": tailbox - tailboxbg(todo) - textbox
 */
enum textmode { TAILMODE, TAILBGMODE, TEXTMODE};

static int
do_text(enum textmode mode, struct bsddialog_conf conf, char* path, int rows, int cols)
{
	WINDOW *widget, *pad, *shadow;
	int i, input, y, x, padrows, padcols, ypad, xpad, ys, ye, xs, xe;
	char buf[BUFSIZ], *exitbutt ="EXIT";
	FILE *fp;
	bool loop;

	if (widget_init(conf, &widget, &y, &x, NULL, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	exitbutt = conf.exit_label == NULL ? exitbutt : conf.exit_label;
	draw_button(widget, rows-2, (cols-2)/2 - strlen(exitbutt)/2, strlen(exitbutt)+2,
	    exitbutt, true, true);

	wrefresh(widget);

	padrows = rows - 4;
	padcols = cols - 2;
	pad = newpad(padrows, padcols);
	wbkgd(pad, t.widgetcolor);

	fp = fopen(path, "r");
	/*if (mode == TAILMODE) {
		fseek (fp, 0, SEEK_END);
		i = nlines = 0;
		while (i < padrows) {
			line = ;
		}
		for (i=padrows-1; i--; i>=0) {
		}
	}*/
	i = 0;
	while(fgets(buf, BUFSIZ, fp) != NULL) {
		if (strlen(buf) > padcols) {
			padcols = strlen(buf);
			wresize(pad, padrows, padcols);
		}
		if (i > padrows-1) {
			padrows++;
			wresize(pad, padrows, padcols);
		}
		mvwaddstr(pad, i, 0, buf);
		i++;
	}

	ys = y + 1;
	xs = x + 1;
	ye = ys + rows-5;
	xe = xs + cols-3;
	ypad = xpad = 0;
	loop = true;
	while(loop) {
		prefresh(pad, ypad, xpad, ys, xs, ye, xe);
		input = getch();
		switch(input) {
		case 10: // Enter
			loop = false;
			break;
		case 27: // Esc
			loop = false;
			break;
		case KEY_LEFT:
			xpad = xpad > 0 ? xpad - 1 : 0;
			break;
		case KEY_RIGHT:
			xpad = (xpad + cols-2) < padcols-1 ? xpad + 1 : xpad;
			break;
		case KEY_UP:
			ypad = ypad > 0 ? ypad - 1 : 0;
			break;
		case KEY_DOWN:
			ypad = (ypad + rows-4) <= padrows ? ypad + 1 : ypad;
			break;
		}
	}

	return (BSDDIALOG_ERROR);
}

int bsddialog_tailbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{

	return (do_text(TAILMODE, conf, text, rows, cols));
}

int bsddialog_tailboxbg(struct bsddialog_conf conf, char* text, int rows, int cols)
{

	return (do_text(TAILBGMODE, conf, text, rows, cols));
}


int bsddialog_textbox(struct bsddialog_conf conf, char* text, int rows, int cols)
{

	return (do_text(TEXTMODE, conf, text, rows, cols));
}

/*
 * SECTION 10 "File": dselect(todo) - fselect(todo)
 */
int bsddialog_dselect(struct bsddialog_conf conf, char* text, int rows, int cols)
{

	return (BSDDIALOG_ERROR);
}

int bsddialog_fselect(struct bsddialog_conf conf, char* text, int rows, int cols)
{

	return (BSDDIALOG_ERROR);
}
