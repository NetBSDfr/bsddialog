# BSDDialog Library

### PART I - Introduction, Tutorial and Examples

## 1 Introduction

The bsddialog library provides and API to build utilities with a Text User Interface.

### 1.1 Getting started


FreeBSD:

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make
% ./bsddialog --msgbox "Hello World!" 8 20
```

If you are using XFCE install devel/ncurses

```
% sudo pkg install ncurses
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make -DPORTNCURSES
% ./bsddialog --msgbox "Hello World!" 8 20
```

Linux:

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make -GNUMakefile
% ./bsddialog --msgbox "Hello World!" 8 20
```

### 1.2 Hello World

helloworld.c

```c
#include <stdio.h>
#include <string.h>

#include <bsddialog.h>

int main()
{
	struct bsddialog_conf conf;

	memset(&conf, 0, sizeof(struct bsddialog_conf));
	conf.y = conf.x = BSDDIALOG_CENTER;
	conf.shadow = true;
	
	bsddialog_init();
	bsddialog_msgbox(conf, "Hello World!", 8, 20);
	bsddialog_end();

	return 0;
}
```

**Compiling and Running**:

```
% cc -I/usr/local/include helloworld.c -o helloworld -L/usr/local/lib/ -lbsddialog
% ./helloworld
```

**Output**:

![hellowworld](../screenshot.png)

# 2 API

BSDDialog provides functions to build graphical widget in a terminal.

## 2 Global tools

```c
#include <bsddialog.h>
```

```c
#define BSDDIALOG_YESOK		 0 // YES or OK buttons
#define BSDDIALOG_NOCANCEL	 1 // No or Cancel buttons
#define BSDDIALOG_HELP		 2 // Help button
#define BSDDIALOG_EXTRA		 3 // Extra button
#define BSDDIALOG_ITEM_HELP	 4
#define BSDDIALOG_ERROR		-1 // generic error or ESC key
```

### 2.1 Starting

```c
int  bsddialog_init(void);
void bsddialog_end(void);
int  bsddialog_backtitle(struct bsddialog_conf conf, char *backtitle);
const char *bsddialog_geterror(void);
```

### 2.2 Ending

```c
```

### 2.3 bsddialog\_conf

```c
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
```

Common members:

# PART II - Widgets Reference

# Widgets

```c
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
```

## 3 "Info"

infobox

## 4 Button

msgbox
yesno

## 5 Menu

```c
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
```

buildlist
checklist
menu
radiolist
treeview

## 6 Form

inputbox
passwordbox 
form
passwordform
mixedform

## 7 "Editor"

editbox(todo)

## 8 "Bar"

gauge
mixedgauge
rangebox
pause

## 9 "Time"

timebox
calendar

## 10 "Command" 

prgbox
programbox(todo)
progressbox(todo)

## 11 "Text"

tailbox
tailboxbg(todo)
textbox

## 12 "File"

dselect(todo)
fselect(todo)

## 13 Style

```c
#include <bsddialog_theme.h>
```

```c
struct bsddialog_theme {
	int shadowcolor;
	int backgroundcolor;
	bool surroundtitle;
	int titlecolor;
	int lineraisecolor;
	int linelowercolor;
	int widgetcolor;

	int curritemcolor;
	int itemcolor;
	int currtagcolor;
	int tagcolor;
	int namesepcolor;
	int descsepcolor;

	int currfieldcolor;
	int fieldcolor;
	int fieldreadonlycolor;

	int currbarcolor;
	int barcolor;

	unsigned int buttonspace;
	int buttleftch;
	int buttrightchar;
	int currbuttdelimcolor;
	int buttdelimcolor;
	int currbuttoncolor;
	int buttoncolor;
	int currshortkeycolor;
	int shortkeycolor;

	int bottomtitlecolor;
};

enum bsddialog_default_theme {
	BSDDIALOG_THEME_DEFAULT,
	BSDDIALOG_THEME_DIALOG,
	BSDDIALOG_THEME_MAGENTA,
};

//struct bsddialog_theme bsddialog_gettheme();
//int bsddialog_setcustomtheme(struct bsddialog_theme *theme);
int bsddialog_settheme(enum bsddialog_default_theme theme);
```
