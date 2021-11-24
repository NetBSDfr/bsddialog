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
#include "bsddialog_theme.h"

/* "Text": tailbox - tailboxbg - textbox */

extern struct bsddialog_theme t;

enum textmode { TAILMODE, TAILBGMODE, TEXTMODE};

static int
do_text(enum textmode mode, struct bsddialog_conf conf, char* path, int rows, int cols)
{
	WINDOW *widget, *pad, *shadow;
	int i, input, y, x, padrows, padcols, ypad, xpad, ys, ye, xs, xe;
	char buf[BUFSIZ], *exitbutt ="EXIT";
	FILE *fp;
	bool loop;
	int output;

	if (mode == TAILMODE || mode == TAILBGMODE) {
		bsddialog_msgbox(conf, "Tailbox and Tailboxbg unimplemented", rows, cols);
		RETURN_ERROR("Tailbox and Tailboxbg unimplemented");
	}

	if (new_widget(conf, &widget, &y, &x, NULL, &rows, &cols, &shadow,
	    true) <0)
		return -1;

	exitbutt = conf.button.exit_label == NULL ? exitbutt : conf.button.exit_label;
	draw_button(widget, rows-2, (cols-2)/2 - strlen(exitbutt)/2, strlen(exitbutt)+2,
	    exitbutt, true, true);

	wrefresh(widget);

	padrows = 1;
	padcols = 1;
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
		if ((int) strlen(buf) > padcols) {
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
		case 10: /* Enter */
			output = BSDDIALOG_YESOK;
			loop = false;
			break;
		case 27: /* Esc */
			output = BSDDIALOG_ESC;
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

	/* to improve: name, rows and cols, now only for F1 */
	end_widget(conf, widget, rows, cols, shadow);

	return output;
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

