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

#ifndef _LIBBSDDIALOG_UTIL_H_
#define _LIBBSDDIALOG_UTIL_H_

#define BORDER          1
#define BORDERS         (BORDER + BORDER)
#define TEXTHMARGIN     1
#define TEXTHMARGINS    (TEXTHMARGIN + TEXTHMARGIN)

/* MIN and MAX */
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))

/* theme utils */
extern struct bsddialog_theme t;
extern bool hastermcolors;

/* debug */
#define BSDDIALOG_DEBUG(y,x,fmt, ...) do {                                     \
	mvprintw(y, x, fmt, __VA_ARGS__);                                      \
	refresh();                                                             \
} while (0)

/* unicode */
unsigned int strcols(const char *mbstring);
int str_props(const char *mbstring, unsigned int *cols, bool *has_multi_col);
void mvwaddwch(WINDOW *w, int y, int x, wchar_t wch);
wchar_t* alloc_mbstows(const char *mbstring);

/* error buffer */
const char *get_error_string(void);
void set_error_string(const char *string);
void set_fmt_error_string(const char *fmt, ...);

#define RETURN_ERROR(str) do {                                                 \
	set_error_string(str);                                                 \
	return (BSDDIALOG_ERROR);                                              \
} while (0)

#define RETURN_FMTERROR(fmt, ...) do {                                         \
	set_fmt_error_string(fmt, __VA_ARGS__);                                \
	return (BSDDIALOG_ERROR);                                              \
} while (0)

/* ptr checker */
#define CHECK_PTR(p, type) do {                                                \
	if (p == NULL)                                                         \
		RETURN_ERROR("*" #p " is NULL");                               \
	if (sizeof(*p) != sizeof(type))                                        \
		RETURN_ERROR("Bad *" #p " size");                              \
} while (0)

#define CHECK_ARRAY(nitem, a, type) do {                                       \
	if (nitem == 0)                                                        \
		/* no check*/;                                                 \
	else if(a == NULL)                                                     \
		RETURN_FMTERROR(#nitem " is %d but " #a " is NULL");           \
	else if (sizeof(*a) != sizeof(type))                                   \
		RETURN_ERROR("Bad " #a "[0] size");                            \
} while (0)

#define SET_PTR(p, type, value) do {                                           \
	if (p != NULL && sizeof(*p) != sizeof(type))                           \
		RETURN_ERROR(*p " is not NULL but bad (" #type ") size");      \
	if (p != NULL)                                                         \
		*p = value;                                                    \
} while (0)

/* buttons */
#define HBUTTONS        2

struct buttons {
	unsigned int nbuttons;
#define MAXBUTTONS 6 /* ok + extra + cancel + help + 2 generics */
	const char *label[MAXBUTTONS];
	bool shortcut;
	wchar_t first[MAXBUTTONS];
	int value[MAXBUTTONS];
	int curr;
#define BUTTONVALUE(bs) bs.value[bs.curr]
	unsigned int sizebutton; /* including left and right delimiters */
};

#define BUTTON_OK_LABEL      "OK"
#define BUTTON_CANCEL_LABEL  "Cancel"
void
get_buttons(struct bsddialog_conf *conf, struct buttons *bs, bool shortcut,
    const char *yesoklabel, const char *nocancellabel);

void draw_buttons(WINDOW *window, struct buttons bs);
int buttons_min_width(struct buttons bs);
bool shortcut_buttons(wint_t key, struct buttons *bs);

#define DRAW_REFRESH_BUTTONS(widget, bs) do {                                  \
	draw_buttons(widget, bs);                                              \
	wrefresh(widget);                                                      \
} while (0)

/* Clear Dialog */
int hide_dialog(int y, int x, int h, int w, bool withshadow);

/* help window with F1 key */
int f1help_dialog(struct bsddialog_conf *conf);

/* (auto) size and (auto) position */
#define SCREENLINES (getmaxy(stdscr))
#define SCREENCOLS  (getmaxx(stdscr))

int
set_widget_size(struct bsddialog_conf *conf, int rows, int cols, int *h,
    int *w);

int
set_widget_autosize(struct bsddialog_conf *conf, int rows, int cols, int *h,
    int *w, const char *text, int *rowstext, struct buttons *bs, int hnotext,
    int minw);

int widget_checksize(int h, int w, struct buttons *bs, int hnotext, int minw);

int
set_widget_position(struct bsddialog_conf *conf, int *y, int *x, int h, int w);

int
widget_size_position(struct bsddialog_conf *conf, int rows, int cols,
    const char *text, int hnotext, int minw, struct buttons *bs, int *y, int *x,
    int *h, int *w, int *htext);

/* widget builders */
enum elevation { RAISED, LOWERED };

void
draw_borders(struct bsddialog_conf *conf, WINDOW *win, int rows, int cols,
    enum elevation elev);

WINDOW *
new_boxed_window(struct bsddialog_conf *conf, int y, int x, int rows, int cols,
    enum elevation elev);

int
new_dialog(struct bsddialog_conf *conf, WINDOW **shadow, WINDOW **widget, int y,
    int x, int h, int w, WINDOW **textpad, const char *text, struct buttons *bs);

int
update_dialog(struct bsddialog_conf *conf, WINDOW *shadow, WINDOW *widget,
    int y, int x, int h, int w, WINDOW *textpad, const char *text,
    struct buttons *bs);

void
end_dialog(struct bsddialog_conf *conf, WINDOW *shadow, WINDOW *widget,
    WINDOW *textpad);

#endif
