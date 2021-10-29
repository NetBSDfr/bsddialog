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

#ifndef _BSDDIALOG_H_
#define _BSDDIALOG_H_

#include <stdbool.h>

/* Exit */
#define BSDDIALOG_YESOK		 0 // YES or OK buttons
#define BSDDIALOG_NOCANCEL	 1 // No or Cancel buttons
#define BSDDIALOG_HELP		 2 // Help button
#define BSDDIALOG_EXTRA		 3 // Extra button
#define BSDDIALOG_ITEM_HELP	 4
#define BSDDIALOG_ERROR		-1 // generic error or ESC key

struct bsddialog_conf {
	bool item_prefix; //BSDDialog extension
	bool ascii_lines;
	int aspect;	// aspect ratio
	//char *backtitle;
	int x;		// BEGIN
	int y;		// BEGIN
	char *cancel_label;
	bool clear;	// useful?
	bool colors;
	char *colums_separator;
	bool cr_wrap;
	char *create_rc;// useful?
	char *date_format;
	bool defaultno;
	char *default_button;
	char *default_item;
	char *exit_label;
	bool extra_button;
	char *extra_label;
	bool help;	//useful?
	bool help_button;
	char *help_label;
	bool help_status;
	bool help_tags;
	char *hfile;
	char *hline;
	//bool ignore; utility
	int input_fd;
	bool insecure;
	bool item_help;
	bool keep_tite;
	bool keep_window;
	bool last_key;
	int max_input;
	bool no_cancel;  // alias
	//bool nocancel; // alias
	bool no_collapse;
	bool no_items;
	bool no_kill;
	char *no_label;
	bool no_lines;
	bool no_mouse;
	bool no_nl_expand;
	bool no_ok;  // alias
	//bool nook; // alias
	//bool no_shadow; utility (.shadow for lib)
	bool no_tags;
	char *ok_label;
	int output_fd;
	char *separator;
	char *output_separator;
	bool print_maxsize; // useful?
	bool print_size; // useful?
	//bool print_version; utility
	bool quoted;
	bool scrollbar;
	bool separate_output;
	char *separate_witget;
	bool shadow;
	bool single_quoted;
	bool size_err;
	int sleep;
	//bool stderr_; utility, use .output_fd
	//bool stdout_; utility, use .output_fd
	bool tab_correct;
	int tab_len;
	char *time_format;
	int timeout;
	char *title;
	char *trace;
	bool trim;
	//bool version; utility
	bool visit_items;
	char *yes_label;
};

struct bsddialog_menuitem {
	bool on;
	int depth;
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

int  bsddialog_init(void);
void bsddialog_end(void);
int  bsddialog_backtitle(struct bsddialog_conf conf, char *backtitle);
const char *bsddialog_geterror(void);

/* widgets */
int bsddialog_buildlist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
int bsddialog_calendar(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int yy, unsigned int mm, unsigned int dd);
int bsddialog_checklist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
int bsddialog_dselect(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_editbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_form(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv);
int bsddialog_fselect(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_gauge(struct bsddialog_conf conf, char* text, int rows, int cols, int perc);
int bsddialog_infobox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_inputbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_inputmenu(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_menu(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
int bsddialog_mixedform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv);
int bsddialog_mixedgauge(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int perc, int argc, char **argv);
int bsddialog_mixedlist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int ngroups, struct bsddialog_menugroup *groups);
int bsddialog_msgbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_passwordbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_passwordform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv);
int bsddialog_pause(struct bsddialog_conf conf, char* text, int rows, int cols, int sec);
int bsddialog_prgbox(struct bsddialog_conf conf, char* text, int rows, int cols,
    char *command);
int bsddialog_programbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_progressbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_radiolist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
int bsddialog_rangebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    int min, int max, int def);
int bsddialog_tailbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_tailboxbg(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_textbox(struct bsddialog_conf conf, char* text, int rows, int cols);
int bsddialog_timebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int hh, unsigned int mm, unsigned int ss);
int bsddialog_treeview(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
int bsddialog_yesno(struct bsddialog_conf conf, char* text, int rows, int cols);

#endif