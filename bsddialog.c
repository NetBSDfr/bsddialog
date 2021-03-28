#ifdef PORTNCURSES
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION "0.1"

/* Foreground_Background */
#define WHITE_WHITE	1
#define RED_RED		2 // to check
#define WHITE_RED	3 // to check
#define BLUE_BLUE	4
#define BLACK_WHITE	5
#define BLUE_WHITE	6
#define WHITE_BLUE	7

enum elevation { RAISED, LOWERED };

void usage(void);
int  init_view(bool enable_color);
WINDOW *
new_window(int rows, int cols, const char *title, int color, enum elevation elev,
    bool scrolling);
void window_handler(WINDOW *window);
void window_scrolling_handler(WINDOW *pad, int rows, int cols);
void print_text(const char* text, int x, int y, bool bold, int color);
int  print_text_multiline(WINDOW *win, int x, int y, const char *str, int size_line);

void usage(void)
{

	printf("usage: dialog [-c | -h | -v]\n");
}

int main(int argc, char *argv[argc])
{
	bool enable_color = true;
	char title[1024], msgbox[1024];
	int input, row, cols;
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
	    { "output-separatorstring", no_argument, NULL, 'X' },
	    { "print-maxsize", no_argument, NULL, 'X' },
	    { "print-size", no_argument, NULL, 'X' },
	    { "print-version", no_argument, NULL, 'X' },
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
	    { "title", required_argument, /*&t*/ NULL, 't' },
	    { "trace", required_argument, NULL /*filename*/, 'X' },
	    { "trim", no_argument, NULL, 'X' },
	    { "version", no_argument, NULL, 'X' },
	    { "visit-items", no_argument, NULL, 'X' },
	    { "yes-label", required_argument, NULL /*string*/, 'X' },
	    /* Widgets */
/*buildlist, calendar, checklist, dselect, editbox,	form, fselect,
	      gauge, infobox, inputbox,	inputmenu, menu, mixedform,
	      mixedgauge, msgbox (message), passwordbox, passwordform, pause,
	      prgbox, programbox, progressbox, radiolist, rangebox, tailbox,
	      tailboxbg, textbox, timebox, treeview, and yesno (yes/no).*/
	    { "msgbox", required_argument, /*&m*/ NULL, 'm' },
	    { NULL, 0, NULL, 0 }
	};

	while ((input = getopt_long(argc, argv, "cf:hm:t:v", longopts, NULL)) != -1) {
		switch (input) {
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
		case 'f':
			/*if ((fd = open(optarg, O_RDONLY, 0)) == -1)
			err(1, "unable to open %s", optarg);*/
			break;
		case 'm':
			strcpy(msgbox, optarg);
			break;
		case 't':
			strcpy(title, optarg);
			break;
		case 'v':
			printf("bsddialog %s\n", VERSION);
			return 0;
		default:
			usage();
			return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		/* todo window checks */
		if (argc != 2) { /* msgbox */
			usage();
			return (1);
		}
	}

	argc = 0;
	//while (argv[argc]) {
		row  = atoi(argv[argc]);
		argc++;
		cols = atoi(argv[argc]);
    		argc++;
	//}

	if(init_view(enable_color) != 0) {
		printf("Cannot init ncurses\n");
		return 1;
	}
	refresh();

	WINDOW *popup = new_window(row, cols, title, BLACK_WHITE, RAISED, false);
	mvwaddstr(popup, 1, 1, msgbox);

	wrefresh(popup);
	window_handler(popup);
	delwin(popup);

	endwin();
	return 0;
}

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

/* Popup */

WINDOW *
new_window(int rows, int cols, const char *title, int color, enum elevation elev,
    bool scrolling)
{
	WINDOW *popup;
	int leftcolor, rightcolor;

	if (scrolling)
		popup = newpad(rows, cols);
	else
		//popup = newwin(rows, cols, 2, COLS/2 - cols/2);
		popup = newwin(rows, cols, LINES/2 - rows/2, COLS/2 - cols/2);

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

void window_handler(WINDOW* window)
{
	int input;
	bool loop = true;

	//wrefresh(window);
	//getch();
	//delwin(window);

	while(loop) {
		input = getch();
		switch(input) {
		case 'Q':
		case 'q':
		case  27: /* Esc */
			loop = false;
			break;
		case KEY_F(1):
		case '?':
		case 'H':
		case 'h':
			break;
		case '\t': /* TAB */
			break;
		case KEY_END:
		case KEY_HOME:
			break;
		case KEY_UP:
			break;
		case KEY_DOWN:
			break;
		case KEY_NPAGE:
			break;
		case KEY_PPAGE:
			break;
		case KEY_LEFT:
			break;
		case KEY_RIGHT:
			break;
		case KEY_RESIZE:
		case 'r':
		case 'R':
			break;
		}
	}
}

/*int select_device_popup(char *mixer)
{
	int i, h = 20, w = 45, input, ndevices;
	bool loop;
	WINDOW *selectwin, *subwin;
	MENU *menu;
	ITEM *devices[MAXDEVICES];
	int index = 0;
	char oidname[DEV_NAME + 12];
	char pcm[MAXDEVICES][DEV_NAME], desc[MAXDEVICES][DEV_DESC];
	size_t desclen;

	if((ndevices = get_ndevices(mixer)) < 0)
		return - 1;

	for(i=0; i < ndevices; i++)	{
		snprintf(pcm[i], DEV_NAME, "pcm%d", i);
		desclen = DEV_DESC;
		sprintf(oidname, "dev.pcm.%d.%%desc", i);
		sysctlbyname(oidname, desc[i], &desclen, NULL, 0);
		desc[i][desclen] = '\0';
		desc[i][w-4-strlen(pcm[i])] = '\0';
		devices[i] = new_item(pcm[i], desc[i]);
	}
	devices[i] = (ITEM *)NULL;

	h = (h < ndevices + 2) ? h : ndevices + 2;

	selectwin = new_window(h, w, " Select Device ", WHITE_BLUE, RAISED, false);

	menu = new_menu(devices);
	set_menu_fore(menu, COLOR_PAIR(WHITE_BLUE) | A_REVERSE);
	set_menu_back(menu, COLOR_PAIR(WHITE_BLUE));
	set_menu_win( menu, selectwin);
	set_menu_mark(menu, " ");
	set_menu_format(menu, h, 0);

	subwin = derwin(selectwin, h-2, w-2, 1, 1);
	set_menu_sub( menu, subwin);
	post_menu(menu);

	loop = true;
	while(loop) {
		wrefresh(selectwin);
		input = getch();
		switch(input) {
		case KEY_UP:
			menu_driver(menu, REQ_UP_ITEM);
			index = index > 0 ? index-1 : index;
			break;
		case KEY_DOWN:
			menu_driver(menu, REQ_DOWN_ITEM);
			index = index < ndevices-1 ? index+1 : index;
			break;
		case 10: // Enter
			loop = false;
			break;
		default:
			index = - 1;
			loop = false;
		}
	}

	for(i=0; i < ndevices; i++)
		free_item(devices[i]);
	free_menu(menu);
	delwin(subwin);
	delwin(selectwin);

	if(ndevices <= 0)
		index = -1;

	return index;
}*/

/*void sysctl_popup(const char *title, const char *root)
{
	WINDOW *popup;
	int h = 100, w = 55, line;
	const char *pname, *close="Press any key to close";
	struct sysctlmif_object_list *list;
	struct sysctlmif_object *obj;
	size_t valuesize;
	char value[MAXPATHLEN];

	if((list = sysctlmif_grouplistbyname(root)) == NULL) {
		sysctlinfo_error_popup();
		return;
	}

	popup = new_window(h, w, title, WHITE_BLUE, RAISED, true);

	line = 1;
	SLIST_FOREACH(obj, list, object_link) {
		valuesize = MAXPATHLEN;
		if(sysctl(obj->id, obj->idlevel, value, &valuesize, NULL, 0) != 0)
			continue;

		if(obj->type == CTLTYPE_STRING && strlen(value) == 0)
			continue;

		line += print_text_multiline(popup, line, 1, obj->desc, w-2);
		line++;
		pname = &obj->name[strlen(root)+1];
		mvwprintw(popup, line, 1, "%s: ", pname);
		if(obj->type == CTLTYPE_INT)
			mvwprintw(popup, line, strlen(pname)+3, "%d", *((int*)value));

		if(obj->type == CTLTYPE_UINT)
			mvwprintw(popup, line, strlen(pname)+3, "%u",
			    *((unsigned int*)value));

		if (obj->type == CTLTYPE_STRING) {
			value[valuesize] = '\0';
			line += print_text_multiline(popup, line, strlen(pname)+3,
			    value, w-2-strlen(pname)-3);
		}

		line += 2;
	}

	sysctlmif_freelist(list);

	mvwaddstr(popup, line, w/2 - strlen(close)/2, close);
	line++;
	wattron(popup, A_BOLD);
	mvwaddch(popup, line, 0, ACS_LLCORNER);
	mvwhline(popup, line, 1, 0, w-2);
	mvwaddch(popup, line, w-1, ACS_LRCORNER);
	wattroff(popup, A_BOLD);

	scrolling_popup(popup, line+1, w);

	delwin(popup);
}*/
