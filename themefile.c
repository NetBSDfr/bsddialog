/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Alfonso Sabato Siciliano
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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <bsddialog.h>
#include <bsddialog_theme.h>

static struct bsddialog_theme t;

enum typeprop {
	BOOL,
	CHAR,
	INT,
	UINT,
	COLOR
};

struct property {
	const char* name;
	enum typeprop type;
	void *value;
};

#define NPROPERTY 35
static struct property p[NPROPERTY] = {
	{ "theme.screen.color", COLOR, &t.screen.color },

	{ "theme.shadow.color", COLOR, &t.shadow.color },
	{ "theme.shadow.h", UINT, &t.shadow.h },
	{ "theme.shadow.w", UINT, &t.shadow.w },

	{ "theme.dialog.color", COLOR, &t.dialog.color },
	{ "theme.dialog.delimtitle", BOOL, &t.dialog.delimtitle },
	{ "theme.dialog.titlecolor", COLOR, &t.dialog.titlecolor },
	{ "theme.dialog.lineraisecolor", COLOR, &t.dialog.lineraisecolor },
	{ "theme.dialog.linelowercolor", COLOR, &t.dialog.linelowercolor },
	{ "theme.dialog.bottomtitlecolor", COLOR, &t.dialog.bottomtitlecolor },
	{ "theme.dialog.arrowcolor", COLOR, &t.dialog.arrowcolor },

	{ "theme.menu.f_selectorcolor", COLOR, &t.menu.f_selectorcolor},
	{ "theme.menu.selectorcolor", COLOR, &t.menu.selectorcolor},
	{ "theme.menu.f_namecolor", COLOR, &t.menu.f_namecolor},
	{ "theme.menu.namecolor", COLOR, &t.menu.namecolor},
	{ "theme.menu.f_desccolor", COLOR, &t.menu.f_desccolor},
	{ "theme.menu.desccolor", COLOR, &t.menu.desccolor},
	{ "theme.menu.namesepcolor", COLOR, &t.menu.namesepcolor},
	{ "theme.menu.descsepcolor", COLOR, &t.menu.descsepcolor},
	{ "theme.menu.f_shortcutcolor", COLOR, &t.menu.f_shortcutcolor},
	{ "theme.menu.shortcutcolor", COLOR, &t.menu.shortcutcolor},

	{ "theme.form.f_fieldcolor", COLOR, &t.form.f_fieldcolor},
	{ "theme.form.fieldcolor", COLOR, &t.form.fieldcolor},
	{ "theme.form.readonlycolor", COLOR, &t.form.readonlycolor},
	
	{ "theme.bar.f_color", COLOR, &t.bar.f_color},
	{ "theme.bar.color", COLOR, &t.bar.color},
	
	{ "theme.button.hmargin", UINT, &t.button.hmargin},
	{ "theme.button.leftdelim", CHAR, &t.button.leftdelim},
	{ "theme.button.rightdelim", CHAR, &t.button.rightdelim},
	{ "theme.button.delimcolor", COLOR, &t.button.delimcolor},
	{ "theme.button.f_delimcolor", COLOR, &t.button.f_delimcolor},
	{ "theme.button.color", COLOR, &t.button.color},
	{ "theme.button.f_color", COLOR, &t.button.f_color},
	{ "theme.button.shortcutcolor", COLOR, &t.button.shortcutcolor},
	{ "theme.button.f_shortcutcolor", COLOR, &t.button.f_shortcutcolor}
};

static char *color[8] = {
	"black",
	"red",
	"green",
	"yellow",
	"blue",
	"magenta",
	"cyan",
	"white"
};

int savetheme(const char *file, char *errbuf, const char *version)
{
	int i;
	unsigned int flags;
	enum bsddialog_color bg, fg;
	time_t clock;
	FILE *fp;

	if (bsddialog_get_theme(&t) != BSDDIALOG_OK) {
		sprintf(errbuf, "Cannot save theme: %s\n", bsddialog_geterror());
		return (BSDDIALOG_ERROR);
	}

	if(time(&clock) < 0) {
		sprintf(errbuf, "Cannot save profile (getting current time)\n");
		return (BSDDIALOG_ERROR);
	}

	if ((fp = fopen(file, "w")) == NULL) {
		sprintf(errbuf, "Cannot open %s to save profile\n", file);
		return (BSDDIALOG_ERROR);
	}

	fprintf(fp, "### bsddialog theme - %s", ctime(&clock));
	fputs("# Refer to bsddialog(3) manual to know theme.* properties\n", fp);
	fprintf(fp, "version %s\n", version);

	for (i = 0; i < NPROPERTY; i++) {
		switch (p[i].type) {
		case CHAR:
			fprintf(fp, "%s %c\n", p[i].name, *((char*)p[i].value));
			break;
		case INT:
			fprintf(fp, "%s %d\n", p[i].name, *((int*)p[i].value));
			break;
		case UINT:
			fprintf(fp, "%s %u\n", p[i].name,
			    *((unsigned int*)p[i].value));
			break;
		case BOOL:
			fprintf(fp, "%s %s\n", p[i].name,
			    *((bool*)p[i].value) ? "true" : "false");
			break;
		case COLOR:
			bsddialog_color_attrs(*(int*)p[i].value, &fg, &bg,
			    &flags);
			fprintf(fp, "%s %s %s%s%s%s\n",
			    p[i].name, color[fg], color[bg],
			    flags & BSDDIALOG_BOLD ? " bold" : "",
			    flags & BSDDIALOG_REVERSE ? " reverse" : "",
			    flags & BSDDIALOG_UNDERLINE ? " underline" : "");
			break;
		}
	}

	fclose(fp);

	return (BSDDIALOG_OK);
}

#define RETURN_ERROR(name, error) do {                                         \
	sprintf(errbuf, "%s for \"%s\"", error, name);                         \
	fclose(fp);                                                            \
	return (BSDDIALOG_ERROR);                                              \
} while (0)


int loadtheme(const char *file, char *errbuf)
{
	bool boolvalue;
	char charvalue, *value;
	char line[BUFSIZ], name[BUFSIZ], c1[BUFSIZ], c2[BUFSIZ];
	int i, j, intvalue, flags;
	unsigned int uintvalue;
	enum bsddialog_color bg, fg;
	FILE *fp;

	if (bsddialog_get_theme(&t) != BSDDIALOG_OK) {
		sprintf(errbuf, "Cannot save theme: %s\n", bsddialog_geterror());
		return (BSDDIALOG_ERROR);
	}

	if((fp = fopen(file, "r")) == NULL) {
		sprintf(errbuf, "Cannot open %s theme\n", file);
		return (BSDDIALOG_ERROR);
	}

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
				continue;
			RETURN_ERROR(name, "Unknown theme property name");
		}
		switch (p[i].type) {
		case CHAR:
			if (sscanf(value, "%c", &charvalue) != 1)
				RETURN_ERROR(p[i].name, "Cannot get a char");
			*((int*)p[i].value) = charvalue;
			break;
		case INT:
			if (sscanf(value, "%d", &intvalue) != 1)
				RETURN_ERROR(p[i].name, "Cannot get a int");
			*((int*)p[i].value) = intvalue;
			break;
		case UINT:
			if (sscanf(value, "%u", &uintvalue) != 1)
				RETURN_ERROR(p[i].name, "Cannot get a uint");
			*((unsigned int*)p[i].value) = uintvalue;
			break;
		case BOOL:
			boolvalue = (strstr(value, "true") != NULL) ? true :false;
			*((bool*)p[i].value) = boolvalue;
			break;
		case COLOR:
			if (sscanf(value, "%s %s", c1, c2) != 2)
				RETURN_ERROR(p[i].name, "Cannot get 2 colors");
			/* Foreground */
			for (j = 0; j < 8 ; j++)
				if ((strstr(c1, color[j])) != NULL)
					break;
			if ((fg = j) > 7)
				RETURN_ERROR(p[i].name, "Bad foreground");
			/* Background */
			for (j = 0; j < 8 ; j++)
				if ((value = strstr(c2, color[j])) != NULL)
					break;
			if ((bg = j) > 7)
				RETURN_ERROR(p[i].name, "Bad background");
			/* Flags */
			flags = 0;
			if (strstr(value, "bold") != NULL)
				flags |= BSDDIALOG_BOLD;
			if (strstr(value, "reverse") != NULL)
				flags |= BSDDIALOG_REVERSE;
			if (strstr(value, "underline") != NULL)
				flags |= BSDDIALOG_UNDERLINE;
			*((int*)p[i].value) = bsddialog_color(fg, bg, flags);
			break;
		}
	}

	fclose(fp);

	bsddialog_set_theme(&t);

	return 0;
}
