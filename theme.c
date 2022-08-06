#include <stdio.h>
#include <time.h>

#include <bsddialog.h>
#include <bsddialog_theme.h>

static struct bsddialog_theme t;

enum typeprop {
	STRING,
	BOOL,
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
	{ "theme.button.leftdelim", COLOR, &t.button.leftdelim},
	{ "theme.button.rightdelim", COLOR, &t.button.rightdelim},
	{ "theme.button.delimcolor", COLOR, &t.button.delimcolor},
	{ "theme.button.f_delimcolor", COLOR, &t.button.f_delimcolor},
	{ "theme.button.color", COLOR, &t.button.color},
	{ "theme.button.f_color", COLOR, &t.button.f_color},
	{ "theme.button.shortcutcolor", COLOR, &t.button.shortcutcolor},
	{ "theme.button.f_shortcutcolor", COLOR, &t.button.f_shortcutcolor}
};

int savetheme(const char *file, const char *version)
{
	int i;
	FILE *fp;
	time_t clock;
	unsigned int flags;
	enum bsddialog_color bg, fg;
	char *color[8] = {
		"black",
		"red",
		"green",
		"yellow",
		"blue",
		"magenta",
		"cyan",
		"white"
	};

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
	fprintf(fp, "version %s\n", version);

		for (i = 0; i < NPROPERTY; i++) {
		switch (p[i].type) {
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
		case STRING:
			fprintf(fp, "%s %s\n", p[i].name, (char*)p[i].value);
			break;
		case COLOR:
			bsddialog_color_attrs(*(int*)p[i].value, &bg, &fg,
			    &flags);
			fprintf(fp, "%s %s %s%s%s%s\n",
			    p[i].name, color[bg], color[fg],
			    flags & BSDDIALOG_BOLD ? " bold" : "",
			    flags & BSDDIALOG_REVERSE ? " reverse" : "",
			    flags & BSDDIALOG_UNDERLINE ? " underline" : "");
			break;
		}
	}

	fclose(fp);

	return (BSDDIALOG_OK);
}
