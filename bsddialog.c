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

#include <sys/ioctl.h>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bsddialog.h"

#define BSDDIALOG_VERSION "0.1 devel"

/* Common options */
#define ASCII_LINES	1 // ascii-lines
#define ASPECT		2 // aspect
#define BACKTITLE	3 // backtitle
#define BEGIN		4 // begin
#define CANCEL_LABEL	5 // cancel-label
#define CLEAR		6 // clear
#define COLORS		7 // colors
#define COLUMN_SEPARATOR 8 // column-separator
#define CR_WRAP		9 // cr-wrap
#define CREATE_RC	10 // create-rc
#define DATE_FORMAT	11 // date-format
#define DEFAULTNO	12 // defaultno
#define DEFAULT_BUTTON	13 // default-button
#define DEFAULT_ITEM	14 // default-item
#define EXIT_LABEL	15 // exit-label
#define EXTRA_BUTTON	16 // extra-button
#define EXTRA_LABEL	17 // extra-label
#define HELP		18 // help
#define HELP_BUTTON	19 // help-button
#define HELP_LABEL	20 // help-label
#define HELP_STATUS	21 // help-status
#define HELP_TAGS	22 // help-tags
#define HFILE		23 // hfile
#define HLINE		24 // hline
#define IGNORE		25 // ignore
#define INPUT_FD	26 // input-fd
#define INSECURE	27 // insecure
#define ITEM_HELP	28 // item-help
#define KEEP_TITE	29 // keep-tite
#define KEEP_WINDOW	30 // keep-window
#define LAST_KEY	31 // last-key
#define MAX_INPUT	32 // max-input
#define NO_CANCEL	33 // no-cancel
#define NOCANCEL	34 // nocancel
#define NO_COLLAPSE	35 // no-collapse
#define NO_ITEMS	36 // no-items
#define NO_KILL		37 // no-kill
#define NO_LABEL	38 // no-label
#define NO_LINES	39 // no-lines
#define NO_MOUSE	40 // no-mouse
#define NO_NL_EXPAND	41 // no-nl-expand
#define NO_OK		42 // no-ok
#define NOOK		43 // nook 
#define NO_SHADOW	44 // no-shadow
#define NO_TAGS		45 // no-tags
#define OK_LABEL	46 // ok-label
#define OUTPUT_FD	47 // output-fd
#define SEPARATOR	48 // separator
#define OUTPUT_SEPARATOR 49 // output-separator
#define PRINT_MAXSIZE	50 // print-maxsize
#define PRINT_SIZE	51 // print-size
#define PRINT_VERSION	52 // print-version
#define QUOTED		53 // quoted
#define SCROLLBAR	54 // scrollbar
#define SEPARATE_OUTPUT	55 // separate-output
#define SEPARATE_WIDGET	56 // separate-widget
#define SHADOW		57 // shadow
#define SINGLE_QUOTED	58 // single-quoted
#define SIZE_ERR	59 // size-err
#define SLEEP		60 // sleep
#define STDERR		61 // stderr
#define STDOUT		62 // stdout
#define TAB_CORRECT	63 // tab-correct
#define TAB_LEN		64 // tab-len
#define TIME_FORMAT	65 // time-format
#define TIMEOUT		66 // timeout
#define TITLE		67 // title
#define TRACE		68 // trace
#define TRIM		69 // trim
#define VERSION		70 // version
#define VISIT_ITEMS	71 // visit-items
#define YES_LABEL	72 // yes-label
/* Widgets */
#define BUILDLIST	73 // buildlist
#define CALENDAR	74 // calendar
#define CHECKLIST	75 // checklist
#define DSELECT		76 // dselect
#define EDITBOX		77 // editbox
#define FORM		78 // form
#define FSELECT		79 // fselect
#define GAUGE		80 // gauge
#define INFOBOX		81 // infobox
#define INPUTBOX	82 // inputbox
#define INPUTMENU	83 // inputmenu
#define MENU		84 // menu
#define MIXEDFORM	85 // mixedform
#define MIXEDGAUGE	86 // mixedgauge
#define MSGBOX		87 // msgbox
#define PASSWORDBOX	88 // passwordbox
#define PASSWORDFORM	89 // passwordform
#define PAUSE		90 // pause
#define PRGBOX		91 // prgbox
#define PROGRAMBOX	92 // programbox
#define PROGRESSBOX	93 // progressbox
#define RADIOLIST	94 // radiolist
#define RANGEBOX	95 // rangebox
#define TAILBOX		96 // tailbox
#define TAILBOXBG	97 // tailboxbg
#define TEXTBOX		98 // textbox
#define TIMEBOX		99 // timebox
#define TREEVIEW	100 // treeview
#define YESNO		101 // yesno

void usage(void);
/* widgets */
#define BUILDER_ARGS struct config conf, char* text, int rows, int cols, \
	int argc, char **argv
//int buildlist_builder(BUILDER_ARGS);
int calendar_builder(BUILDER_ARGS);
int checklist_builder(BUILDER_ARGS);
//int dselect_builder(BUILDER_ARGS);
//int editbox_builder(BUILDER_ARGS);
int form_builder(BUILDER_ARGS);
//int fselect_builder(BUILDER_ARGS);
int gauge_builder(BUILDER_ARGS);
int infobox_builder(BUILDER_ARGS);
int inputbox_builder(BUILDER_ARGS);
int inputmenu_builder(BUILDER_ARGS);
int menu_builder(BUILDER_ARGS);
int mixedform_builder(BUILDER_ARGS);
int mixedgauge_builder(BUILDER_ARGS);
int msgbox_builder(BUILDER_ARGS);
int passwordbox_builder(BUILDER_ARGS);
int passwordform_builder(BUILDER_ARGS);
int pause_builder(BUILDER_ARGS);
int prgbox_builder(BUILDER_ARGS);
int programbox_builder(BUILDER_ARGS);
int progressbox_builder(BUILDER_ARGS);
int radiolist_builder(BUILDER_ARGS);
int rangebox_builder(BUILDER_ARGS);
//int tailbox_builder(BUILDER_ARGS);
//int tailboxbg_builder(BUILDER_ARGS);
//int textbox_builder(BUILDER_ARGS);
int timebox_builder(BUILDER_ARGS);
//int treeview_builder(BUILDER_ARGS);
int yesno_builder(BUILDER_ARGS);



void usage(void)
{

	printf("usage: bsddialog --help | -- version\n"\
	       "       bsddialog [common-opts] widget <text> <height> <width> "\
	       "[widget-opts]\n");
}

int main(int argc, char *argv[argc])
{
	char text[1024], *backtitle = NULL;
	int input, rows, cols, output;
	int (*widgetbuilder)(BUILDER_ARGS) = NULL;
	struct winsize ws;
	struct config conf;

	memset(&conf, 0, sizeof(struct config));
	conf.y = conf.x = -1;
	conf.cancel_label = "Cancel";
	conf.exit_label = "EXIT";
	conf.extra_label = "Extra";
	conf.help_label = "Help";
	conf.no_label = "No";
	conf.ok_label = "OK";
	conf.yes_label = "Yes";
	conf.shadow = true;
	conf.output_fd = STDERR_FILENO;

	/* options descriptor */
	struct option longopts[] = {
	    /* common options */
	    { "ascii-lines", no_argument, NULL, ASCII_LINES },
	    { "aspect", required_argument, NULL	/*ratio*/, 'X' },
	    { "backtitle", required_argument, NULL /*backtitle*/, BACKTITLE },
	    { "begin", required_argument, NULL /*y x*/, BEGIN },
	    { "cancel-label", required_argument, NULL /*string*/, CANCEL_LABEL },
	    { "clear", no_argument, NULL, 'X' },
	    { "colors", no_argument, NULL, 'X' },
	    { "column-separator", required_argument, NULL /*string*/, 'X' },
	    { "cr-wrap", no_argument, NULL, 'X' },
	    { "create-rc", required_argument, NULL /*file*/, 'X' },
	    { "date-format", required_argument, NULL /*format*/, DATE_FORMAT },
	    { "defaultno", no_argument, NULL, DEFAULTNO },
	    { "default-button", required_argument, NULL	/*string*/, 'X' },
	    { "default-item", required_argument, NULL /*string*/, DEFAULT_ITEM },
	    { "exit-label", required_argument, NULL /*string*/, EXIT_LABEL },
	    { "extra-button", no_argument, NULL, EXTRA_BUTTON },
	    { "extra-label", required_argument, NULL /*string*/, EXTRA_LABEL },
	    { "help", no_argument, NULL, HELP },
	    { "help-button", no_argument, NULL, HELP_BUTTON },
	    { "help-label", required_argument, NULL /*string*/, HELP_LABEL },
	    { "help-status", no_argument, NULL, HELP_STATUS },
	    { "help-tags", no_argument, NULL, 'X' },
	    { "hfile", required_argument, NULL /*filename*/, 'X' },
	    { "hline", required_argument, NULL /*string*/, HLINE },
	    { "ignore", no_argument, NULL, 'X' },
	    { "input-fd", required_argument, NULL /*fd*/, 'X' },
	    { "insecure", no_argument, NULL, 'X' },
	    { "item-help", no_argument, NULL, 'X' },
	    { "keep-tite", no_argument, NULL, 'X' },
	    { "keep-window", no_argument, NULL, 'X' },
	    { "last-key", no_argument, NULL, 'X' },
	    { "max-input", required_argument, NULL /*size*/, 'X' },
	    { "no-cancel", no_argument, NULL, NO_CANCEL },
	    { "nocancel", no_argument, NULL, NOCANCEL },
	    { "no-collapse", no_argument, NULL, 'X' },
	    { "no-items", no_argument, NULL, 'X' },
	    { "no-kill", no_argument, NULL, 'X' },
	    { "no-label", required_argument, NULL /*string*/, NO_LABEL },
	    { "no-lines", no_argument, NULL, NO_LINES },
	    { "no-mouse", no_argument, NULL, 'X' },
	    { "no-nl-expand", no_argument, NULL, 'X' },
	    { "no-ok", no_argument, NULL, NO_OK },
	    { "nook ", no_argument, NULL, NOOK },
	    { "no-shadow", no_argument, NULL, NO_SHADOW },
	    { "no-tags", no_argument, NULL, 'X' },
	    { "ok-label", required_argument, NULL /*string*/, OK_LABEL },
	    { "output-fd", required_argument, NULL /*fd*/, OUTPUT_FD },
	    { "separator", required_argument, NULL /*string*/, 'X' },
	    { "output-separator", required_argument, NULL /*string*/, 'X' },
	    { "print-maxsize", no_argument, NULL, PRINT_MAXSIZE },
	    { "print-size", no_argument, NULL, PRINT_SIZE },
	    { "print-version", no_argument, NULL, PRINT_VERSION },
	    { "quoted", no_argument, NULL, QUOTED },
	    { "scrollbar", no_argument, NULL, 'X' },
	    { "separate-output", no_argument, NULL, SEPARATE_OUTPUT },
	    { "separate-widget", required_argument, NULL /*string*/, 'X' },
	    { "shadow", no_argument, NULL, SHADOW },
	    { "single-quoted", no_argument, NULL, SINGLE_QUOTED },
	    { "size-err", no_argument, NULL, 'X' },
	    { "sleep", required_argument, NULL /*secs*/, SLEEP },
	    { "stderr", no_argument, NULL, STDERR },
	    { "stdout", no_argument, NULL, STDOUT },
	    { "tab-correct", no_argument, NULL, 'X' },
	    { "tab-len", required_argument, NULL /*n*/, 'X' },
	    { "time-format", required_argument, NULL /*format*/, TIME_FORMAT },
	    { "timeout", required_argument, NULL /*secs*/, 'X' },
	    { "title", required_argument, NULL /*title*/, TITLE },
	    { "trace", required_argument, NULL /*filename*/, 'X' },
	    { "trim", no_argument, NULL, 'X' },
	    { "version", no_argument, NULL, VERSION },
	    { "visit-items", no_argument, NULL, 'X' },
	    { "yes-label", required_argument, NULL /*string*/, YES_LABEL },
	    /* Widgets */
	    { "buildlist", no_argument, NULL, 'X' },
	    { "calendar", no_argument, NULL, CALENDAR },
	    { "checklist", no_argument, NULL, CHECKLIST },
	    { "dselect", no_argument, NULL, 'X' },
	    { "editbox", no_argument, NULL, 'X' },
	    { "form", no_argument, NULL, FORM },
	    { "fselect", no_argument, NULL, 'X' },
	    { "gauge", no_argument, NULL, GAUGE },
	    { "infobox", no_argument, NULL, INFOBOX },
	    { "inputbox", no_argument, NULL, INPUTBOX },
	    { "inputmenu", no_argument, NULL, INPUTMENU },
	    { "menu", no_argument, NULL, MENU },
	    { "mixedform", no_argument, NULL, MIXEDFORM },
	    { "mixedgauge", no_argument, NULL, MIXEDGAUGE },
	    { "msgbox", no_argument, NULL, MSGBOX },
	    { "passwordbox", no_argument, NULL, PASSWORDBOX },
	    { "passwordform", no_argument, NULL, PASSWORDFORM },
	    { "pause", no_argument, NULL, PAUSE },
	    { "prgbox", no_argument, NULL, PRGBOX },
	    { "programbox", no_argument, NULL, PROGRAMBOX },
	    { "progressbox", no_argument, NULL, PROGRESSBOX },
	    { "radiolist", no_argument, NULL, RADIOLIST },
	    { "rangebox", no_argument, NULL, RANGEBOX },
	    { "tailbox", no_argument, NULL, 'X' },
	    { "tailboxbg", no_argument, NULL, 'X' },
	    { "textbox", no_argument, NULL, 'X' },
	    { "timebox", no_argument, NULL, TIMEBOX },
	    { "treeview", no_argument, NULL, 'X' },
	    { "yesno", no_argument, NULL, YESNO },
	    /* END */
	    { NULL, 0, NULL, 0 }
	};

	while ((input = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
		switch (input) {
		/* Common options */
		case ASCII_LINES:
			conf.ascii_lines = true;
			break;
		case BACKTITLE:
			backtitle = optarg;
			break;
		case BEGIN:
			conf.y = atoi(optarg);
			conf.x = atoi(argv[optind]);
			if (conf.y < 0 || conf.x < 0) {
				printf("Can't make new window at (y:%d, x:%d).",
				    conf.y, conf.x);
				return 1;
			}
			optind++;
			break;
		case CANCEL_LABEL:
			conf.cancel_label = optarg;
			break;
		case DATE_FORMAT:
			conf.date_format = optarg;
			break;
		case DEFAULT_ITEM:
			conf.default_item = optarg;
			break;
		case DEFAULTNO:
			conf.defaultno = true;
			break;
		case EXIT_LABEL:
			conf.exit_label = optarg;
			break;
		case EXTRA_BUTTON:
			conf.extra_button = true;
			break;
		case EXTRA_LABEL:
			conf.extra_label = optarg;
			break;
		case HELP:
			usage();
			printf("\n");
			printf("See \'man 1 bsddialog\' for more information.\n");
			return 0;
		case HELP_BUTTON:
			conf.help_button = true;
			break;
		case HELP_LABEL:
			conf.help_label = optarg;
			break;
		case HELP_STATUS:
			conf.help_status = true;
			break;
		case HLINE:
			conf.hline = optarg;
			break;
		case NOCANCEL:
		case NO_CANCEL:
			conf.no_cancel = true;
			break;
		case NO_LABEL:
			conf.no_label = optarg;
			break;
		case NO_LINES:
			conf.no_lines = true;
			break;
		case NOOK:
		case NO_OK:
			conf.no_ok = true;
			break;
		case NO_SHADOW:
			conf.shadow = false;
			break;
		case OK_LABEL:
			conf.ok_label = optarg;
			break;
		case OUTPUT_FD:
			conf.output_fd = atoi(optarg);
			break;
		case QUOTED:
			conf.quoted = true;
			break;
		case PRINT_MAXSIZE:
			conf.print_maxsize = true;
			break;
		case PRINT_SIZE:
			conf.print_size = true;
			break;
		case PRINT_VERSION:
			printf("bsddialog version %s\n", BSDDIALOG_VERSION);
			break;
		case SEPARATE_OUTPUT:
			conf.separate_output = true;
			break;
		case SHADOW:
			conf.shadow = true;
			break;
		case SINGLE_QUOTED:
			conf.single_quoted = true;
			break;
		case SLEEP:
			conf.sleep = atoi(optarg);
			break;
		case STDERR:
			conf.output_fd = STDERR_FILENO;
			break;
		case STDOUT:
			conf.output_fd = STDOUT_FILENO;
			break;
		case TIME_FORMAT:
			conf.time_format = optarg;
			break;
		case TITLE:
			conf.title = optarg;
			break;
		case VERSION:
			printf("bsddialog version %s\n", BSDDIALOG_VERSION);
			return 0;
		case YES_LABEL:
			conf.yes_label = optarg;
			break;
		/* Widgets */
		case CALENDAR:
			widgetbuilder = calendar_builder;
			break;
		case CHECKLIST:
			widgetbuilder = checklist_builder;
			break;
		case FORM:
			widgetbuilder = form_builder;
			break;
		case GAUGE:
			widgetbuilder = gauge_builder;
			break;
		case INFOBOX:
			widgetbuilder = infobox_builder;
			break;
		case INPUTBOX:
			widgetbuilder = inputbox_builder;
			break;
		case INPUTMENU:
			widgetbuilder = inputmenu_builder;
			break;
		case MENU:
			widgetbuilder = menu_builder;
			break;
		case MIXEDFORM:
			widgetbuilder = mixedform_builder;
			break;
		case MIXEDGAUGE:
			widgetbuilder = mixedgauge_builder;
			break;
		case MSGBOX:
			widgetbuilder = msgbox_builder;
			break;
		case PAUSE:
			widgetbuilder = pause_builder;
			break;
		case PASSWORDBOX:
			widgetbuilder = passwordbox_builder;
			break;
		case PASSWORDFORM:
			widgetbuilder = passwordform_builder;
			break;
		case PRGBOX:
			widgetbuilder = prgbox_builder;
			break;
		case PROGRAMBOX:
			widgetbuilder = programbox_builder;
			break;
		case PROGRESSBOX:
			widgetbuilder = progressbox_builder;
			break;
		case RADIOLIST:
			widgetbuilder = radiolist_builder;
			break;
		case RANGEBOX:
			widgetbuilder = rangebox_builder;
			break;
		case TIMEBOX:
			widgetbuilder = timebox_builder;
			break;
		case YESNO:
			widgetbuilder = yesno_builder;
			break;
		/* Error */
		default:
			usage();
			return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if (conf.print_maxsize) {
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		dprintf(conf.output_fd, "Terminal size: %d, %d\n",
		    ws.ws_row, ws.ws_col);
	}

	if (argc < 3) {
		usage();
		return (1);
	}
	strcpy(text, argv[0]);
	rows = atoi(argv[1]);
	cols = atoi(argv[2]);
	argc -= 3;
	argv += 3;

	if(bsddialog_init() != 0) {
		printf("Cannot init ncurses\n");
		return 1;
	}

	if (backtitle != NULL)
		bsddialog_backtitle(conf, backtitle);

	output = -1;
	if (widgetbuilder != NULL)
		output = widgetbuilder(conf, text, rows, cols, argc, argv);

	bsddialog_end();

	// debug
	printf("[Debug] Exit status: %d\n", output);
	return output;
}

int calendar_builder(BUILDER_ARGS)
{
	int output;
	unsigned int yy, mm, dd;
	time_t cal;
	struct tm *dt;

	time(&cal);
	dt = localtime(&cal);
	yy = dt->tm_year + 1900;
	mm = dt->tm_mon;
	dd = dt->tm_mday;

	if (argc > 0) {
		yy = atoi(argv[0]) + 1900;
		yy = yy > 9999 ? 9999 : yy;
	}

	if (argc > 1) {
		mm = atoi(argv[1]);
		mm = mm > 12 ? 12 : mm;
	}

	if (argc > 2) {
		dd = atoi(argv[2]);
		dd = dd > 31 ? 31 : dd;
	}

	output = bsddialog_calendar(conf, text, rows, cols, yy, mm, dd);

	return (output);
}

int checklist_builder(BUILDER_ARGS)
{
	int output, menurows;

	if (argc < 1 || (((argc-1) % 3) != 0)) {
		usage();
		return (-1);
	}

	menurows = atoi(argv[0]);

	output = bsddialog_checklist(conf, text, rows, cols, menurows, argc-1,
	    argv + 1);

	return output;
}

int form_builder(BUILDER_ARGS)
{
	int output, formheight;

	if (argc < 1 || (((argc-1) % 8) != 0) ) {
		usage();
		return (-1);
	}

	formheight = atoi(argv[0]);

	output = bsddialog_form(conf, text, rows, cols, formheight, argc-1,
	    argv + 1);

	return output;
}

int gauge_builder(BUILDER_ARGS)
{
	int output /* always BSDDIALOG_YESOK */, perc;

	perc = argc > 0 ? atoi (argv[0]) : 0;
	perc = perc < 0 ? 0 : perc;
	perc = perc > 100 ? 100 : perc;

	output = bsddialog_gauge(conf, text, rows, cols, perc);

	return (output);
}

int infobox_builder(BUILDER_ARGS)
{
	int output; /* always BSDDIALOG_YESOK */

	output = bsddialog_infobox(conf, text, rows, cols);

	return output;
}

int inputbox_builder(BUILDER_ARGS)
{
	int output;

	output = bsddialog_inputbox(conf, text, rows, cols);

	return output;
}

int inputmenu_builder(BUILDER_ARGS)
{
	return 0;
}

int menu_builder(BUILDER_ARGS)
{
	int output, menurows;

	if (argc < 1 || (((argc-1) % 2) != 0)) {
		usage();
		return (-1);
	}

	menurows = atoi(argv[0]);

	output = bsddialog_menu(conf, text, rows, cols, menurows, argc-1,
	    argv + 1);

	return output;
}

int mixedform_builder(BUILDER_ARGS)
{
	int output, formheight;

	if (argc < 1 || (((argc-1) % 9) != 0) ) {
		usage();
		return (-1);
	}

	formheight = atoi(argv[0]);

	output = bsddialog_mixedform(conf, text, rows, cols, formheight, argc-1,
	    argv + 1);

	return output;
}

int mixedgauge_builder(BUILDER_ARGS)
{
	int output /* always BSDDIALOG_YESOK */, perc;

	if (argc < 1 || (((argc-1) % 2) != 0) ) {
		usage();
		return (-1);
	}

	perc = atoi(argv[0]);
	perc = perc < 0 ? 0 : perc;
	perc = perc > 100 ? 100 : perc;

	output = bsddialog_mixedgauge(conf, text, rows, cols, perc,
	    argc-1, argv + 1);

	return (output);
}

int msgbox_builder(BUILDER_ARGS)
{
	int output;

	output = bsddialog_msgbox(conf, text, rows, cols);

	return output;
}

int passwordbox_builder(BUILDER_ARGS)
{
	int output;

	output = bsddialog_passwordbox(conf, text, rows, cols);

	return output;
}

int passwordform_builder(BUILDER_ARGS)
{
	int output, formheight;

	if (argc < 1 || (((argc-1) % 8) != 0) ) {
		usage();
		return (-1);
	}

	formheight = atoi(argv[0]);

	output = bsddialog_passwordform(conf, text, rows, cols, formheight,
	    argc-1, argv + 1);

	return output;
}

int pause_builder(BUILDER_ARGS)
{
	int output, sec;

	if (argc < 1) {
		usage();
		return (-1);
	}

	sec = atoi(argv[0]);
	output = bsddialog_pause(conf, text, rows, cols, sec);

	return output;
}

int prgbox_builder(BUILDER_ARGS)
{
	int output;

	if (argc < 1) {
		usage();
		return (-1);
	}

	output = bsddialog_prgbox(conf, strlen(text) == 0 ? NULL : text, rows,
	    cols, argv[0]);

	return output;
}

int programbox_builder(BUILDER_ARGS)
{
	int output;

	output = bsddialog_programbox(conf, strlen(text) == 0 ? NULL : text, rows, cols);

	return output;
}

int progressbox_builder(BUILDER_ARGS)
{
	int output;

	output = bsddialog_progressbox(conf, strlen(text) == 0 ? NULL : text, rows, cols);

	return output;
}

int radiolist_builder(BUILDER_ARGS)
{
	int output, menurows;

	if (argc < 1 || (((argc-1) % 3) != 0)) {
		usage();
		return (-1);
	}

	menurows = atoi(argv[0]);

	output = bsddialog_radiolist(conf, text, rows, cols, menurows, argc-1,
	    argv + 1);

	return output;
}

int rangebox_builder(BUILDER_ARGS)
{
	int output /* always BSDDIALOG_YESOK */, min, max, def;

	if (argc != 3)
		return (-1);

	min = atoi(argv[0]);
	max = atoi(argv[1]);
	def = atoi(argv[2]);
	def = def < min ? min : def;
	def = def > max ? max : def;

	output = bsddialog_rangebox(conf, text, rows, cols, min, max, def);

	return (output);
}

int timebox_builder(BUILDER_ARGS)
{
	int output;
	unsigned int hh, mm, ss;
	time_t clock;
	struct tm *localtm;

	time(&clock);
	localtm = localtime(&clock);
	hh = localtm->tm_hour;
	mm = localtm->tm_min;
	ss = localtm->tm_sec;

	if (argc > 0) {
		hh = atoi(argv[0]);
		hh = hh > 23 ? 23 : hh;
	}

	if (argc > 1) {
		mm = atoi(argv[1]);
		mm = mm > 60 ? 60 : mm;
	}

	if (argc > 2) {
		ss = atoi(argv[2]);
		ss = ss > 60 ? 60 : ss;
	}

	output = bsddialog_timebox(conf, text, rows, cols, hh, mm, ss);

	return (output);
}

int yesno_builder(BUILDER_ARGS)
{
	int output;

	output = bsddialog_yesno(conf, text, rows, cols);

	return output;
}
