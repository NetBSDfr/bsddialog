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

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

#include "theme.h"

#define BSD_COLOR(BACKGROUND, FOREGROUND) (BACKGROUND * 8 + FOREGROUND +1)

struct bsddialog_theme t;

static void settheme(struct bsddialog_theme newtheme);

static struct bsddialog_theme defaulttheme = {
	.shadowcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_BLACK)),
	.backgroundcolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,  COLOR_BLUE))  | A_BOLD,
	.surroundtitle   = false,
	.titlecolor      = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,
	.lineraisecolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_WHITE)) | A_BOLD,
	.linelowercolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
	.widgetcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)),

	.curritemcolor   = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.itemcolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
	.currtagcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW,COLOR_BLUE))  | A_BOLD,
	.tagcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD,
	.fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD,
	.fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE))| A_BOLD,

	.currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.currbuttoncolor = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, COLOR_BLUE)) | A_BOLD,
	.buttoncolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),
	.currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,COLOR_BLUE)) | A_BOLD,
	.shortkeycolor   = COLOR_PAIR(BSD_COLOR(COLOR_RED,    COLOR_WHITE))| A_BOLD,

	.bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
};

static struct bsddialog_theme purpletheme = {
	.shadowcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_BLACK)),
	.backgroundcolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_MAGENTA))| A_BOLD,
	.surroundtitle   = true,
	.titlecolor      = COLOR_PAIR(BSD_COLOR(COLOR_RED,   COLOR_WHITE)),
	.lineraisecolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)),
	.linelowercolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)),
	.widgetcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)),

	.curritemcolor   = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.itemcolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
	.currtagcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW,COLOR_BLUE))  | A_BOLD,
	.tagcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD,
	.fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD,
	.fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE))| A_BOLD,

	.currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.currbuttoncolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_RED)),
	.buttoncolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),
	.currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,COLOR_RED)),
	.shortkeycolor   = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),

	.bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
};

static struct bsddialog_theme bluetheme = {
	.shadowcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_BLACK)),
	.backgroundcolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD,
	.surroundtitle   = true,
	.titlecolor      = COLOR_PAIR(BSD_COLOR(COLOR_RED,   COLOR_WHITE)),
	.lineraisecolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)),
	.linelowercolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)),
	.widgetcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,

	.curritemcolor   = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.itemcolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
	.currtagcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW,COLOR_BLUE))  | A_BOLD,
	.tagcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD,
	.fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD,
	.fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE))| A_BOLD,

	.currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.currbuttoncolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_RED)),
	.buttoncolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),
	.currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,COLOR_RED)),
	.shortkeycolor   = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),

	.bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,
};

static void settheme(struct bsddialog_theme newtheme)
{
	t.shadowcolor     = newtheme.shadowcolor;
	t.backgroundcolor = newtheme.backgroundcolor;
	t.surroundtitle   = newtheme.surroundtitle;
	t.titlecolor      = newtheme.titlecolor;
	t.lineraisecolor  = newtheme.lineraisecolor;
	t.linelowercolor  = newtheme.linelowercolor;
	t.widgetcolor     = newtheme.widgetcolor;

	t.curritemcolor   = newtheme.widgetcolor;
	t.itemcolor       = newtheme.itemcolor;
	t.currtagcolor    = newtheme.currtagcolor;
	t.tagcolor        = newtheme.tagcolor;

	t.currfieldcolor  = newtheme.currfieldcolor;
	t.fieldcolor      = newtheme.fieldcolor;
	t.fieldreadonlycolor = newtheme.fieldreadonlycolor;

	t.currbarcolor    = newtheme.currbarcolor;
	t.barcolor        = newtheme.barcolor;

	t.currbuttoncolor = newtheme.currbuttoncolor;
	t.buttoncolor     = newtheme.buttoncolor;
	t.currshortkeycolor = newtheme.currshortkeycolor;
	t.shortkeycolor   = newtheme.shortkeycolor;

	t.bottomtitlecolor= newtheme.bottomtitlecolor;
}

int bsddialog_settheme(enum bsddialog_default_theme t)
{
	int error = 0;

	if (t == BSDDIALOG_THEME_DEFAULT || t == BSDDIALOG_THEME_DIALOG)
		settheme(defaulttheme);
	else if ( t == BSDDIALOG_THEME_PURPLE)
		settheme(purpletheme);
	else if (t == BSDDIALOG_THEME_BLUE)
		settheme(bluetheme);
	else
		error = -1;

	return error;
}
