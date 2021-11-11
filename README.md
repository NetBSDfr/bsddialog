# BSDDialog

**Work In Progress!**

This project provides **bsddialog** and **libbsddialog**, an utility and a
library to build scripts and tools with *TUI Widgets*.

Description:
<https://www.freebsd.org/status/report-2021-04-2021-06/#_bsddialog_tui_widgets>


**Getting Started**

FreeBSD:

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make
% ./bsddialog --msgbox "Hello World!" 8 20
```

If you are using XFCE install 
[devel/ncurses](https://www.freshports.org/devel/ncurses/)

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

Output:

![screenshot](screenshot.png)


Examples utility:
```
% ./bsddialog --title msgbox --msgbox "Hello World!" 5 30
% ./bsddialog --title yesno --yesno "Hello World!" 5 30
% ./bsddialog --inputbox "Hello World!" 10 30 2>input.txt; cat input.txt; rm input.txt
% ./bsddialog --backtitle "BSD-2-Clause License" --begin 5 5 --title yesno --hline "bsddialog" --yesno "Hello World!" 5 25
% ./bsddialog --ascii-lines --backtitle "BSD-2-Clause License" --title "yesno widget" --extra-button --help-button --defaultno --yesno "Hello World!" 10 50
% ./bsddialog --pause "Hello World!" 8 50 5
```

Examples library:
```
% cd library_examples
% sh compile
% ./buildlist
% ./checklist
% ./menu
% ./radiolist
% ./treeview
```

 - Common Options:
 
--ascii-lines, --backtitle *backtitle*, --begin *y x* (add --starty --startx?),
--cancel-label *string*, -clear (test with multiple widgets), --colors,
--date-format *format*, --defaultno, --default-item *string*, 
--exit-label *string*, --extra-button, --extra-label *string*, --help-button,
--help-label *string*, --help-status, --help-tags, --hline *string*,
--item-help, --no-cancel, --nocancel, --no-label *string*, --no-lines, --no-ok,
--nook, --no-shadow, --ok-label *string*, --output-separator *string*,
--print-version, --print-size (todo move lib -> utility),
--quoted (quotes all != dialog), --shadow,
--single-quoted (add --quote-with *ch*?), 
--separator *string* (alias --output-separator *string*),
--separate-output (rename --separate-output-withnl?), --sleep *secs*, --stderr,
--stdout, --theme *string* ("default", "dialog" and "magenta"),
--time-format *format*, --title *title*, --version, --yes-label *string*.

 - Widgets:
 
 infobox (Do not clear the screen), msgbox,
 yesno (dialog renames "yes/no" -> "ok/cancel" with --extra-button --help-button).


**TODO**

 * fix: % bsddialog --hline "bottom line" title "test title" --infobox test 8 40


 - Common Options:

|  Option                      | Status      | Note                            |
| ---------------------------- | ----------- | ------------------------------- |
| --aspect *ratio*             |             |                                 |
| --column-separator *string*  |             |                                 |
| --cr-wrap                    | Coding      |                                 |
| --create-rc *file*           |             |                                 |
| --default-button *string*    |             |                                 |
| --help                       | In progress |                                 |
| --hfile *filename*           | In progress | implemented via textbox         |
| --ignore                     | In progress | todo --ignore -error            |
| --input-fd *fd*              |             |                                 |
| --insecure                   |             |                                 |
| --iso-week                   |             |                                 |
| --keep-tite                  |             |                                 |
| --keep-window                |             |                                 |
| --last-key                   |             |                                 |
| --max-input *size*           |             |                                 |
| --no-collapse                | Coding      |                                 |
| --no-items                   |             | useful?                         |
| --no-kill                    |             |                                 |
| --no-mouse                   |             |                                 |
| --no-nl-expand               | Coding      |                                 |
| --no-tags                    |             | useful?                         |
| --output-fd *fd*             | In progress |                                 |
| --print-maxsize              | In progress |                                 |
| --print-text-only *str h w*  |             | useful?                         |
| --print-text-size *str h w*  |             | useful?                         |
| --reorder                    |             |                                 |
| --scrollbar                  |             |                                 |
| --separate-widget *string*   |             |                                 |
| --size-err                   |             |                                 |
| --tab-correct                |             |                                 |
| --tab-len *n*                |             |                                 |
| --timeout *secs*             |             |                                 |
| --trace *filename*           |             |                                 |
| --week-start *day*           |             |                                 |
| --trim                       | Coding      |                                 |
| --visit-items                |             |                                 |



 - Widgets:

| Widget         | Status      | Note                                          |
|--------------- | ----------- | ----------------------------------------------|
| --buildlist    | In progress | todo autosize, resize, F1                     |
| --calendar     | In progress | todo autosize, resize, F1, leap year, year <=0, month days |
| --checklist    | In progress | todo autosize, resize, F1                     |
| --dselect      |             |                                               |
| --editbox      |             |                                               |
| --form         | In progress | implemented via --mixedform                   |
| --fselect      |             |                                               |
| --gauge        | In progress |                                               |
| --inputbox     | In progress | implemented via --mixedform, todo \<init\>    |
| --inputmenu    |             |                                               |
| --menu         | In progress | todo autosize, resize, F1, todo scrolling     |
| --mixedform    | In progress | todo autosize, resize, F1                     |
| --mixedgauge   | In progress | todo autosize, resize, F1                     |
| --mixedlist    | In progress | Can implement dialog4ports(1)                 |
| --passwordbox  | In progress | implemented via --mixedform, todo \<init\>    |
| --passwordform | In progress | implemented via --mixedform                   |
| --pause        | In progress | todo autosize, resize, F1                     |
| --prgbox       | In progress | add command opts                              |
| --programbox   | Coding      |                                               |
| --progressbox  |             |                                               |
| --radiolist    | In progress | todo autosize, resize, F1                     |
| --rangebox     | In progress | todo autosize, resize, F1, PAGE-UP/PAGE-DOWN/HOME/END keys |
| --tailbox      | Coding      | todo via --textbox or fseek?                  |
| --tailboxbg    |             |                                               |
| --textbox      | In progress | todo navigation keys                          |
| --timebox      | In progress | todo autosize, resize, F1                     |
| --treeview     | In progress | todo autosize, resize, F1                     |

