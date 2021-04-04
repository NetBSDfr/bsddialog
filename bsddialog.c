#include <sys/ioctl.h>

#include <curses.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BSDDIALOG_VERSION "0.1"

#define MAX(a,b) (((a)>(b))?(a):(b))

/* Exit */
#define BSDDIALOG_YESOK		 0 // YES or OK buttons
#define BSDDIALOG_NOCANCEL	 1 // No or Cancel buttons
#define BSDDIALOG_HELP		 2 // Help button
#define BSDDIALOG_EXTRA		 3 // Extra button
#define BSDDIALOG_ITEM_HELP	 4
#define BSDDIALOG_ERROR		-1 // generic error or ESC key

/* Foreground_Background */
#define BLUE_BLUE	1 // main background
#define BLUE_WHITE	2 // widget title
#define WHITE_WHITE	3 // widgets background and border
#define BLACK_WHITE	4 // widget border and key
#define RED_WHITE	5 // key
#define WHITE_BLUE	6 // key
#define YELLOW_BLUE	7 // key
#define BLACK_BLACK	8 // shadow
#define CYAN_BLUE	9 // backtitle

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
#define PASSWORD	88 // passwordbox
#define PASSWORDBOX	89 // passwordform
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

#define SIZEBUTTON	8

struct config {
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
	bool ignore;
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

enum elevation { RAISED, LOWERED, NOLINES };

void usage(void);
int  bsddialog_init(void);
WINDOW *
new_window(int x, int y, int rows, int cols, char *title, char *bottomtitle, 
    int color, enum elevation elev, bool asciilines, bool subwindowborders,
    bool scrolling);
void window_scrolling_handler(WINDOW *pad, int rows, int cols);
void print_text(WINDOW *window, int x, int y, char* text, bool bold, int color);
int  print_text_multiline(WINDOW *win, int x, int y, const char *str, int size_line);
void draw_button(WINDOW *window, int y, int size, char *text, bool selected);
int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int sleep, int fd);
/* widgets */
int checklist_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv);
int msgbox_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv);
int infobox_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv);
int inputbox_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv);
int pause_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv);
int yesno_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv);

void usage(void)
{

	printf("usage: bsddialog --help | -- version\n"\
	       "       bsddialog [common-opts] widget <text> <height> <width> "\
	       "[widget-opts]\n");
}

int main(int argc, char *argv[argc])
{
	bool enable_color = true;
	char text[1024], *backtitle = NULL;
	int input, rows, cols, output;
	int (*widgetbuilder)(struct config conf, char* text, int rows, int cols, int argc, char **argv) = NULL;
	WINDOW *shadow;
	struct winsize ws;
	struct config conf;

	memset(&conf, 0, sizeof(struct config));
	conf.x = -1;
	conf.y = -1;
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
	    { "date-format", required_argument, NULL /*format*/, 'X' },
	    { "defaultno", no_argument, NULL, DEFAULTNO },
	    { "default-button", required_argument, NULL	/*string*/, 'X' },
	    { "default-item", required_argument, NULL /*string*/, 'X' },
	    { "exit-label", required_argument, NULL /*string*/, EXIT_LABEL },
	    { "extra-button", no_argument, NULL, EXTRA_BUTTON },
	    { "extra-label", required_argument, NULL /*string*/, EXTRA_LABEL },
	    { "help", no_argument, NULL, HELP },
	    { "help-button", no_argument, NULL, HELP_BUTTON },
	    { "help-label", required_argument, NULL /*string*/, HELP_LABEL },
	    { "help-status", no_argument, NULL, 'X' },
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
	    { "quoted", no_argument, NULL, 'X' },
	    { "scrollbar", no_argument, NULL, 'X' },
	    { "separate-output", no_argument, NULL, 'X' },
	    { "separate-widget", required_argument, NULL /*string*/, 'X' },
	    { "shadow", no_argument, NULL, SHADOW },
	    { "single-quoted", no_argument, NULL, 'X' },
	    { "size-err", no_argument, NULL, 'X' },
	    { "sleep", required_argument, NULL /*secs*/, SLEEP },
	    { "stderr", no_argument, NULL, STDERR },
	    { "stdout", no_argument, NULL, STDOUT },
	    { "tab-correct", no_argument, NULL, 'X' },
	    { "tab-len", required_argument, NULL /*n*/, 'X' },
	    { "time-format", required_argument, NULL /*format*/, 'X' },
	    { "timeout", required_argument, NULL /*secs*/, 'X' },
	    { "title", required_argument, NULL /*title*/, TITLE },
	    { "trace", required_argument, NULL /*filename*/, 'X' },
	    { "trim", no_argument, NULL, 'X' },
	    { "version", no_argument, NULL, VERSION },
	    { "visit-items", no_argument, NULL, 'X' },
	    { "yes-label", required_argument, NULL /*string*/, YES_LABEL },
	    /* Widgets */
	    { "buildlist", no_argument, NULL, 'X' },
	    { "calendar", no_argument, NULL, 'X' },
	    { "checklist", no_argument, NULL, CHECKLIST },
	    { "dselect", no_argument, NULL, 'X' },
	    { "editbox", no_argument, NULL, 'X' },
	    { "form", no_argument, NULL, 'X' },
	    { "fselect", no_argument, NULL, 'X' },
	    { "gauge", no_argument, NULL, 'X' },
	    { "infobox", no_argument, NULL, INFOBOX },
	    { "inputbox", no_argument, NULL, INPUTBOX },
	    { "inputmenu", no_argument, NULL, 'X' },
	    { "menu", no_argument, NULL, 'X' },
	    { "mixedform", no_argument, NULL, 'X' },
	    { "mixedgauge", no_argument, NULL, 'X' },
	    { "msgbox", no_argument, NULL, MSGBOX },
	    { "passwordbox", no_argument, NULL, 'X' },
	    { "passwordform", no_argument, NULL, 'X' },
	    { "pause", no_argument, NULL, PAUSE },
	    { "prgbox", no_argument, NULL, },
	    { "programbox", no_argument, NULL, 'X' },
	    { "progressbox", no_argument, NULL, 'X' },
	    { "radiolist", no_argument, NULL, 'X' },
	    { "rangebox", no_argument, NULL, 'X' },
	    { "tailbox", no_argument, NULL, 'X' },
	    { "tailboxbg", no_argument, NULL, 'X' },
	    { "textbox", no_argument, NULL, 'X' },
	    { "timebox", no_argument, NULL, 'X' },
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
			conf.x = atoi(optarg);
			conf.y = atoi(argv[optind]);
			if (conf.x < 0 || conf.y < 0) {
				printf("Can't make new window at (%d,%d).",
				    conf.x, conf.y);
				return 1;
			}
			optind++;
			break;
		case CANCEL_LABEL:
			conf.cancel_label = optarg;
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
		case PRINT_MAXSIZE:
			conf.print_maxsize = true;
			break;
		case PRINT_SIZE:
			conf.print_size = true;
			break;
		case PRINT_VERSION:
			printf("bsddialog version %s\n", BSDDIALOG_VERSION);
			break;
		case SHADOW:
			conf.shadow = true;
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
		case CHECKLIST:
			widgetbuilder = checklist_builder;
			break;
		case INFOBOX:
			widgetbuilder = infobox_builder;
			break;
		case INPUTBOX:
			widgetbuilder = inputbox_builder;
			break;
		case MSGBOX:
			widgetbuilder = msgbox_builder;
			break;
		case PAUSE:
			widgetbuilder = pause_builder;
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

	if (argc != 3) {
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
	if (backtitle != NULL) {
		attron(COLOR_PAIR(CYAN_BLUE) | A_BOLD );
		mvaddstr(0, 1, backtitle);
		if (conf.no_lines != true) {
			mvhline(1, 1, conf.ascii_lines ? '-' : ACS_HLINE, COLS-2);
		}
		attroff(COLOR_PAIR(CYAN_BLUE) | A_BOLD);
	}
	refresh();

	conf.x = conf.x < 0 ? (LINES/2 - rows/2 - 1) : conf.x;
	conf.y = conf.y < 0 ? (COLS/2 - cols/2) : conf.y;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.x+1, conf.y+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	output = widgetbuilder(conf, text, rows, cols, argc, argv);

	if (conf.shadow)
		delwin(shadow);
	endwin();

	// debug
	printf("Exit status: %d\n", output);
	return output;
}

int bsddialog_init(void)
{
	int error = 0;

	if(initscr() == NULL)
		return -1;

	error += keypad(stdscr, TRUE);
	nl();
	error += cbreak();
	error += noecho();
	curs_set(0);

	error += start_color();
	error += init_pair(BLUE_BLUE,   COLOR_BLUE,   COLOR_BLUE);
	error += init_pair(BLUE_WHITE,  COLOR_BLUE,   COLOR_WHITE);
	error += init_pair(WHITE_WHITE, COLOR_WHITE,  COLOR_WHITE);
	error += init_pair(BLACK_WHITE, COLOR_BLACK,  COLOR_WHITE);
	error += init_pair(RED_WHITE,   COLOR_RED,    COLOR_WHITE);
	error += init_pair(WHITE_BLUE,  COLOR_WHITE,  COLOR_BLUE);
	error += init_pair(YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE);
	error += init_pair(BLACK_BLACK, COLOR_BLACK,  COLOR_BLACK);
	error += init_pair(CYAN_BLUE,   COLOR_CYAN,   COLOR_BLUE);

	bkgd(COLOR_PAIR(BLUE_BLUE));

	return error;
}

void print_text(WINDOW *window, int x, int y, char* text, bool bold, int color)
{

	attron(COLOR_PAIR(color) | (bold ? A_BOLD : 0));
	mvwaddstr(window, x, y, text);
	attroff(COLOR_PAIR(color) | (bold ? A_BOLD : 0));
}

int print_text_multiline(WINDOW *win, int x, int y, const char *str, int size_line)
{
	char fmtstr[8];
	int line = 0;
	
	sprintf(fmtstr, "%%.%ds", size_line);
	while(strlen(str) > 0) {
		mvwprintw(win, x + line, y, fmtstr, str);
		if((int)strlen(str) > size_line)
			str += size_line;
		else
			str += strlen(str);

		line++;
	}
	line = line > 0 ? line-1 : 0;
	return line;
}

WINDOW *
new_window(int x, int y, int rows, int cols, char *title, char *bottomtitle,
    int color, enum elevation elev, bool asciilines, bool subwindowborders,
    bool scrolling)
{
	WINDOW *popup;
	int leftcolor, rightcolor;
	int ls, rs, ts, bs, tl, tr, bl, br;
	int ltee, rtee;

	ls = rs = ACS_VLINE;
	ts = bs = ACS_HLINE;
	tl = ACS_ULCORNER;
	tr = ACS_URCORNER;
	bl = ACS_LLCORNER;
	br = ACS_LRCORNER;
	ltee = ACS_LTEE;
	rtee = ACS_RTEE;

	popup = scrolling ? newpad(rows, cols) : newwin(rows, cols, x, y);
	wbkgd(popup, COLOR_PAIR(color));

	if (elev != NOLINES) {
		if (asciilines) {
			ls = rs = '|';
			ts = bs = '-';
			tl = tr = bl = br = ltee = rtee = '+';
		}
		leftcolor  = elev == RAISED ? WHITE_WHITE : BLACK_WHITE;
		rightcolor = elev == RAISED ? BLACK_WHITE : WHITE_WHITE;
		wattron(popup, A_BOLD | COLOR_PAIR(leftcolor));
		wborder(popup, ls, rs, ts, bs, tl, tr, bl, br);
		wattroff(popup, A_BOLD | COLOR_PAIR(leftcolor));

		wattron(popup, A_BOLD | COLOR_PAIR(rightcolor));
		mvwaddch(popup, 0, cols-1, tr);
		mvwvline(popup, 1, cols-1, rs, rows-2);
		mvwaddch(popup, rows-1, cols-1, br);
		mvwhline(popup, rows-1, 1, bs, cols-2);
		wattroff(popup, A_BOLD | COLOR_PAIR(rightcolor));

		if (subwindowborders) {
			wattron(popup, A_BOLD | COLOR_PAIR(leftcolor));
			mvwaddch(popup, 0, 0, ltee);
			wattroff(popup, A_BOLD | COLOR_PAIR(leftcolor));

			wattron(popup, A_BOLD | COLOR_PAIR(rightcolor));
			mvwaddch(popup, 0, cols-1, rtee);
			wattroff(popup, A_BOLD | COLOR_PAIR(rightcolor));
		}
	}

	if (title != NULL) {
		wattron(popup, A_BOLD | COLOR_PAIR(BLUE_WHITE));
		wmove(popup, 0, cols/2 - strlen(title)/2);
		waddstr(popup, title);
		wattroff(popup, A_BOLD | COLOR_PAIR(BLUE_WHITE));
	}

	if (bottomtitle != NULL) {
		wattron(popup, A_BOLD | COLOR_PAIR(BLACK_WHITE));
		wmove(popup, rows - 1, cols/2 - strlen(bottomtitle)/2 - 1);
		waddch(popup, '[');
		waddstr(popup, bottomtitle);
		waddch(popup, ']');
		wattroff(popup, A_BOLD | COLOR_PAIR(BLACK_WHITE));
	}

	return popup;
}

void window_scrolling_handler(WINDOW *pad, int rows, int cols)
{
	int input, cur_line = 0, shown_lines;
	bool loop = true;
	int x = 2, y = COLS/2 - cols/2;

	shown_lines = rows > (LINES - x - 1) ? (LINES - x - 1) : rows;
	wattron(pad, COLOR_PAIR(WHITE_BLUE) | A_BOLD);
	while(loop) {
		mvwvline(pad, 1, cols-1, ACS_VLINE, rows-2);
		if(cur_line > 0)
			mvwaddch(pad, cur_line, cols-1, ACS_UARROW);
		if(cur_line + shown_lines < rows)
			mvwaddch(pad, cur_line + shown_lines-1, cols-1, 'v');
		prefresh(pad, cur_line, 0, x, y, shown_lines+1, COLS-2);
		input = getch();
		switch(input) {
		case KEY_UP:
			if (cur_line > 0)
				cur_line--;
			break;
		case KEY_DOWN:
			if (cur_line + shown_lines < rows)
				cur_line++;
			break;
		default:
			loop = false;
		}
	}
	wattroff(pad, COLOR_PAIR(WHITE_BLUE) | A_BOLD);
}

void draw_button(WINDOW *window, int start_y, int size, char *text, bool selected)
{
	int i, y, color_arrows, color_first_char, color_tail_chars;

	if (selected) {
		color_arrows = A_BOLD | COLOR_PAIR(WHITE_BLUE);
		color_first_char = A_BOLD | COLOR_PAIR(WHITE_BLUE);
		color_tail_chars = A_BOLD | COLOR_PAIR(YELLOW_BLUE);
	} else {
		color_arrows = COLOR_PAIR(BLACK_WHITE);
		color_first_char = A_BOLD | COLOR_PAIR(RED_WHITE);
		color_tail_chars = A_BOLD | COLOR_PAIR(BLACK_WHITE);
	}

	wattron(window, color_arrows);
	mvwaddch(window, 1, start_y, '<');
	for(i = 1; i < size - 1; i++)
		mvwaddch(window, 1, start_y + i, ' ');
	mvwaddch(window, 1, start_y + i, '>');
	wattroff(window, color_arrows);

	y = start_y + 1 + ((size - 2 - strlen(text))/2);

	wattron(window, color_tail_chars);
	mvwaddstr(window, 1, y, text);
	wattroff(window, color_tail_chars);

	wattron(window, color_first_char);
	mvwaddch(window, 1, y, text[0]);
	wattroff(window, color_first_char);
}

int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int sleeptime, int fd)
{
	bool loop = true, update;
	int i, y, start_y, size, input;
	int output;
#define BUTTONSPACE 3

	size = MAX(SIZEBUTTON - 2, strlen(buttons[0]));
	for (i=1; i < nbuttons; i++)
		size = MAX(size, strlen(buttons[i]));
	size += 2;

	start_y = size * nbuttons + (nbuttons - 1) * BUTTONSPACE;
	start_y = cols/2 - start_y/2;

	for (i = 0; i < nbuttons; i++) {
		y = i * (size + BUTTONSPACE);
		draw_button(window, start_y + y, size, buttons[i], i == selected);
	}

	while(loop) {
		wrefresh(window);
		input = getch();
		if (input == 10 ) { // Enter
			output = values[selected]; // the caller knows the value
			loop = false;
		} else if (input == 27) { // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
		} else if (input == '\t') { // TAB
			selected = (selected + 1) % nbuttons;
			update = true;
		} else if (input == KEY_LEFT) {
			if (selected > 0) {
				selected--;
				update = true;
			}
		} else if (input == KEY_RIGHT) {
			if (selected < nbuttons - 1) {
				selected++;
				update = true;
			}
		} else if (shortkey) {
			for (i = 0; i < nbuttons; i++)
				if (input == (buttons[i])[0]) {
					output = values[selected]; // like Esc
					loop = false;
				}
		}

		if (update) {
			for (i = 0; i < nbuttons; i++) {
				y = i * (size + BUTTONSPACE);
				draw_button(window, start_y + y, size, buttons[i], i == selected);
			}
			update = false;
		}
	}

	sleep(sleeptime);

	return output;
}

/* Widgets */
int
checklist_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget, *button, *entry;
	char *buttons[4];
	int values[4], output, nbuttons = 0, defbutton = 0;

	widget = new_window(conf.x, conf.y, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	//WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
	entry = new_window(conf.x + rows - 6, conf.y +1, 3, cols-2, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false, false);
	button = new_window(conf.x + rows -3, conf.y, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);
	wrefresh(entry);

	if (conf.no_ok == false) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons++;
	}

	if (conf.extra_button) {
		buttons[nbuttons] = conf.extra_label;
		values[nbuttons] = BSDDIALOG_EXTRA;
		nbuttons++;
	}

	if (conf.no_cancel == false) {
		buttons[nbuttons] = conf.cancel_label;
		values[nbuttons] = BSDDIALOG_NOCANCEL;
		if (conf.defaultno)
			defbutton = nbuttons;
		nbuttons++;
	}

	if (conf.help_button) {
		buttons[nbuttons] = conf.help_label;
		values[nbuttons] = BSDDIALOG_HELP;
		nbuttons++;
	}

	if (nbuttons == 0) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons = 1;
	}

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /* fd */ 0);

	delwin(button);
	delwin(entry);
	delwin(widget);

	if (conf.print_size)
		dprintf(conf.output_fd, "Checklist size: %d, %d\n", rows, cols);

	return output;
}

int 
infobox_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget;

	widget = new_window(conf.x, conf.y, rows, cols, conf.title, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);

	wrefresh(widget);
	getch();
	delwin(widget);

	if (conf.print_size)
		dprintf(conf.output_fd, "Infobox size: %d, %d\n", rows, cols);

	return (BSDDIALOG_YESOK);
}

int 
msgbox_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget, *button;
	char *buttons[3];
	int values[3], output, nbuttons = 0, defbutton = 0;

	widget = new_window(conf.x, conf.y, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	//WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
	button = new_window(conf.x+rows -3, conf.y, 3, cols, NULL, conf.hline, BLACK_WHITE, 
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);

	if (conf.no_ok == false) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons++;
	}

	if (conf.extra_button) {
		buttons[nbuttons] = conf.extra_label;
		values[nbuttons] = BSDDIALOG_EXTRA;
		nbuttons++;
	}

	if (conf.help_button) {
		buttons[nbuttons] = conf.help_label;
		values[nbuttons] = BSDDIALOG_HELP;
		nbuttons++;
	}

	if (nbuttons == 0) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons = 1;
	}

	output = buttons_handler(button, cols, nbuttons, buttons, values, 0,
	    true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(widget);

	if (conf.print_size)
		dprintf(conf.output_fd, "Msgbox size: %d, %d\n", rows, cols);

	return output;
}

int
inputbox_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget, *button, *entry;
	char *buttons[4];
	int values[4], output, nbuttons = 0, defbutton = 0;

	widget = new_window(conf.x, conf.y, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	//WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
	entry = new_window(conf.x + rows - 6, conf.y +1, 3, cols-2, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false, false);
	button = new_window(conf.x + rows -3, conf.y, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);
	wrefresh(entry);

	if (conf.no_ok == false) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons++;
	}

	if (conf.extra_button) {
		buttons[nbuttons] = conf.extra_label;
		values[nbuttons] = BSDDIALOG_EXTRA;
		nbuttons++;
	}

	if (conf.no_cancel == false) {
		buttons[nbuttons] = conf.cancel_label;
		values[nbuttons] = BSDDIALOG_NOCANCEL;
		if (conf.defaultno)
			defbutton = nbuttons;
		nbuttons++;
	}

	if (conf.help_button) {
		buttons[nbuttons] = conf.help_label;
		values[nbuttons] = BSDDIALOG_HELP;
		nbuttons++;
	}

	if (nbuttons == 0) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons = 1;
	}

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(entry);
	delwin(widget);

	if (conf.print_size)
		dprintf(conf.output_fd, "Inputbox size: %d, %d\n", rows, cols);

	return output;
}

int
pause_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget, *button, *entry;
	char *buttons[4];
	int values[4], output, nbuttons = 0, defbutton = 0;

	widget = new_window(conf.x, conf.y, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	//WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
	entry = new_window(conf.x + rows - 6, conf.y +2, 3, cols-4, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	button = new_window(conf.x + rows -3, conf.y, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);
	wrefresh(entry);

	if (conf.no_ok == false) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons++;
	}

	if (conf.extra_button) {
		buttons[nbuttons] = conf.extra_label;
		values[nbuttons] = BSDDIALOG_EXTRA;
		nbuttons++;
	}

	if (conf.no_cancel == false) {
		buttons[nbuttons] = conf.cancel_label;
		values[nbuttons] = BSDDIALOG_NOCANCEL;
		if (conf.defaultno)
			defbutton = nbuttons;
		nbuttons++;
	}

	if (conf.help_button) {
		buttons[nbuttons] = conf.help_label;
		values[nbuttons] = BSDDIALOG_HELP;
		nbuttons++;
	}

	if (nbuttons == 0) {
		buttons[0] = conf.ok_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons = 1;
	}

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(entry);
	delwin(widget);

	if (conf.print_size)
		dprintf(conf.output_fd, "Pause size: %d, %d\n", rows, cols);

	return output;
}

int
yesno_builder(struct config conf, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget, *button;
	char *buttons[4];
	int values[4], output, nbuttons = 0, defbutton = 0;

	widget = new_window(conf.x, conf.y, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	//WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
	button = new_window(conf.x+rows -3, conf.y, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);

	if (conf.no_ok == false) {
		buttons[0] = conf.yes_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons++;
	}

	if (conf.extra_button) {
		buttons[nbuttons] = conf.extra_label;
		values[nbuttons] = BSDDIALOG_EXTRA;
		nbuttons++;
	}

	if (conf.no_cancel == false) {
		buttons[nbuttons] = conf.no_label;
		values[nbuttons] = BSDDIALOG_NOCANCEL;
		if (conf.defaultno)
			defbutton = nbuttons;
		nbuttons++;
	}

	if (conf.help_button) {
		buttons[nbuttons] = conf.help_label;
		values[nbuttons] = BSDDIALOG_HELP;
		nbuttons++;
	}

	if (nbuttons == 0) {
		buttons[0] = conf.yes_label;
		values[0] = BSDDIALOG_YESOK;
		nbuttons = 1;
	}

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(widget);

	if (conf.print_size)
		dprintf(conf.output_fd, "Yesno size: %d, %d\n", rows, cols);

	return output;
}
