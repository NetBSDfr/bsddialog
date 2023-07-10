/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Alfonso Sabato Siciliano
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

#ifndef _LIBBSDDIALOG_UTIL_DIALOG_H_
#define _LIBBSDDIALOG_UTIL_DIALOG_H_

struct dialog {
	bool built;
	struct bsddialog_conf *conf;
	WINDOW *widget;   /* Size and position refer to widget */
	int y, x;         /* Current position, API conf.[y|x]: -1, >=0 */
	int rows, cols;   /* API rows and cols: -1, 0, >0 */
	int h, w;         /* Current height and width */
	const char *text; /* Checked API *text, at least "", fake for textbox */
	WINDOW *textpad;  /* Fake for textbox */
	int htext;        /* Real h text: 0, >0 */
	struct buttons bs;/* bs.nbuttons = 0 for no buttons */
	WINDOW *shadow;
};

int dialog_size_position(struct dialog *d ,int *htext, int hnotext, int minw);

void
draw_box(struct bsddialog_conf *conf, WINDOW *win, int y, int x, int h, int w,
    enum elevation elev);

void
textpad(struct dialog *d, int ytext, int xtext, int upnotext, int downnotext);

#define TEXTPAD(d, downnotext) textpad(d, 0, 0, 0, downnotext)
/* msgbox and yesno (ytext) */
//#define YTEXTPAD(d, ytext, downnotext) textpad(d, ytext, 0, 0, downnotext)
/* mixedgauge */
//#define YSTEXTPAD(d, upnotext, downnotext) textpad(d, 0, 0, upnotext, downnotext)
/* textbox */
//#define YXTEXTPAD(d, ytext, xtext, downnotext) textpad(pad, ytext, xtext, 0, downnotext)

/* Dialog build, update, destroy */
int hide2_dialog(struct dialog *d);
void destroy_dialog(struct dialog *d);
int draw_dialog(struct dialog *d);
int
prepare_dialog(struct bsddialog_conf *conf, const char *text, int rows,
    int cols, struct dialog *d);

#endif