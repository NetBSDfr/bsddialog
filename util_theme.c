/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022-2023 Alfonso Sabato Siciliano
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

#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <bsddialog.h>
#include <bsddialog_theme.h>

#include "util.h"

static struct bsddialog_theme t;
static char title[1024];

#define NPROPERTY  38
#define NCOLOR      8
#define NATTR       6

#define PROP_ERROR(name, error) do {                                           \
	fclose(fp);                                                            \
	exit_error(false, "%s for \"%s\"", error, name);                       \
} while (0)

enum typeproperty {
	BOOL,
	CHAR,
	INT,
	UINT,
	COLOR
};

struct property {
	const char *comment;
	const char *name;
	enum typeproperty type;
	void *value;
};

struct namevalue {
	const char *name;
	unsigned int value;
};

static struct namevalue color[NCOLOR] = {
	{"black",   BSDDIALOG_BLACK},
	{"red",     BSDDIALOG_RED},
	{"green",   BSDDIALOG_GREEN},
	{"yellow",  BSDDIALOG_YELLOW},
	{"blue",    BSDDIALOG_BLUE},
	{"magenta", BSDDIALOG_MAGENTA},
	{"cyan",    BSDDIALOG_CYAN},
	{"white",   BSDDIALOG_WHITE}
};

static struct namevalue attr[NATTR] = {
 	{"bold",       BSDDIALOG_BOLD},
	{"reverse",    BSDDIALOG_REVERSE},
	{"underline",  BSDDIALOG_UNDERLINE},
	{"blink",      BSDDIALOG_BLINK},
	{"halfbright", BSDDIALOG_HALFBRIGHT},
	{"highlight",  BSDDIALOG_HIGHLIGHT}
};

static struct property p[NPROPERTY] = {
	{"\n#Terminal\n", "theme.screen.color", COLOR, &t.screen.color},

	{"\n# Shadow\n",
	    "theme.shadow.color", COLOR, &t.shadow.color},
	{"# shift down right from main widget\n",
	    "theme.shadow.y", UINT, &t.shadow.y},
	{"", "theme.shadow.x", UINT, &t.shadow.x},

	{"\n# Main widget\n",
	    "theme.dialog.color", COLOR, &t.dialog.color},
	{"", "theme.dialog.delimtitle", BOOL, &t.dialog.delimtitle},
	{"", "theme.dialog.titlecolor", COLOR, &t.dialog.titlecolor},
	{"", "theme.dialog.lineraisecolor", COLOR, &t.dialog.lineraisecolor},
	{"", "theme.dialog.linelowercolor", COLOR, &t.dialog.linelowercolor},
	{"", "theme.dialog.bottomtitlecolor", COLOR,
	    &t.dialog.bottomtitlecolor},
	{"", "theme.dialog.arrowcolor", COLOR, &t.dialog.arrowcolor},

	{"\n# --checklist, --menu, --radiolist, --treeview\n",
	    "theme.menu.f_selectorcolor", COLOR, &t.menu.f_selectorcolor},
	{"", "theme.menu.selectorcolor", COLOR, &t.menu.selectorcolor},
	{"", "theme.menu.f_namecolor", COLOR, &t.menu.f_namecolor},
	{"", "theme.menu.namecolor", COLOR, &t.menu.namecolor},
	{"", "theme.menu.f_desccolor", COLOR, &t.menu.f_desccolor},
	{"", "theme.menu.desccolor", COLOR, &t.menu.desccolor},
	{"", "theme.menu.namesepcolor", COLOR, &t.menu.namesepcolor},
	{"", "theme.menu.descsepcolor", COLOR, &t.menu.descsepcolor},
	{"", "theme.menu.f_shortcutcolor", COLOR, &t.menu.f_shortcutcolor},
	{"", "theme.menu.shortcutcolor", COLOR, &t.menu.shortcutcolor},
	{"", "theme.menu.bottomdesccolor", COLOR, &t.menu.bottomdesccolor},

	{"\n# Forms\n",
	    "theme.form.f_fieldcolor", COLOR, &t.form.f_fieldcolor},
	{"", "theme.form.fieldcolor", COLOR, &t.form.fieldcolor},
	{"", "theme.form.readonlycolor", COLOR, &t.form.readonlycolor},
	{"", "theme.form.bottomdesccolor", COLOR, &t.form.bottomdesccolor},

	{"\n# Bar of --gauge, --mixedgauge, --pause, --rangebox\n",
	    "theme.bar.f_color", COLOR, &t.bar.f_color},
	{"", "theme.bar.color", COLOR, &t.bar.color},

	{"\n# Buttons\n",
	    "theme.button.minmargin", UINT, &t.button.minmargin},
	{"", "theme.button.maxmargin", UINT, &t.button.maxmargin},
	{"", "theme.button.leftdelim", CHAR, &t.button.leftdelim},
	{"", "theme.button.rightdelim", CHAR, &t.button.rightdelim},
	{"", "theme.button.delimcolor", COLOR, &t.button.delimcolor},
	{"", "theme.button.f_delimcolor", COLOR, &t.button.f_delimcolor},
	{"", "theme.button.color", COLOR, &t.button.color},
	{"", "theme.button.f_color", COLOR, &t.button.f_color},
	{"", "theme.button.shortcutcolor", COLOR, &t.button.shortcutcolor},
	{"", "theme.button.f_shortcutcolor", COLOR, &t.button.f_shortcutcolor}
};

void savetheme(const char *file, const char *version)
{
	int i, j;
	unsigned int flags;
	enum bsddialog_color bg, fg;
	time_t clock;
	FILE *fp;

	if (bsddialog_get_theme(&t) != BSDDIALOG_OK)
		exit_error(false,
		    "cannot save theme: %s", bsddialog_geterror());

	if(time(&clock) < 0)
		exit_error(false, "cannot save profile getting current time");

	if ((fp = fopen(file, "w")) == NULL)
		exit_error(false, "cannot open %s to save profile", file);

	fprintf(fp, "### bsddialog theme - %s\n", ctime(&clock));
	fputs("# Colors:", fp);
	for (i = 0; i < NCOLOR; i++)
		fprintf(fp, " %s", color[i].name);
	fputs(".\n# Attributes: ", fp);
	for (i = 0; i < NATTR; i++)
		fprintf(fp, " %s", attr[i].name);
	fputs(".\n# f_* refers to elements with focus.\n\n", fp);
	fprintf(fp, "version %s\n", version);

	for (i = 0; i < NPROPERTY; i++) {
		fprintf(fp, "%s%s", p[i].comment, p[i].name);
		switch (p[i].type) {
		case CHAR:
			fprintf(fp, " %c\n", *((char*)p[i].value));
			break;
		case INT:
			fprintf(fp, " %d\n", *((int*)p[i].value));
			break;
		case UINT:
			fprintf(fp, " %u\n", *((unsigned int*)p[i].value));
			break;
		case BOOL:
			fprintf(fp, " %s\n",
			    *((bool*)p[i].value) ? "true" : "false");
			break;
		case COLOR:
			bsddialog_color_attrs(*(int*)p[i].value, &fg, &bg,
			    &flags);
			fprintf(fp, " %s %s", color[fg].name, color[bg].name);
			for (j = 0; j < NATTR; j++)
				if (flags & attr[j].value)
					fprintf(fp, " %s", attr[j].name);
			fputs("\n", fp);
			break;
		}
	}

	fclose(fp);
}

void setdeftheme(enum bsddialog_default_theme theme)
{
	if (bsddialog_set_default_theme(theme) != BSDDIALOG_OK)
		exit_error(false, bsddialog_geterror());
}

void loadtheme(const char *file)
{
	bool boolvalue;
	char charvalue, *value;
	char line[BUFSIZ], name[BUFSIZ], c1[BUFSIZ], c2[BUFSIZ];
	int i, j, intvalue;
	unsigned int uintvalue, flags;
	enum bsddialog_color bg, fg;
	FILE *fp;

	if (bsddialog_get_theme(&t) != BSDDIALOG_OK)
		exit_error(false, "Cannot get current theme: %s",
		    bsddialog_geterror());

	if((fp = fopen(file, "r")) == NULL)
		exit_error(false, "Cannot open theme \"%s\" file", file);

	while(fgets(line, BUFSIZ, fp) != NULL) {
		if(line[0] == '#' || line[0] == '\n')
			continue;  /* superfluous, only for efficiency */
		sscanf(line, "%s", name);
		for (i = 0; i < NPROPERTY; i++) {
			if (strcmp(name, p[i].name) == 0) {
				value = &line[strlen(name)];
				break;
			}
		}
		if (i >= NPROPERTY) {
			if (strcmp(name, "version") == 0)
				continue; /* nothing for now */
			PROP_ERROR(name, "Unknown theme property name");
		}
		switch (p[i].type) {
		case CHAR:
			while (value[0] == ' ' || value[0] == '\n' ||
			    value[0] == '\0')
				value++;
			if (sscanf(value, "%c", &charvalue) != 1)
				PROP_ERROR(p[i].name, "Cannot get a char");
			*((int*)p[i].value) = charvalue;
			break;
		case INT:
			if (sscanf(value, "%d", &intvalue) != 1)
				PROP_ERROR(p[i].name, "Cannot get a int");
			*((int*)p[i].value) = intvalue;
			break;
		case UINT:
			if (sscanf(value, "%u", &uintvalue) != 1)
				PROP_ERROR(p[i].name, "Cannot get a uint");
			*((unsigned int*)p[i].value) = uintvalue;
			break;
		case BOOL:
			boolvalue = (strstr(value, "true") != NULL) ?
			    true :false;
			*((bool*)p[i].value) = boolvalue;
			break;
		case COLOR:
			if (sscanf(value, "%s %s", c1, c2) != 2)
				PROP_ERROR(p[i].name, "Cannot get 2 colors");
			/* Foreground */
			for (j = 0; j < NCOLOR ; j++)
				if ((strstr(c1, color[j].name)) != NULL)
					break;
			if (j >= NCOLOR)
				PROP_ERROR(p[i].name, "Bad foreground");
			fg = color[j].value;
			/* Background */
			for (j = 0; j < NCOLOR ; j++)
				if ((strstr(c2, color[j].name)) != NULL)
					break;
			if (j >= NCOLOR)
				PROP_ERROR(p[i].name, "Bad background");
			bg = color[j].value;
			/* Flags */
			flags = 0;
			for (j = 0; j < NATTR; j++)
				if (strstr(value, attr[j].name) != NULL)
					flags |= attr[j].value;
			*((int*)p[i].value) = bsddialog_color(fg, bg, flags);
			break;
		}
	}

	fclose(fp);

	if(bsddialog_set_theme(&t) != BSDDIALOG_OK)
		exit_error(false, bsddialog_geterror());
}

void bikeshed(struct bsddialog_conf *conf)
{
	int margin, i;
	int colors[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	char delim[8] = {'[', '<', '(', '|', ']', '>', ')', '|'};
	enum bsddialog_color col[6];
	struct timeval tv;

	/* theme */
	if (bsddialog_get_theme(&t) != BSDDIALOG_OK)
		exit_error(false, bsddialog_geterror());

	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	for (i = 0; i < 6; i++) {
		do {
			col[i] = rand() % 8;
		} while (colors[col[i]] == 1);
		colors[col[i]] = 1;
	}

	t.screen.color = bsddialog_color(col[4], col[3], 0);

	t.shadow.color   = bsddialog_color(col[0], col[0], 0);
	t.shadow.y       = 1,
	t.shadow.x       = 2,

	t.dialog.delimtitle       = (~rand() & 1) ? true : false;
	t.dialog.titlecolor       = bsddialog_color(col[3], col[5], 0);
	t.dialog.lineraisecolor   = bsddialog_color(col[0], col[5], 0);
	t.dialog.linelowercolor   = bsddialog_color(col[0], col[5], 0);
	t.dialog.color            = bsddialog_color(col[0], col[5], 0);
	t.dialog.bottomtitlecolor = bsddialog_color(col[0], col[5], 0);
	t.dialog.arrowcolor       = bsddialog_color(col[3], col[5], 0);

	t.menu.f_selectorcolor = bsddialog_color(col[5], col[3], 0);
	t.menu.selectorcolor   = bsddialog_color(col[0], col[5], 0);
	t.menu.f_desccolor     = bsddialog_color(col[5], col[3], 0);
	t.menu.desccolor       = bsddialog_color(col[0], col[5], 0);
	t.menu.f_namecolor     = bsddialog_color(col[5], col[3], 0);
	t.menu.namecolor       = bsddialog_color(col[3], col[5], 0);
	t.menu.namesepcolor    = bsddialog_color(col[1], col[5], 0);
	t.menu.descsepcolor    = bsddialog_color(col[1], col[5], 0);
	t.menu.f_shortcutcolor = bsddialog_color(col[1], col[3], 0);
	t.menu.shortcutcolor   = bsddialog_color(col[1], col[5], 0);
	t.menu.bottomdesccolor = bsddialog_color(col[4], col[3], 0);

	t.form.f_fieldcolor    = bsddialog_color(col[5], col[3], 0);
	t.form.fieldcolor      = bsddialog_color(col[5], col[4], 0);
	t.form.readonlycolor   = bsddialog_color(col[4], col[5], 0);
	t.form.bottomdesccolor = bsddialog_color(col[4], col[3], 0);

	t.bar.f_color = bsddialog_color(col[5], col[3], 0);
	t.bar.color   = bsddialog_color(col[3], col[5], 0);

	t.button.minmargin       = 1,
	t.button.maxmargin       = 5,
	i = rand() % 4;
	t.button.leftdelim       = delim[i];
	t.button.rightdelim      = delim[i + 4];
	t.button.f_delimcolor    = bsddialog_color(col[5], col[3], 0);
	t.button.delimcolor      = bsddialog_color(col[0], col[5], 0);
	t.button.f_color         = bsddialog_color(col[2], col[3], 0);
	t.button.color           = bsddialog_color(col[0], col[5], 0);
	t.button.f_shortcutcolor = bsddialog_color(col[5], col[3], 0);
	t.button.shortcutcolor   = bsddialog_color(col[1], col[5], 0);

	if (bsddialog_set_theme(&t))
		exit_error(false, bsddialog_geterror());

	/* conf */
	conf->button.always_active = (~rand() & 1) ? true : false;
	if (conf->title != NULL) {
		memset(title, 0, 1024);
		margin = rand() % 5;
		memset(title, ' ', margin);
		strcpy(title + margin, conf->title);
		memset(title + strlen(title), ' ', margin);
		conf->title = title;
	}
}
