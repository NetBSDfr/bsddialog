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

static void set_dialog_theme(struct bsddialogtheme *s)
{
	s->backgroundcolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,  COLOR_BLUE))  | A_BOLD;

	s->shadowcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_BLACK));
	s->titlecolor      = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD;
	s->lineraisecolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_WHITE)) | A_BOLD;
	s->linelowercolor  = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD;
	s->widgetcolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE));

	s->curritemcolor   = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD;
	s->itemcolor       = COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD;
	s->currtagcolor    = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW,COLOR_BLUE))  | A_BOLD;
	s->tagcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD;

	s->currfieldcolor  = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_BLUE)) | A_BOLD;
	s->fieldcolor      = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,  COLOR_CYAN)) | A_BOLD;
	s->fieldreadonlycolor = COLOR_PAIR(BSD_COLOR(COLOR_CYAN,COLOR_WHITE))| A_BOLD;

	s->currbarcolor    = COLOR_PAIR(BSD_COLOR(COLOR_WHITE, COLOR_BLUE))  | A_BOLD;
	s->barcolor        = COLOR_PAIR(BSD_COLOR(COLOR_BLUE,  COLOR_WHITE)) | A_BOLD;

	s->currbuttoncolor = COLOR_PAIR(BSD_COLOR(COLOR_YELLOW, COLOR_BLUE)) | A_BOLD;
	s->buttoncolor     = COLOR_PAIR(BSD_COLOR(COLOR_BLACK,  COLOR_WHITE));
	s->currshortkeycolor = COLOR_PAIR(BSD_COLOR(COLOR_WHITE,COLOR_BLUE)) | A_BOLD;
	s->shortkeycolor   = COLOR_PAIR(BSD_COLOR(COLOR_RED,    COLOR_WHITE))| A_BOLD;

	s->bottomtitlecolor= COLOR_PAIR(BSD_COLOR(COLOR_BLACK, COLOR_WHITE)) | A_BOLD;
}

int set_theme(enum bsddialog_theme t, struct bsddialogtheme *s)
{
	int error = 0;

	if (t == DIALOG || t == DEFAULT)
		set_dialog_theme(s);
	else
		error = -1;

	return error;
}
