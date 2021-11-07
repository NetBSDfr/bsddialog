# BSDDialog

**Work In Progress!**

This project provides **bsddialog** and **libbsddialog**, an utility and a
library to build scripts and tools with *TUI Widgets*.

Description:
<https://www.freebsd.org/status/report-2021-04-2021-06/#_bsddialog_tui_widgets>


**Getting Started**

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make
% ./bsddialog --msgbox "Hello World!" 8 20
```
(Tip: If you are using XFCE install PORTNECURSES and build bsddialog by
*% make -D PORTNCURSES*.


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

**TODO**

 * fix: % bsddialog --hline "bottom line" title "test title" --infobox test 8 40


 - Common Options:

|  Option                      | Status      | Note                            |
| ---------------------------- | ----------- | ------------------------------- |
| --ascii-lines                | Completed   |                                 |
| --aspect *ratio*             |             |                                 |
| --backtitle *backtitle*      | Completed   |                                 |
| --begin *y x*                | Completed   | add --starty --startx?          |
| --cancel-label *string*      | Completed   |                                 |
| --clear                      | Completed   | test with multiple widgets      |
| --colors                     | Completed   |                                 |
| --column-separator *string*  |             |                                 |
| --cr-wrap                    | Coding      |                                 |
| --create-rc *file*           |             |                                 |
| --date-format *format*       | Completed   |                                 |
| --defaultno                  | Completed   |                                 |
| --default-button *string*    |             |                                 |
| --default-item *string*      | Completed   |                                 |
| --exit-label *string*        | Completed   |                                 |
| --extra-button               | Completed   |                                 |
| --extra-label *string*       | Completed   |                                 |
| --help                       | In progress |                                 |
| --help-button                | Completed   |                                 |
| --help-label *string*        | Completed   |                                 |
| --help-status                | Completed   |                                 |
| --help-tags                  |             | example?                        |
| --hfile *filename*           | In progress | implemented via textbox         |
| --hline *string*             | Completed   |                                 |
| --ignore                     | In progress | todo --ignore -error            |
| --input-fd *fd*              |             |                                 |
| --insecure                   |             |                                 |
| --iso-week                   |             |                                 |
| --item-help                  | Coding      |                                 |
| --keep-tite                  |             |                                 |
| --keep-window                |             |                                 |
| --last-key                   |             |                                 |
| --max-input *size*           |             |                                 |
| --no-cancel                  | Completed   |                                 |
| --nocancel                   | Completed   |                                 |
| --no-collapse                | Coding      |                                 |
| --no-items                   |             | useful?                         |
| --no-kill                    |             |                                 |
| --no-label *string*          | Completed   |                                 |
| --no-lines                   | Completed   |                                 |
| --no-mouse                   |             |                                 |
| --no-nl-expand               | Coding      |                                 |
| --no-ok                      | Completed   |                                 |
| --nook                       | Completed   |                                 |
| --no-shadow                  | Completed   |                                 |
| --no-tags                    |             | useful?                         |
| --ok-label *string*          | Completed   |                                 |
| --output-fd *fd*             | In progress |                                 |
| --separator *string*         |             |                                 |
| --output-separator *string*  |             |                                 |
| --print-maxsize              | In progress |                                 |
| --print-size                 | In progress |                                 |
| --print-text-only *str h w*  |             | useful?                         |
| --print-text-size *str h w*  |             | useful?                         |
| --print-version              | Completed   |                                 |
| --quoted                     | Coding      | bsddialog (all tags) != dialog  |
| --reorder                    |             |                                 |
| --scrollbar                  |             |                                 |
| --separate-output            | In progress | for buildlist, checklist and treeview, rename --separate-out-nl?  |
| --separate-widget *string*   |             |                                 |
| --shadow                     | Completed   | useful?                         |
| --single-quoted              | Coding      | add --quote-string *string*?    |
| --size-err                   |             |                                 |
| --sleep *secs*               | Completed   |                                 |
| --stderr                     | In progress |                                 |
| --stdout                     | In progress |                                 |
| --tab-correct                |             |                                 |
| --tab-len *n*                |             |                                 |
| --theme *string*             | Completed   | themes: "default", "dialog" and "magenta" |
| --time-format *format*       | Completed   |                                 |
| --timeout *secs*             |             |                                 |
| --title *title*              | Completed   |                                 |
| --trace *filename*           |             |                                 |
| --week-start *day*           |             |                                 |
| --trim                       | Coding      |                                 |
| --version                    | Completed   |                                 |
| --visit-items                |             |                                 |
| --yes-label *string*         | Completed   |                                 |


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
| --infobox      | In progress | Do not clear the screen                       |
| --inputbox     | In progress | implemented via --mixedform, todo \<init\>    |
| --inputmenu    |             |                                               |
| --menu         | In progress | todo autosize, resize, F1, todo scrolling     |
| --mixedform    | In progress | todo autosize, resize, F1                     |
| --mixedgauge   | In progress | todo autosize, resize, F1                     |
| --mixedlist    | In progress | Can implement dialog4ports(1)                 |
| --msgbox       | Completed   |                                               |
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
| --yesno        | Completed   | dialog renames yes/no -> ok/cancel with --extra-button --help-button |

