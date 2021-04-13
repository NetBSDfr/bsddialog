#include <string.h>
#include <unistd.h>

#ifdef PORTNCURSES
#include <ncurses/curses.h>
#include <ncurses/form.h>
#else
#include <curses.h>
#include <form.h>
#endif

#include "bsddialog.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

/* Foreground_Background */
#define BLUE_BLUE	1 // main background
#define BLUE_WHITE	2 // widget title
#define WHITE_WHITE	3 // widget background and border
#define BLACK_WHITE	4 // widget border and key
#define RED_WHITE	5 // key
#define WHITE_BLUE	6 // key
#define YELLOW_BLUE	7 // key
#define BLACK_BLACK	8 // shadow
#define CYAN_BLUE	9 // backtitle

enum elevation { RAISED, LOWERED, NOLINES };

WINDOW *
new_window(int y, int x, int rows, int cols, char *title, char *bottomtitle,
    int color, enum elevation elev, bool asciilines, bool subwindowborders,
    bool scrolling);
void window_scrolling_handler(WINDOW *pad, int rows, int cols);
void print_text(WINDOW *window, int y, int x, char* text, bool bold, int color);
int  print_text_multiline(WINDOW *win, int y, int x, const char *str, int size_line);

void
draw_button(WINDOW *window, int x, int size, char *text, bool selected,
    bool shortkey);
void
draw_buttons(WINDOW *window, int cols, int nbuttons, char **buttons,
    int selected, bool shortkey);
void
get_buttons(int *nbuttons, char *buttons[4], int values[4], bool yesok,
    char* yesoklabel, bool extra, char *extralabel, bool nocancel,
    char *nocancellabel, bool help, char *helplabel, bool defaultno,
    int *defbutton);

int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int sleep, int fd);
int
bar_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons, int *values,
    int selected, bool shortkey, WINDOW *bar, int sizebar, int min, int max,
    int def, int sleeptime, int fd);
int
forms_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *entry, FORM *form,
    FIELD **field, bool showinput, int sleeptime, int fd);


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

	refresh();

	return error;
}

void bsddialog_end(void)
{

	endwin();
}

void bsddialog_centreyx(int rows, int cols, int *y, int *x)
{
	if (y != NULL)
		*y = LINES/2 - rows/2 - 1;

	if (x != NULL)
		*x = COLS/2 - cols/2;
}

int bsddialog_backtitle(struct config conf, char *backtitle)
{

	attron(COLOR_PAIR(CYAN_BLUE) | A_BOLD );
	mvaddstr(0, 1, backtitle);
	if (conf.no_lines != true)
		mvhline(1, 1, conf.ascii_lines ? '-' : ACS_HLINE, COLS-2);

	attroff(COLOR_PAIR(CYAN_BLUE) | A_BOLD);

	refresh();

	return 0;
}

void print_text(WINDOW *window, int y, int x, char* text, bool bold, int color)
{

	attron(COLOR_PAIR(color) | (bold ? A_BOLD : 0));
	mvwaddstr(window, y, x, text);
	attroff(COLOR_PAIR(color) | (bold ? A_BOLD : 0));
}

int print_text_multiline(WINDOW *win, int y, int x, const char *str, int size_line)
{
	char fmtstr[8];
	int line = 0;
	
	sprintf(fmtstr, "%%.%ds", size_line);
	while(strlen(str) > 0) {
		mvwprintw(win, y + line, x, fmtstr, str);
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
new_window(int y, int x, int rows, int cols, char *title, char *bottomtitle,
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

	popup = scrolling ? newpad(rows, cols) : newwin(rows, cols, y, x);
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
	int x = 2, y = COLS/2 - cols/2; /* tofix x & y*/

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

void
draw_button(WINDOW *window, int x, int size, char *text, bool selected,
    bool shortkey)
{
	int i, color_arrows, color_first_char, color_tail_chars;

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
	mvwaddch(window, 1, x, '<');
	for(i = 1; i < size - 1; i++)
		mvwaddch(window, 1, x + i, ' ');
	mvwaddch(window, 1, x + i, '>');
	wattroff(window, color_arrows);

	x = x + 1 + ((size - 2 - strlen(text))/2);

	wattron(window, color_tail_chars);
	mvwaddstr(window, 1, x, text);
	wattroff(window, color_tail_chars);

	if (shortkey) {
		wattron(window, color_first_char);
		mvwaddch(window, 1, x, text[0]);
		wattroff(window, color_first_char);
	}
}

void
draw_buttons(WINDOW *window, int cols, int nbuttons, char **buttons,
    int selected, bool shortkey)
{
	int i, x, start_x, size;
#define SIZEBUTTON  8
#define BUTTONSPACE 3

	size = MAX(SIZEBUTTON - 2, strlen(buttons[0]));
	for (i=1; i < nbuttons; i++)
		size = MAX(size, strlen(buttons[i]));
	size += 2;

	start_x = size * nbuttons + (nbuttons - 1) * BUTTONSPACE;
	start_x = cols/2 - start_x/2;

	for (i = 0; i < nbuttons; i++) {
		x = i * (size + BUTTONSPACE);
		draw_button(window, start_x + x, size, buttons[i],
		    i == selected, shortkey);
	}
}

void
get_buttons(int *nbuttons, char *buttons[4], int values[4], bool yesok, 
    char *yesoklabel, bool extra, char *extralabel, bool nocancel,
    char *nocancellabel, bool help, char *helplabel, bool defaultno,
    int *defbutton)
{

	*nbuttons = 0;
	*defbutton = 0;

	if (yesok) {
		buttons[0] = yesoklabel;
		values[0] = BSDDIALOG_YESOK;
		*nbuttons = *nbuttons + 1;
	}

	if (extra) {
		buttons[*nbuttons] = extralabel;
		values[*nbuttons] = BSDDIALOG_EXTRA;
		*nbuttons = *nbuttons + 1;
	}

	if (nocancel) {
		buttons[*nbuttons] = nocancellabel;
		values[*nbuttons] = BSDDIALOG_NOCANCEL;
		if (defaultno)
			*defbutton = *nbuttons;
		*nbuttons = *nbuttons + 1;
	}

	if (help) {
		buttons[*nbuttons] = helplabel;
		values[*nbuttons] = BSDDIALOG_HELP;
		*nbuttons = *nbuttons + 1;
	}

	if (*nbuttons == 0) {
		buttons[0] = yesoklabel;
		values[0] = BSDDIALOG_YESOK;
		*nbuttons = 1;
	}
}

int
buttons_handler(WINDOW *window, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, int sleeptime, int fd)
{
	bool loop, update;
	int i, input;
	int output;

	loop = update = true;
	while(loop) {
		if (update) {
			draw_buttons(window, cols, nbuttons, buttons, selected,
			    shortkey);
			update = false;
		}
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
	}

	sleep(sleeptime);

	return output;
}

/* Widgets */
int
bsddialog_checklist(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *entry, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	entry = new_window(conf.y + rows - 6, conf.x + 1, 3, cols-2, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false, false);
	button = new_window(conf.y + rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);
	wrefresh(entry);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /* fd */ 0);

	delwin(button);
	delwin(entry);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Checklist size: %d, %d\n", rows, cols);

	return output;
}

int 
bsddialog_infobox(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *shadow;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);

	wrefresh(widget);
	getch();
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Infobox size: %d, %d\n", rows, cols);

	return (BSDDIALOG_YESOK);
}

int 
bsddialog_msgbox(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[3];
	int values[3], output, nbuttons, defbutton;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	button = new_window(conf.y+rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, false, NULL,
	conf.help_button, conf.help_label, false, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values, 0,
	    true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Msgbox size: %d, %d\n", rows, cols);

	return output;
}

int
bsddialog_pause(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *entry, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	entry = new_window(conf.y + rows - 6, conf.x + 2, 3, cols-4, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	button = new_window(conf.y + rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);
	wrefresh(entry);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(entry);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Pause size: %d, %d\n", rows, cols);

	return output;
}

int
bsddialog_yesno(struct config conf, char* text, int rows, int cols)
{
	WINDOW *widget, *button, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	button = new_window(conf.y+rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	wrefresh(widget);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.yes_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.no_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	output = buttons_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, conf.sleep, /*fd*/ 0);

	delwin(button);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Yesno size: %d, %d\n", rows, cols);

	return output;
}

/* Forms: Form, Inputbox, Inputmenu, Mixedform, Password, Passwordform */
int
forms_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *entry, FORM *form,
    FIELD **field, bool showinput, int sleeptime, int fd)
{
	bool loop = true, buttupdate, inentry = true;
	int input, output, buflen = 0, pos = 0;
	char *bufp;

	curs_set(showinput ? 2 : 0);
	pos_form_cursor(form);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(buttwin, cols, nbuttons, buttons, selected,
			    shortkey);
			buttupdate = false;
		}
		wrefresh(buttwin);
		wrefresh(entry);
		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selected]; // values -> outputs
			loop = false;
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
			bufp = field_buffer(field[0], 0);
			bufp[buflen] = '\0';
			dprintf(fd, "%s", bufp);
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			selected = (selected + 1) % nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (inentry) {
				form_driver(form, REQ_PREV_CHAR);
				pos = pos > 0 ? pos - 1 : 0;
			} else {
				if (selected > 0) {
					selected--;
					buttupdate = true;
				}
			}
			break;
		case KEY_RIGHT:
			if (inentry) {
				if (pos < buflen) {
					form_driver(form, REQ_NEXT_CHAR);
					pos++;
				}
			} else {
				if (selected < nbuttons - 1) {
					selected++;
					buttupdate = true;
				}
			}
			break;
		case KEY_UP:
			inentry = true;
			curs_set(showinput ? 2 : 0);
			pos_form_cursor(form);
			break;
		case KEY_DOWN:
			inentry = false;
			curs_set(0);
			break;
		case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
			buflen = buflen > 0 ? buflen - 1 : 0;
			pos = pos > 0 ? pos - 1 : 0;
			break;
		case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
			buflen = buflen > 0 ? buflen - 1 : 0;
			break;
		default:
			if (inentry) {
				form_driver(form, input);
				buflen++;
				pos++;
			}
			break;
		}
	}

	sleep(sleeptime);

	curs_set(0);

	return output;
}

int do_forms(struct config conf, char* text, int rows, int cols, bool showinput)
{
	WINDOW *widget, *button, *entry, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;
	FIELD *field[2];
	FORM *form;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	entry = new_window(conf.y + rows - 6, conf.x +1, 3, cols-2, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false, false);
	button = new_window(conf.y + rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	field[0] = new_field(1, cols-4, 0, 0, 0, 0);
	field[1] = NULL;

	field_opts_off(field[0], O_AUTOSKIP);
	field_opts_off(field[0], O_STATIC);
	if (showinput == false)
		field_opts_off(field[0], O_PUBLIC);
	set_field_fore(field[0], COLOR_PAIR(BLACK_WHITE));
	set_field_back(field[0], COLOR_PAIR(BLACK_WHITE));

	form = new_form(field);
	set_form_win(form, entry);
	set_form_sub(form, derwin(entry, 1, cols-4, 1, 1));
	post_form(form);

	wrefresh(widget);
	wrefresh(entry);

	output = forms_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, entry, form, field, showinput, conf.sleep,
	    conf.output_fd);

	unpost_form(form);
	free_form(form);
	free_field(field[0]);

	delwin(button);
	delwin(entry);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Inputbox size: %d, %d\n", rows, cols);

	return output;
}

int bsddialog_form(struct config conf, char* text, int rows, int cols)
{
	return 0;
}

int bsddialog_inputbox(struct config conf, char* text, int rows, int cols)
{
	int output;

	output = do_forms(conf, text, rows, cols, true);

	return output;
}


int bsddialog_inputmenu(struct config conf, char* text, int rows, int cols)
{
	return 0;
}

#define ISITEMHIDDEN (item) (item->itemflags & 0x1)
#define ISITEMWRITABLE (item) (item->itemflags & 0x2)
struct formitem {
	char *label;
	unsigned int ylabel;
	unsigned int xlabel;
	char *item;
	unsigned int yitem;
	unsigned int xitem;
	int itemlen;
	unsigned int inputlen;
	unsigned int itemflags;
};

int
mixedform_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *entry, FORM *form,
    FIELD **field, bool showinput, int sleeptime, int fd)
{
	bool loop = true, buttupdate, inentry = true;
	int input, output, buflen = 0, pos = 0;
	char *bufp;

	curs_set(showinput ? 2 : 0);
	pos_form_cursor(form);
	loop = buttupdate = true;
	while(loop) {
		if (buttupdate) {
			draw_buttons(buttwin, cols, nbuttons, buttons, selected,
			    shortkey);
			buttupdate = false;
		}
		wrefresh(buttwin);
		wrefresh(entry);
		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selected]; // values -> outputs
			loop = false;
			form_driver(form, REQ_NEXT_FIELD);
			form_driver(form, REQ_PREV_FIELD);
			bufp = field_buffer(field[0], 0);
			bufp[buflen] = '\0';
			dprintf(fd, "%s", bufp);
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			selected = (selected + 1) % nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (inentry) {
				form_driver(form, REQ_PREV_CHAR);
				pos = pos > 0 ? pos - 1 : 0;
			} else {
				if (selected > 0) {
					selected--;
					buttupdate = true;
				}
			}
			break;
		case KEY_RIGHT:
			if (inentry) {
				if (pos < buflen) {
					form_driver(form, REQ_NEXT_CHAR);
					pos++;
				}
			} else {
				if (selected < nbuttons - 1) {
					selected++;
					buttupdate = true;
				}
			}
			break;
		case KEY_UP:
			inentry = true;
			curs_set(showinput ? 2 : 0);
			pos_form_cursor(form);
			break;
		case KEY_DOWN:
			inentry = false;
			curs_set(0);
			break;
		case KEY_BACKSPACE:
			form_driver(form, REQ_DEL_PREV);
			buflen = buflen > 0 ? buflen - 1 : 0;
			pos = pos > 0 ? pos - 1 : 0;
			break;
		case KEY_DC:
			form_driver(form, REQ_DEL_CHAR);
			buflen = buflen > 0 ? buflen - 1 : 0;
			break;
		default:
			if (inentry) {
				form_driver(form, input);
				buflen++;
				pos++;
			}
			break;
		}
	}

	sleep(sleeptime);

	curs_set(0);

	return output;
}

int do_mixedform(struct config conf, char* text, int rows, int cols, bool showinput)
{
	WINDOW *widget, *button, *entry, *shadow;
	char *buttons[4];
	int values[4], output, nbuttons, defbutton;
	FIELD *field[2];
	FORM *form;

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	entry = new_window(conf.y + rows - 6, conf.x +1, 3, cols-2, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : LOWERED, conf.ascii_lines, false, false);
	button = new_window(conf.y + rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	field[0] = new_field(1, cols-4, 0, 0, 0, 0);
	field[1] = NULL;

	field_opts_off(field[0], O_AUTOSKIP);
	field_opts_off(field[0], O_STATIC);
	if (showinput == false)
		field_opts_off(field[0], O_PUBLIC);
	set_field_fore(field[0], COLOR_PAIR(BLACK_WHITE));
	set_field_back(field[0], COLOR_PAIR(BLACK_WHITE));

	form = new_form(field);
	set_form_win(form, entry);
	set_form_sub(form, derwin(entry, 1, cols-4, 1, 1));
	post_form(form);

	wrefresh(widget);
	wrefresh(entry);

	output = mixedform_handler(button, cols, nbuttons, buttons, values,
	    defbutton, true, entry, form, field, showinput, conf.sleep,
	    conf.output_fd);

	unpost_form(form);
	free_form(form);
	free_field(field[0]);

	delwin(button);
	delwin(entry);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.print_size)
		dprintf(conf.output_fd, "Inputbox size: %d, %d\n", rows, cols);

	return output;
}

int bsddialog_mixedform(struct config conf, char* text, int rows, int cols)
{
	int output;
	struct formitem items[4] = {
		{"L1:", 0, 0, "Item1", 0, 5, 10, 15, 0},
		{"L2:", 1, 0, "Item2", 1, 5, 10, 15, 1},
		{"L3:", 2, 0, "Item3", 2, 5, 10, 15, 2},
		{"L4:", 3, 0, "Item4", 3, 5, 10, 15, 3}
	};

	output = do_mixedform(conf, text, rows, cols, true);

	return output;
}

int bsddialog_passwordbox(struct config conf, char* text, int rows, int cols)
{
	int output;

	output = do_forms(conf, text, rows, cols, false);

	return output;
}

int bsddialog_passwordform(struct config conf, char* text, int rows, int cols)
{
	return 0;
}

 /* Gauge and rangebox */
int bsddialog_gauge(struct config conf, char* text, int rows, int cols, int perc)
{
	WINDOW *widget, *bar, *shadow;
	char percstr[5], input[2048];
	int i, blue_x, color;
	bool mainloop = true;

	blue_x = (int)((perc*(cols-8))/100);

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	bar = new_window(conf.y+rows -4, conf.x+3, 3, cols-6, NULL, conf.hline, BLACK_WHITE, 
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);

	wrefresh(widget);
	wrefresh(bar);

	while (mainloop) {
		for (i = 0; i < cols - 8; i++) {
			if  (i <= blue_x) {
				wattron(bar, A_BOLD | COLOR_PAIR(BLUE_BLUE));
				mvwaddch(bar, 1, i + 1, ' ');
				wattroff(bar, A_BOLD | COLOR_PAIR(BLUE_BLUE));
			}
			else {
				wattron(bar, A_BOLD | COLOR_PAIR(WHITE_WHITE));
				mvwaddch(bar, 1, i, ' ');
				wattroff(bar, A_BOLD | COLOR_PAIR(WHITE_WHITE));
			}
		}

		sprintf(percstr, "%3d%%", perc);
		wmove(bar, 1, ((cols-6)/2 - 2) );
		for (i=0; i<4; i++) {
			color = ( (blue_x + 1) < ((cols-6)/2 - 2 + i) ) ?
			    BLUE_WHITE : WHITE_BLUE;
			wattron(bar, A_BOLD | COLOR_PAIR(color));
			waddch(bar, percstr[i]);
			wattroff(bar, A_BOLD | COLOR_PAIR(color));
		}

		wrefresh(widget);
		wrefresh(bar);

		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
		}
		scanf("%d", &perc);
		perc = perc < 0 ? 0 : perc;
		perc = perc > 100 ? 100 : perc;
		blue_x = (int)((perc*(cols-8))/100);
		i=2;
		wmove(widget, 1, 1);
		wclrtoeol(widget);
		while (true) {
			scanf("%s", input);
			if (strcmp(input,"EOF") == 0) {
				mainloop = false;
				break;
			}
			if (strcmp(input,"XXX") == 0)
				break;
			print_text_multiline(widget, 1, i, input, cols - 4);
			i = i + strlen(input) + 1;
			wrefresh(widget);
		}
	}

	delwin(bar);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.sleep > 0)
		sleep(conf.sleep);

	if (conf.print_size)
		dprintf(conf.output_fd, "Gauge size: %d, %d\n", rows, cols);

	return BSDDIALOG_YESOK;
}

int bar_handler(WINDOW *buttwin, int cols, int nbuttons, char **buttons,
    int *values, int selected, bool shortkey, WINDOW *bar, int sizebar, int min,
    int max, int def, int sleeptime, int fd)
{
	bool loop, buttupdate, barupdate;
	int i, input, currvalue, output, pos, color;
	float unitxpos;
	char valuestr[128];

	currvalue = def;
	sizebar = sizebar - 2;
	unitxpos = ((float)(max - min + 1))/sizebar;

	loop = buttupdate = barupdate = true;
	while(loop) {
		if (barupdate) {
			pos = (int)( ((float)(currvalue - min)) / unitxpos);
			for (i = 0; i < sizebar; i++) {
				color = i <= pos ? BLUE_BLUE : WHITE_WHITE;
				wattron(bar, A_BOLD | COLOR_PAIR(color));
				mvwaddch(bar, 1, i + 1, ' ');
				wattroff(bar, A_BOLD | COLOR_PAIR(color));
			}
			sprintf(valuestr, "%d", currvalue);
			wmove(bar, 1, sizebar/2 - strlen(valuestr)/2 + 1);
			for (i=0; i<strlen(valuestr); i++) {
				color = (pos < sizebar/2 - strlen(valuestr)/2 + i) ?
				    BLUE_WHITE : WHITE_BLUE;
				wattron(bar, A_BOLD | COLOR_PAIR(color));
				waddch(bar, valuestr[i]);
				wattroff(bar, A_BOLD | COLOR_PAIR(color));
			}
			barupdate = false;
			wrefresh(bar);
		}

		if (buttupdate) {
			draw_buttons(buttwin, cols, nbuttons, buttons, selected,
			    shortkey);
			wrefresh(buttwin);
			buttupdate = false;
		}

		input = getch();
		switch(input) {
		case 10: // Enter
			output = values[selected]; // values -> outputs
			loop = false;
			dprintf(fd, "%d", currvalue);
			break;
		case 27: // Esc
			output = BSDDIALOG_ERROR;
			loop = false;
			break;
		case '\t': // TAB
			selected = (selected + 1) % nbuttons;
			buttupdate = true;
			break;
		case KEY_LEFT:
			if (selected > 0) {
				selected--;
				buttupdate = true;
			}
			break;
		case KEY_RIGHT:
			if (selected < nbuttons - 1) {
				selected++;
				buttupdate = true;
			}
			break;
		case KEY_UP:
			if (currvalue < max) {
				currvalue++;
				barupdate = true;
			}
			break;
		case KEY_DOWN:
			if (currvalue > min) {
				currvalue--;
				barupdate = true;
			}
			break;
		}
	}

	sleep(sleeptime);

	return output;
}

int bsddialog_rangebox(struct config conf, char* text, int rows, int cols, int min, int max, int def)
{
	WINDOW *widget, *button, *bar, *shadow;
	char*buttons[4];
	int nbuttons, defbutton, values[4];

	if (conf.shadow) {
		shadow = newwin(rows, cols+1, conf.y+1, conf.x+1);
		wbkgd(shadow, COLOR_PAIR(BLACK_BLACK));
		wrefresh(shadow);
	}

	widget = new_window(conf.y, conf.x, rows, cols, conf.title, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	print_text_multiline(widget, 1, 2, text, cols - 4);
	bar = new_window(conf.y + rows - 6, conf.x +7, 3, cols-14, NULL, NULL, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, false, false);
	button = new_window(conf.y + rows -3, conf.x, 3, cols, NULL, conf.hline, BLACK_WHITE,
	    conf.no_lines ? NOLINES : RAISED, conf.ascii_lines, true, false);

	get_buttons(&nbuttons, buttons, values, ! conf.no_ok, conf.ok_label,
	conf.extra_button, conf.extra_label, ! conf.no_cancel, conf.cancel_label,
	conf.help_button, conf.help_label, conf.defaultno, &defbutton);

	wrefresh(widget);

	bar_handler(button, cols, nbuttons, buttons, values, defbutton, true,
	    bar, cols-14, min, max, def, conf.sleep, conf.output_fd);

	delwin(button);
	delwin(bar);
	delwin(widget);
	if (conf.shadow)
		delwin(shadow);

	if (conf.sleep > 0)
		sleep(conf.sleep);

	if (conf.print_size)
		dprintf(conf.output_fd, "Gauge size: %d, %d\n", rows, cols);

	return BSDDIALOG_YESOK;
}