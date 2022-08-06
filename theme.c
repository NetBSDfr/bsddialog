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
	{ "theme.button.leftdelim", INT, &t.button.leftdelim},
	{ "theme.button.rightdelim", INT, &t.button.rightdelim},
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

int savetheme(const char *file, const char *version)
{
	int i;
	FILE *fp;
	time_t clock;
	unsigned int flags;
	enum bsddialog_color bg, fg;

	if (bsddialog_get_theme(&t) != BSDDIALOG_OK) {
		printf("Error saving theme: %s\n", bsddialog_geterror());
		return (BSDDIALOG_ERROR);
	}

	if(time(&clock) < 0) {
		printf("Cannot save the profile (gettin current time)\n");
		return (BSDDIALOG_ERROR);
	}

	if ((fp = fopen(file, "w")) == NULL) {
		printf("Cannot save the profile (open %s)\n", file);
		return (BSDDIALOG_ERROR);
	}

	fprintf(fp, "### bsddialog theme - %s", ctime(&clock));
	fputs("# To see bsddialog(3) manual to know theme.* properties\n", fp);
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

int loadtheme(const char *file)
{
	bool boolvalue;
	char charvalue;
	int i, j, intvalue, flags;
	unsigned int uintvalue;
	FILE *fp;
	char line[BUFSIZ], name[BUFSIZ], c1[BUFSIZ], c2[BUFSIZ];
	char *value;
	enum bsddialog_color bg, fg;

	if (bsddialog_get_theme(&t) != BSDDIALOG_OK) {
		printf("Error saving theme: %s\n", bsddialog_geterror());
		return (BSDDIALOG_ERROR);
	}

	if((fp = fopen(file, "r")) == NULL) {
		printf("Cannot open %s theme\n", file);
		return 1;
	}

	while(fgets(line, BUFSIZ, fp) != NULL) {
		if(line[0] == '#' || line[0] == '\n')
			continue;  /* superfluous, just for efficiency */
		sscanf(line, "%s", name);
		for (i = 0; i < NPROPERTY; i++) {
			if (strcmp(name, p[i].name) == 0) {
				value = &line[strlen(name)];
				break;
			}
		}
		if (i >= NPROPERTY)
			continue;
		switch (p[i].type) {
		// XXX add checks
		case CHAR:
			sscanf(value, "%c", &charvalue);
			*((int*)p[i].value) = charvalue;
			break;
		case INT:
			sscanf(value, "%d", &intvalue);
			*((int*)p[i].value) = intvalue;
			break;
		case UINT:
			sscanf(value, "%u", &uintvalue);
			*((unsigned int*)p[i].value) = uintvalue;
			break;
		case BOOL:
			boolvalue = (strstr(value, "true") != NULL) ? true :false;
			*((bool*)p[i].value) = boolvalue;
			break;
		case COLOR:
			sscanf(value, "%s %s", c1, c2);
			for (j = 0; j < 8 ; j++)
				if ((strstr(c1, color[j])) != NULL)
					break; // XXX check
			fg = j;
			for (j = 0; j < 8 ; j++)
				if ((value = strstr(c2, color[j])) != NULL)
					break; // XXX check
			bg = j;
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
