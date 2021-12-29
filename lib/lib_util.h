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

#ifndef _LIBBSDDIALOG_UTIL_H_
#define _LIBBSDDIALOG_UTIL_H_

#define HBORDERS	2
#define VBORDERS	2

/* Set default aspect ratio to 9 */
#define GET_ASPECT_RATIO(conf) (conf->aspect_ratio > 0 ? conf->aspect_ratio : 9)

/* debug */
#define BSDDIALOG_DEBUG(y,x,fmt, ...) do {	\
	mvprintw(y, x, fmt, __VA_ARGS__);	\
	refresh();				\
} while (0)

/* error buffer */
const char *get_error_string(void);
void set_error_string(char *string);

#define RETURN_ERROR(str) do {			\
	set_error_string(str);			\
	return (BSDDIALOG_ERROR);		\
} while (0)

/* buttons */
#define BUTTON_OK_LABEL      "OK"
#define BUTTON_YES_LABEL     "Yes"
#define BUTTON_EXTRA_LABEL   "Extra"
#define BUTTON_CANCEL_LABEL  "Cancel"
#define BUTTON_NO_LABEL      "No"
#define BUTTON_HELP_LABEL    "Help"

#define MAXBUTTONS    6 /* ok + extra + cancel + help + 2 generics */
struct buttons {
	unsigned int nbuttons;
	char *label[MAXBUTTONS];
	int value[MAXBUTTONS];
	int curr;
	unsigned int sizebutton; /* including left and right delimiters */
};

void
get_buttons(struct bsddialog_conf *conf, struct buttons *bs, char *yesoklabel,
    char *nocancellabel);

void
draw_buttons(WINDOW *window, struct buttons bs, bool shortcut);

bool shortcut_buttons(int key, struct buttons *bs);

/* help window with F1 key */
int f1help(struct bsddialog_conf *conf);

/* cleaner */
int hide_widget(int y, int x, int h, int w, bool withshadow);

/* (auto) size and (auto) position */
int
get_text_properties(struct bsddialog_conf *conf, char *text, int *maxword,
    int *maxline, int *nlines);

int widget_max_height(struct bsddialog_conf *conf);
int widget_max_width(struct bsddialog_conf *conf);

int
widget_min_height(struct bsddialog_conf *conf, bool withbuttons, int minwidget);

int
widget_min_width(struct bsddialog_conf *conf, struct buttons *bs,
    int minwidget);

int
set_widget_size(struct bsddialog_conf *conf, int rows, int cols, int *h,
    int *w);

int
set_widget_position(struct bsddialog_conf *conf, int *y, int *x, int h, int w);

/* widget builders */
int
print_textpad(struct bsddialog_conf *conf, WINDOW *pad, int *rows, int cols,
    char *text);

enum elevation { RAISED, LOWERED };

void
draw_borders(struct bsddialog_conf *conf, WINDOW *win, int rows, int cols,
    enum elevation elev);

WINDOW *
new_boxed_window(struct bsddialog_conf *conf, int y, int x, int rows, int cols,
    enum elevation elev);

int
new_dialog(struct bsddialog_conf *conf, WINDOW **shadow, WINDOW **widget, int y,
    int x, int h, int w, WINDOW **textpad, char *text, struct buttons *bs,
    bool shortcutbuttons);

int
update_dialog(struct bsddialog_conf *conf, WINDOW *shadow, WINDOW *widget,
    int y, int x, int h, int w, WINDOW *textpad, char *text, struct buttons *bs,
    bool shortcutbuttons);

void
end_dialog(struct bsddialog_conf *conf, WINDOW *shadow, WINDOW *widget,
    WINDOW *textpad);

#endif
