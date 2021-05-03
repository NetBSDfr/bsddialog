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


static struct bsddialog_theme defaulttheme = {
#define bgwidget  COLOR_WHITE
#define bgcurr    COLOR_YELLOW
	.shadowcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_BLACK)),
	.backgroundcolor = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_CYAN)),
	.surroundtitle   = true,
	.titlecolor      = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, bgwidget)) ,
	.lineraisecolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  bgwidget)),
	.linelowercolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  bgwidget)),
	.widgetcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  bgwidget)),

	.curritemcolor   = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  bgcurr)),
	.itemcolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  bgwidget)),
	.currtagcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  bgcurr)),
	.tagcolor        = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, bgwidget)),
	.namesepcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, bgwidget)),
	.descsepcolor    = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  bgwidget)),

	.currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)),
	.fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)),
	.fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE)),

	.currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE)),
	.barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)),

	.buttleftch      = '[',
	.buttrightchar   = ']',
	.currbuttdelimcolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, bgcurr)),
	.buttdelimcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, bgwidget)),
	.currbuttoncolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,    bgcurr))  | A_UNDERLINE,
	.buttoncolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,    bgwidget))| A_UNDERLINE,
	.currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  bgcurr))  | A_UNDERLINE,
	.shortkeycolor     = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, bgwidget))| A_UNDERLINE,

	.bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, bgwidget))
};

static struct bsddialog_theme dialogtheme = {
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
	.namesepcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW,COLOR_WHITE)) | A_BOLD,
	.descsepcolor    = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD,

	.currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD,
	.fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD,
	.fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE))| A_BOLD,

	.currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.buttleftch      = '[',
	.buttrightchar   = ']',
	.currbuttdelimcolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD,
	.buttdelimcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),
	.currbuttoncolor = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, COLOR_BLUE)) | A_BOLD,
	.buttoncolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE)),
	.currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,COLOR_BLUE)) | A_BOLD,
	.shortkeycolor   = COLOR_PAIR(BSD_COLOR(COLOR_RED,    COLOR_WHITE))| A_BOLD,

	.bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD
};

static struct bsddialog_theme magentatheme = {
	.shadowcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_BLACK)),
	.backgroundcolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_MAGENTA))| A_BOLD,
	.surroundtitle   = true,
	.titlecolor      = COLOR_PAIR(BSD_COLOR(COLOR_RED,   COLOR_CYAN)),
	.lineraisecolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_CYAN)) | A_BOLD,
	.linelowercolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_CYAN)),
	.widgetcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_CYAN)),

	.curritemcolor   = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE)) | A_BOLD,
	.itemcolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_CYAN)) | A_BOLD,
	.currtagcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW,COLOR_BLUE)) | A_BOLD,
	.tagcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_CYAN)) | A_BOLD,
	.namesepcolor    = COLOR_PAIR(BSD_COLOR(COLOR_RED,   COLOR_CYAN)) | A_BOLD,
	.descsepcolor    = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_CYAN)) | A_BOLD,

	.currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD,
	.fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD,
	.fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE))| A_BOLD,

	.currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD,
	.barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD,

	.buttleftch      = '<',
	.buttrightchar   = '>',
	.currbuttdelimcolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_RED)) | A_BOLD,
	.buttdelimcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_CYAN)),
	.currbuttoncolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_RED)),
	.buttoncolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_CYAN)),
	.currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_RED)) | A_BOLD,
	.shortkeycolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_CYAN)),

	.bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_CYAN)) | A_BOLD
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

	t.curritemcolor   = newtheme.curritemcolor;
	t.itemcolor       = newtheme.itemcolor;
	t.currtagcolor    = newtheme.currtagcolor;
	t.tagcolor        = newtheme.tagcolor;
	t.namesepcolor    = newtheme.namesepcolor;
	t.descsepcolor    = newtheme.descsepcolor;

	t.currfieldcolor  = newtheme.currfieldcolor;
	t.fieldcolor      = newtheme.fieldcolor;
	t.fieldreadonlycolor = newtheme.fieldreadonlycolor;

	t.currbarcolor    = newtheme.currbarcolor;
	t.barcolor        = newtheme.barcolor;

	t.buttleftch      = newtheme.buttleftch;
	t.buttrightchar   = newtheme.buttrightchar;
	t.currbuttdelimcolor = newtheme.currbuttdelimcolor;
	t.buttdelimcolor     = newtheme.buttdelimcolor;
	t.currbuttoncolor = newtheme.currbuttoncolor;
	t.buttoncolor     = newtheme.buttoncolor;
	t.currshortkeycolor = newtheme.currshortkeycolor;
	t.shortkeycolor   = newtheme.shortkeycolor;

	t.bottomtitlecolor= newtheme.bottomtitlecolor;

	bkgd(t.backgroundcolor);
}


int bsddialog_settheme(enum bsddialog_default_theme t)
{
	int error = 0;

	if (t == BSDDIALOG_THEME_DEFAULT)
		settheme(defaulttheme);
	else if ( t == BSDDIALOG_THEME_DIALOG)
		settheme(dialogtheme);
	else if ( t == BSDDIALOG_THEME_MAGENTA)
		settheme(magentatheme);
	else {
		settheme(defaulttheme);
		error = -1;
	}

	refresh();

	return error;
}
