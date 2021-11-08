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
% cc -I/usr/local/include helloworld.c -o helloworld -L/usr/local/lib -lbsddialog
% ./helloworld
```

**Output**:

![hellowworld](../screenshot.png)

## 2 API

### 2.1 Starting

```c
#include <bsddialog.h>

int bsddialog_init(void);
int bsddialog_end(void);
int bsddialog_backtitle(struct bsddialog_conf conf, char *backtitle);
int terminalheight(void);
int terminalwidth(void);
```

### 2.2 Exit Status

```c
#define BSDDIALOG_ERROR		-1 // generic error (or ESC dialog)
#define BSDDIALOG_YESOK		 0 // YES or OK buttons
#define BSDDIALOG_NOCANCEL	 1 // No or Cancel buttons
#define BSDDIALOG_HELP		 2 // Help button
#define BSDDIALOG_EXTRA		 3 // Extra button
#define BSDDIALOG_ITEM_HELP	 4
#define BSDDIALOG_ESC		 5 // ESC key
```

### 2.3 Errors

```c
const char *bsddialog_geterror(void);
```

### 2.4 Configuration

```c
struct bsddialog_conf {
	bool item_prefix; //BSDDialog extension
	bool ascii_lines;
	int aspect;	// aspect ratio
	//char *backtitle;
	int x;		// BEGIN
	int y;		// BEGIN
	char *cancel_label;
	bool clear;
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

### 2.5 Themes

```c
#include <bsddialog_theme.h>
```

```c
enum bsddialog_color {
	BSDDIALOG_BLACK = 0,
	BSDDIALOG_RED,
	BSDDIALOG_GREEN,
	BSDDIALOG_YELLOW,
	BSDDIALOG_BLUE,
	BSDDIALOG_MAGENTA,
	BSDDIALOG_CYAN,
	BSDDIALOG_WHITE,
};

struct bsddialog_theme {
	int shadowcolor;
	unsigned int shadowrows;
	unsigned int shadowcols;

	int backgroundcolor;
	bool surroundtitle;
	int titlecolor;
	int lineraisecolor;
	int linelowercolor;
	int widgetcolor;

	unsigned int texthmargin;

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

int bsddialog_color(enum bsddialog_color background, enum bsddialog_color foreground);
struct bsddialog_theme bsddialog_get_theme();
void bsddialog_set_theme(struct bsddialog_theme theme);
int bsddialog_set_default_theme(enum bsddialog_default_theme theme);
```

-------------------------------------------------------

### PART II - Widgets Reference

## 3 Info

### 3.1 infobox

**API**

```c
int bsddialog_infobox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include infobox.c -o infobox -L/usr/local/lib -lbsddialog
% ./infobox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 4 Message

### 4.1 msgbox

**API**

```c
int bsddialog_msgbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include msgbox.c -o msgbox -L/usr/local/lib -lbsddialog
% ./msgbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 4.2 yesno

**API**

```c
int bsddialog_yesno(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include yesno.c -o yesno -L/usr/local/lib -lbsddialog
% ./yesno
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

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

### 5.1 buildlist

**API**

```c
int bsddialog_buildlist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include buildlist.c -o buildlist -L/usr/local/lib -lbsddialog
% ./buildlist
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 5.2 checklist

**API**

```c
int bsddialog_checklist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include checklist.c -o checklist -L/usr/local/lib -lbsddialog
% ./checklist
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 5.3 menu

**API**

```c
int bsddialog_menu(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include menu.c -o menu -L/usr/local/lib -lbsddialog
% ./menu
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 5.4 mixedlist

**API**

```c
int bsddialog_mixedlist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int ngroups, struct bsddialog_menugroup *groups);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include mixedlist.c -o mixedlist -L/usr/local/lib -lbsddialog
% ./mixedlist
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 5.5 radiolist

**API**

```c
int bsddialog_radiolist(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include radiolist.c -o radiolist -L/usr/local/lib -lbsddialog
% ./ex
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 5.6 treeview

**API**

```c
int bsddialog_treeview(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int menurows, int nitems, struct bsddialog_menuitem *items);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include treeview.c -o treeview -L/usr/local/lib -lbsddialog
% ./treeview
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 6 Form

### 6.1 inputbox

**API**

```c
int bsddialog_inputbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include inputbox.c -o inputbox -L/usr/local/lib -lbsddialog
% ./inputbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 6.2 passwordbox

**API**

```c
int bsddialog_passwordbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include passwordbox.c -o passwordbox -L/usr/local/lib -lbsddialog
% ./passwordbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 6.3 form

**API**

```c
int bsddialog_form(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include form.c -o form -L/usr/local/lib -lbsddialog
% ./form
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 6.4 passwordform

**API**

```c
int bsddialog_passwordform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include passwordform.c -o passwordform -L/usr/local/lib -lbsddialog
% ./passwordform
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 6.5 mixedform


**API**

```c
```

**Example**

```c
int bsddialog_mixedform(struct bsddialog_conf conf, char* text, int rows, int cols,
    int formheight, int argc, char **argv);
```

Compile and run

```
% cc -I/usr/local/include mixedform.c -o mixedform -L/usr/local/lib -lbsddialog
% ./mixedform
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 7 Editor

### 7.1 editbox

**API**

```c
int bsddialog_editbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include editbox.c -o editbox -L/usr/local/lib -lbsddialog
% ./editbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 8 Bar

### 8.1 gauge

**API**

```c
int bsddialog_gauge(struct bsddialog_conf conf, char* text, int rows, int cols, int perc);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include gauge.c -o gauge -L/usr/local/lib -lbsddialog
% ./gauge
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 8.2 mixedgauge

**API**

```c
int bsddialog_mixedgauge(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int perc, int argc, char **argv);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include mixedgauge.c -o mixedgauge -L/usr/local/lib -lbsddialog
% ./mixedgauge
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 8.3 rangebox

**API**

```c
int bsddialog_rangebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    int min, int max, int def);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include rangebox.c -o rangebox -L/usr/local/lib -lbsddialog
% ./rangebox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 8.4 pause


**API**

```c
```

**Example**

```c
int bsddialog_pause(struct bsddialog_conf conf, char* text, int rows, int cols, int sec);
```

Compile and run

```
% cc -I/usr/local/include pause.c -o pause -L/usr/local/lib -lbsddialog
% ./pause
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 9 Time

### 9.1 timebox

**API**

```c
int bsddialog_timebox(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int hh, unsigned int mm, unsigned int ss);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include timebox.c -o timebox -L/usr/local/lib -lbsddialog
% ./timebox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 9.1 calendar

**API**

```c
int bsddialog_calendar(struct bsddialog_conf conf, char* text, int rows, int cols,
    unsigned int yy, unsigned int mm, unsigned int dd);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include calendar.c -o calendar -L/usr/local/lib -lbsddialog
% ./calendar
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 10 Command

### 10.1 prgbox

**API**

```c
int bsddialog_prgbox(struct bsddialog_conf conf, char* text, int rows, int cols,
    char *command);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include prgbox.c -o prgbox -L/usr/local/lib -lbsddialog
% ./prgbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 10.2 programbox

**API**

```c
int bsddialog_programbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include programbox.c -o programbox -L/usr/local/lib -lbsddialog
% ./programbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 10.3 progressbox

**API**

```c
int bsddialog_progressbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include progressbox.c -o progressbox -L/usr/local/lib -lbsddialog
% ./progressbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 11 Text

### 11.1 tailbox

**API**

```c
int bsddialog_tailbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include tailbox.c -o tailbox -L/usr/local/lib -lbsddialog
% ./tailbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 11.2 tailboxbg

**API**

```c
int bsddialog_tailboxbg(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include tailboxbg.c -o tailboxbg -L/usr/local/lib -lbsddialog
% ./tailboxbg
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 11.3 textbox

**API**

```c
int bsddialog_textbox(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include textbox.c -o textbox -L/usr/local/lib -lbsddialog
% ./textbox
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

## 12 File

### 12.1 dselect

**API**

```c
int bsddialog_dselect(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include dselect.c -o dselect -L/usr/local/lib -lbsddialog
% ./dselect
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**

### 12.2 fselect

**API**

```c
int bsddialog_fselect(struct bsddialog_conf conf, char* text, int rows, int cols);
```

**Example**

```c
```

Compile and run

```
% cc -I/usr/local/include fselect.c -o fselect -L/usr/local/lib -lbsddialog
% ./fselect
```

Output

![example](./images/example.png)

**Exist Status**

**Configuration**

**Theme**
