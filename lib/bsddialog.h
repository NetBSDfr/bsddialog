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

#ifndef _LIBBSDDIALOG_H_
#define _LIBBSDDIALOG_H_

#include <stdbool.h>

#define LIBBSDDIALOG_VERSION    "0.1-devel"

/* Exit status */
#define BSDDIALOG_ERROR		-1
#define BSDDIALOG_YESOK		 0
#define BSDDIALOG_NOCANCEL	 1
#define BSDDIALOG_HELP		 2
#define BSDDIALOG_EXTRA		 3
#define BSDDIALOG_ITEM_HELP	 4
#define BSDDIALOG_ESC		 5
#define BSDDIALOG_GENERIC1       6
#define BSDDIALOG_GENERIC2       7

/* size and position */
#define BSDDIALOG_FULLSCREEN	-1
#define BSDDIALOG_AUTOSIZE	 0
#define BSDDIALOG_CENTER	-1

struct bsddialog_conf {

	bool ascii_lines;
	int aspect_ratio;
	int x;
	int y;
	bool clear;
	int *get_height;
	int *get_width;
	char *hfile;
	char *hline;
	/* int input_fd; */
	bool no_lines;
	bool shadow;
	int sleep;
	char *title;

	struct {
		bool colors;
		bool cr_wrap;
		bool no_collapse;
		bool no_nl_expand;
		/* bool tab_correct; textbox? */
		/* int tab_len; textbox? */
		bool trim;
	} text;

	struct {
		bool align_left;
		/* char *colums_separator; sade(8)? */
		char *default_item;
		bool no_items;
		bool no_tags;
	} menu;

	struct {
		int securech;
	} form;

	struct {
		char *cancel_label;
		bool defaultno;
		char *default_label;
		char *exit_label;
		bool extra_button;
		char *extra_label;
		char *generic1_label;
		char *generic2_label;
		bool help_button;
		char *help_label;
		bool no_cancel;
		char *no_label;
		bool no_ok;
		char *ok_label;
		char *yes_label;
	} button;
};

struct bsddialog_menuitem {
	char *prefix;
	bool  on;
	int   depth;
	char *name;
	char *desc;
	char *bottomdesc;
};

enum bsddialog_grouptype {
	BSDDIALOG_CHECKLIST,
	BSDDIALOG_RADIOLIST,
	BSDDIALOG_SEPARATOR,
};

struct bsddialog_menugroup {
	enum bsddialog_grouptype type;
	unsigned int nitems;
	struct bsddialog_menuitem *items;
};

struct bsddialog_formfield {
	char *label;
	unsigned int ylabel;
	unsigned int xlabel;

	char *init;
	unsigned int yform;
	unsigned int xform;
	int formlen;
	unsigned int maxvaluelen;
#define BSDDIALOG_FIELDHIDDEN   0x1
#define BSDDIALOG_FIELDREADONLY 0x2
	unsigned int flags;
	char *value; /* allocated memory, to free */
};

int bsddialog_init(void);
int bsddialog_end(void);
int bsddialog_backtitle(struct bsddialog_conf conf, char *backtitle);
const char *bsddialog_geterror(void);
int bsddialog_terminalheight(void);
int bsddialog_terminalwidth(void);
void bsddialog_initconf(struct bsddialog_conf *conf);

/* widgets */
int
bsddialog_buildlist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items,
    int *focusitem);

int
bsddialog_checklist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items,
    int *focusitem);

int
bsddialog_datebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int *yy, unsigned int *mm, unsigned int *dd);

int
bsddialog_form(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int formheight, unsigned int nfields,
    struct bsddialog_formfield *fields);

int
bsddialog_gauge(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int perc);

int
bsddialog_infobox(struct bsddialog_conf conf, char* text, int rows, int cols);

int
bsddialog_menu(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items,
    int *focusitem);

int
bsddialog_mixedgauge(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int mainperc, unsigned int nminbars, char **minibars);

int
bsddialog_mixedlist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int ngroups, struct bsddialog_menugroup *groups,
    int *focuslist, int *focusitem);

int
bsddialog_msgbox(struct bsddialog_conf conf, char* text, int rows, int cols);

int
bsddialog_pause(struct bsddialog_conf conf, char* text, int rows, int cols,
    int sec);

int
bsddialog_radiolist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items,
    int *focusitem);

int
bsddialog_rangebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    int min, int max, int *value);

int
bsddialog_textbox(struct bsddialog_conf conf, char* file, int rows, int cols);

int
bsddialog_timebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int *hh, unsigned int *mm, unsigned int *ss);

int bsddialog_yesno(struct bsddialog_conf conf, char* text, int rows, int cols);

#endif
