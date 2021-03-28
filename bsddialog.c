#include <curses.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BSDDIALOG_VERSION "0.1"

/* Foreground_Background */
#define WHITE_WHITE	1
#define RED_RED		2 // to check
#define WHITE_RED	3 // to check
#define BLUE_BLUE	4
#define BLACK_WHITE	5
#define BLUE_WHITE	6
#define WHITE_BLUE	7

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
#define STRERR		61 // stderr
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

struct opts {
	int x;
	int y;
	char *title;
};

enum elevation { RAISED, LOWERED };

void usage(void);
int  init_view(bool enable_color);
WINDOW *
new_window(int x, int y, int rows, int cols, const char *title, int color,
    enum elevation elev, bool scrolling);
void window_scrolling_handler(WINDOW *pad, int rows, int cols);
void print_text(const char* text, int x, int y, bool bold, int color);
int  print_text_multiline(WINDOW *win, int x, int y, const char *str, int size_line);
/* widgets */
int msgbox_builder(struct opts opt, char* text, int rows, int cols, int argc, char **argv);
int infobox_builder(struct opts opt, char* text, int rows, int cols, int argc, char **argv);

void usage(void)
{

	printf("usage: dialog [-c | -h | -v]\n");
}

int main(int argc, char *argv[argc])
{
	bool enable_color = true;
	char title[1024], text[1024];
	int input, x, y, rows, cols;
	int (*widgetbuilder)(struct opts opt, char* text, int rows, int cols, int argc, char **argv) = NULL;
	struct opts myopt;

	/* options descriptor */
	static struct option longopts[] = {
	    /* common options */
	    { "ascii-lines", no_argument, NULL, 'X' },
	    { "aspect", required_argument, NULL	/*ratio*/, 'X' },
	    { "backtitle", required_argument, NULL /*backtitle*/, 'X' },
	    { "begin", required_argument, NULL /*y x*/, 'X' },
	    { "cancel-label", required_argument, NULL /*string*/, 'X' },
	    { "clear", no_argument, NULL, 'X' },
	    { "colors", no_argument, NULL, 'X' },
	    { "column-separator", required_argument, NULL /*string*/, 'X' },
	    { "cr-wrap", no_argument, NULL, 'X' },
	    { "create-rc", required_argument, NULL /*file*/, 'X' },
	    { "date-format", required_argument, NULL /*format*/, 'X' },
	    { "defaultno", no_argument, NULL, 'X' },
	    { "default-button", required_argument, NULL	/*string*/, 'X' },
	    { "default-item", required_argument, NULL /*string*/, 'X' },
	    { "exit-label", required_argument, NULL /*string*/, 'X' },
	    { "extra-button", no_argument, NULL, 'X' },
	    { "extra-label", required_argument, NULL /*string*/, 'X' },
	    { "help", no_argument, NULL, 'X' },
	    { "help-button", no_argument, NULL, 'X' },
	    { "help-label", required_argument, NULL /*string*/, 'X' },
	    { "help-status", no_argument, NULL, 'X' },
	    { "help-tags", no_argument, NULL, 'X' },
	    { "hfile", required_argument, NULL /*filename*/, 'X' },
	    { "hline", required_argument, NULL /*string*/, 'X' },
	    { "ignore", no_argument, NULL, 'X' },
	    { "input-fd", required_argument, NULL /*fd*/, 'X' },
	    { "insecure", no_argument, NULL, 'X' },
	    { "item-help", no_argument, NULL, 'X' },
	    { "keep-tite", no_argument, NULL, 'X' },
	    { "keep-window", no_argument, NULL, 'X' },
	    { "last-key", no_argument, NULL, 'X' },
	    { "max-input", required_argument, NULL /*size*/, 'X' },
	    { "no-cancel", no_argument, NULL, 'X' },
	    { "nocancel", no_argument, NULL, 'X' },
	    { "no-collapse", no_argument, NULL, 'X' },
	    { "no-items", no_argument, NULL, 'X' },
	    { "no-kill", no_argument, NULL, 'X' },
	    { "no-label", required_argument, NULL /*string*/, 'X' },
	    { "no-lines", no_argument, NULL, 'X' },
	    { "no-mouse", no_argument, NULL, 'X' },
	    { "no-nl-expand", no_argument, NULL, 'X' },
	    { "no-ok", no_argument, NULL, 'X' },
	    { "nook ", no_argument, NULL, 'X' },
	    { "no-shadow", no_argument, NULL, 'X' },
	    { "no-tags", no_argument, NULL, 'X' },
	    { "ok-label", required_argument, NULL /*string*/, 'X' },
	    { "output-fd", required_argument, NULL /*fd*/, 'X' },
	    { "separator", required_argument, NULL /*string*/, 'X' },
	    { "output-separator", required_argument, NULL /*string*/, 'X' },
	    { "print-maxsize", no_argument, NULL, 'X' },
	    { "print-size", no_argument, NULL, 'X' },
	    { "print-version", no_argument, NULL, PRINT_VERSION },
	    { "quoted", no_argument, NULL, 'X' },
	    { "scrollbar", no_argument, NULL, 'X' },
	    { "separate-output", no_argument, NULL, 'X' },
	    { "separate-widget", required_argument, NULL /*string*/, 'X' },
	    { "shadow", no_argument, NULL, 'X' },
	    { "single-quoted", no_argument, NULL, 'X' },
	    { "size-err", no_argument, NULL, 'X' },
	    { "sleep", required_argument, NULL /*secs*/, 'X' },
	    { "stderr", no_argument, NULL, 'X' },
	    { "stdout", no_argument, NULL, 'X' },
	    { "tab-correct", no_argument, NULL, 'X' },
	    { "tab-len", required_argument, NULL /*n*/, 'X' },
	    { "time-format", required_argument, NULL /*format*/, 'X' },
	    { "timeout", required_argument, NULL /*secs*/, 'X' },
	    { "title", required_argument, NULL /*title*/, TITLE },
	    { "trace", required_argument, NULL /*filename*/, 'X' },
	    { "trim", no_argument, NULL, 'X' },
	    { "version", no_argument, NULL, VERSION },
	    { "visit-items", no_argument, NULL, 'X' },
	    { "yes-label", required_argument, NULL /*string*/, 'X' },
	    /* Widgets */
	    { "buildlist", no_argument, NULL, 'X' },
	    { "calendar", no_argument, NULL, 'X' },
	    { "checklist", no_argument, NULL, 'X' },
	    { "dselect", no_argument, NULL, 'X' },
	    { "editbox", no_argument, NULL, 'X' },
	    { "form", no_argument, NULL, 'X' },
	    { "fselect", no_argument, NULL, 'X' },
	    { "gauge", no_argument, NULL, 'X' },
	    { "infobox", no_argument, NULL, INFOBOX },
	    { "inputbox", no_argument, NULL, 'X' },
	    { "inputmenu", no_argument, NULL, 'X' },
	    { "menu", no_argument, NULL, 'X' },
	    { "mixedform", no_argument, NULL, 'X' },
	    { "mixedgauge", no_argument, NULL, 'X' },
	    { "msgbox", no_argument, NULL, MSGBOX },
	    { "passwordbox", no_argument, NULL, 'X' },
	    { "passwordform", no_argument, NULL, 'X' },
	    { "pause", no_argument, NULL, 'X' },
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
	    { "yesno", no_argument, NULL, 'X' },
	    /* END */
	    { NULL, 0, NULL, 0 }
	};

	while ((input = getopt_long(argc, argv, "ch", longopts, NULL)) != -1) {
		switch (input) {
		/* Common options */
		case 'c':
			enable_color = false;
			break;
		case 'h':
			usage();
			printf(" -c\t\t Disable color\n");
			printf(" -h\t\t Display this help\n");
			printf(" -v\t\t Show version\n");
			printf("\n");
			printf("See \'man 1 bsddialog\' for more information.\n");
			return 0;
		case TITLE:
			//strcpy(title, optarg);
			myopt.title = optarg;
			break;
		case PRINT_VERSION:
		case VERSION:
			printf("bsddialog %s\n", BSDDIALOG_VERSION);
			return 0;
		/* Widgets */
		case INFOBOX:
			widgetbuilder = infobox_builder;
			break;
		case MSGBOX:
			widgetbuilder = msgbox_builder;
			break;
		/* Error */
		default:
			usage();
			return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 3) {
		usage();
		return (1);
	}
	strcpy(text, argv[0]);
	rows = atoi(argv[1]);
	rows--;
	cols = atoi(argv[2]);
	argc -= 3;
	argv += 3;

	if(init_view(enable_color) != 0) {
		printf("Cannot init ncurses\n");
		return 1;
	}
	refresh();

	myopt.x = LINES/2 - rows/2;
	myopt.y = COLS/2 - cols/2;
	widgetbuilder(myopt, text, rows, cols, argc /*unused*/, argv /*unused*/);

	endwin();
	return 0;
}

/* View */
int init_view(bool enable_color)
{
	int error = 0;

	if(initscr() == NULL)
		return -1;

	error += keypad(stdscr, TRUE);
	nl();
	error += cbreak();
	error += noecho();
	curs_set(0);

	if (enable_color) {
		error += start_color();
		error += init_pair(WHITE_WHITE, COLOR_WHITE, COLOR_WHITE);
		error += init_pair(WHITE_BLUE,  COLOR_WHITE, COLOR_BLUE);
		error += init_pair(RED_RED,     COLOR_RED,   COLOR_RED);
		error += init_pair(WHITE_RED,   COLOR_WHITE, COLOR_RED);

		error += init_pair(BLUE_BLUE,   COLOR_BLUE,  COLOR_BLUE);
		error += init_pair(BLACK_WHITE, COLOR_BLACK, COLOR_WHITE);
		error += init_pair(BLUE_WHITE,  COLOR_BLUE,  COLOR_WHITE);
	}

	bkgd(COLOR_PAIR(BLUE_BLUE));

	return error;
}

void print_text(const char* text, int x, int y, bool bold, int color)
{

	attron(COLOR_PAIR(color) | (bold ? A_BOLD : 0));
	mvaddstr(x, y, text);
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
new_window(int x, int y, int rows, int cols, const char *title, int color,
    enum elevation elev, bool scrolling)
{
	WINDOW *popup;
	int leftcolor, rightcolor;

	if (scrolling)
		popup = newpad(rows, cols);
	else
		//popup = newwin(rows, cols, 2, COLS/2 - cols/2);
		popup = newwin(rows, cols, x, y);

	wbkgd(popup, COLOR_PAIR(color));

	leftcolor  = elev == RAISED ? WHITE_WHITE : BLACK_WHITE;
	rightcolor = elev == RAISED ? BLACK_WHITE : WHITE_WHITE;
	wattron(popup, A_BOLD | COLOR_PAIR(leftcolor));
	box(popup, 0, 0);
	wattroff(popup, A_BOLD | COLOR_PAIR(leftcolor));

	wattron(popup, A_BOLD | COLOR_PAIR(rightcolor));
	mvwaddch(popup, 0, cols-1, ACS_URCORNER);
	mvwvline(popup, 1, cols-1, ACS_VLINE, rows-2);
	mvwaddch(popup, rows-1, cols-1, ACS_LRCORNER);
	mvwhline(popup, rows-1, 1, ACS_HLINE, cols-2);
	wattroff(popup, A_BOLD | COLOR_PAIR(rightcolor));


	wattron(popup, A_BOLD | COLOR_PAIR(BLUE_WHITE));
	wmove(popup, 0, cols/2 - strlen(title)/2);
	waddstr(popup, title);
	wattroff(popup, A_BOLD | COLOR_PAIR(BLUE_WHITE));

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

/* Widgets */
int 
infobox_builder(struct opts opt, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget;
	int line;

	widget = new_window(opt.x, opt.y, rows, cols, opt.title, BLACK_WHITE,
	    RAISED, false);
	print_text_multiline(widget, 1, 1, text, cols - 2);

	wrefresh(widget);
	getch();
	delwin(widget);

	return 0;
}

int 
msgbox_builder(struct opts opt, char* text, int rows, int cols, int argc, char **argv)
{
	WINDOW *widget, *key;
	int input;
	bool loop = true;

	widget = new_window(opt.x, opt.y, rows, cols, opt.title, BLACK_WHITE,
	    RAISED, false);
	mvwaddstr(widget, 1, 1, text);
	//WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x);
	key = new_window(opt.x+rows -2, opt.y, 3, cols, "", BLACK_WHITE, RAISED,
	    false);

	wattron(key, A_BOLD | COLOR_PAIR(WHITE_WHITE));
	mvwaddch(key, 0, 0, ACS_LTEE);
	wattroff(key, A_BOLD | COLOR_PAIR(WHITE_WHITE));

	wattron(key, A_BOLD | COLOR_PAIR(BLACK_WHITE));
	mvwaddch(key, 0, cols-1, ACS_RTEE);
	wattroff(key, A_BOLD | COLOR_PAIR(BLACK_WHITE));

	wrefresh(widget);
	wrefresh(key);

	while(loop) {
		input = getch();
		switch(input) {
		case 10: /* Enter */
		case 'O':
		case 'o':
			loop = false;
			break;
		}
	}

	delwin(key);
	delwin(widget);

	return 0;
}
