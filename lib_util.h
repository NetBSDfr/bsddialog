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

#define MAX(a,b) (((a)>(b))?(a):(b))

#define LABEL_cancel_label "Cancel"
#define LABEL_exit_label   "EXIT"
#define LABEL_extra_label  "Extra"
#define LABEL_help_label   "Help"
#define LABEL_no_label     "No"
#define LABEL_ok_label     "OK"
#define LABEL_yes_label    "Yes"
#define BUTTONLABEL(l) (conf.l != NULL ? conf.l : LABEL_ ##l)

enum elevation { RAISED, LOWERED, NOLINES };

WINDOW *new_window(int y, int x, int rows, int cols, char *title,char *bottomtitle,
    enum elevation elev, bool asciilines, bool subwindowborders);
void window_scrolling_handler(WINDOW *pad, int rows, int cols);
void print_text(struct config conf, WINDOW *pad, int cols, char *text);

void draw_button(WINDOW *window, int x, int size, char *text, bool selected,
    bool shortkey);
#define MAXBUTTONS 4 /* yes|ok - extra - no|cancel - help */
struct buttons {
	unsigned int nbuttons;
	char *label[MAXBUTTONS];
	int value[MAXBUTTONS];
	int curr;
};
void draw_buttons(WINDOW *window, int cols, struct buttons bs, bool shortkey);
void get_buttons(struct buttons *bs, bool yesok, char* yesoklabel, bool extra,
    char *extralabel, bool nocancel, char *nocancellabel, bool defaultno,
    bool help, char *helplabel);

int widget_init(struct config conf, WINDOW **widget, int *y, int *x, char *text,
    int *h, int *w, WINDOW **shadow, bool buttons, WINDOW **buttonswin);

void widget_end(struct config conf, char *name, WINDOW *window, int h, int w,
    WINDOW *shadow, WINDOW *buttonswin);

#endif