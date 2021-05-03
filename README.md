# BSDDialog

**Work In Progress!**

This project provides **bsddialog** and **libbsddialog**, an utility and a
library to build scripts and tools with a *Text User Interface Widgets*,
this project is inspired by **dialog** https://invisible-island.net/dialog.
However **bsddialog** is released under the terms of the *BSD-2-Clause License*
and the *"features synchronization"* with **dialog** is not a priority.

**Getting Started**

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make -D PORTNCURSES
% ./bsddialog --title BSD --msgbox "Hello World!" 5 24
```
Output:

![screenshot](screenshot.png)


Examples utility:
```
% bsddialog --title msgbox --msgbox "Hello World!" 5 30
% bsddialog --title yesno --yesno "Hello World!" 5 30
% bsddialog --inputbox "Hello World!" 10 30 2>input.txt; cat input.txt; rm input.txt
% bsddialog --backtitle "BSD-2-Clause License" --begin 5 5 --title yesno --hline "bsddialog" --yesno "Hello World!" 5 25
% bsddialog --ascii-lines --backtitle "BSD-2-Clause License" --title "yesno widget" --extra-button --help-button --defaultno --yesno "Hello World!" 10 50
% bsddialog --pause "Hello World!" 8 50 5
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

 * [ ] KEY\_RESIZE
 * [ ] Scrollable *text*
 * [ ] Automanic *height* and *width* (% bsddialog text 0 0)
 * [ ] Fullscreen (*height* and *width* less than 0)


 - Common Options:

|  Option                      | Status      | Note                            |
| ---------------------------- | ----------- | ------------------------------- |
| --ascii-lines                | Completed   |                                 |
| --aspect *ratio*             |             |                                 |
| --backtitle *backtitle*      | Completed   |                                 |
| --begin *y x*                | Completed   | add --starty --startx?          |
| --cancel-label *string*      | Completed   |                                 |
| --clear                      |             |                                 |
| --colors                     |             |                                 |
| --column-separator *string*  |             |                                 |
| --cr-wrap                    |             |                                 |
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
| --hfile *filename*           |             | todo via textbox                |
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
| --no-collapse                |             |                                 |
| --no-items                   |             | useful?                         |
| --no-kill                    |             |                                 |
| --no-label *string*          | Completed   |                                 |
| --no-lines                   | Completed   |                                 |
| --no-mouse                   |             |                                 |
| --no-nl-expand               |             |                                 |
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
| --theme *string*             | In progress | default|dialog|purple           |
| --time-format *format*       | Completed   |                                 |
| --timeout *secs*             |             |                                 |
| --title *title*              | Completed   |                                 |
| --trace *filename*           |             |                                 |
| --week-start *day*           |             |                                 |
| --trim                       |             |                                 |
| --version                    | Completed   |                                 |
| --visit-items                |             |                                 |
| --yes-label *string*         | Completed   |                                 |


 - Widgets:

| Widget         | Status      | Note                                          |
|--------------- | ----------- | ----------------------------------------------|
| --buildlist    | In progress |                                               |
| --calendar     | In progress | add leap year, year <=0, month days           |
| --checklist    | In progress |                                               |
| --dselect      |             |                                               |
| --editbox      |             |                                               |
| --form         | In progress | implemented via --mixedform                   |
| --fselect      |             |                                               |
| --gauge        | In progress |                                               |
| --infobox      | In progress | Do not clear the screen                       |
| --inputbox     | In progress | implemented via --mixedform, todo \<init\>    |
| --inputmenu    |             |                                               |
| --menu         | In progress | todo scrolling                                |
| --mixedform    | In progress |                                               |
| --mixedgauge   | In progress |                                               |
| --mixedlist    | In progress | Can implement [dialog4ports(1)](https://man.freebsd.org/dialog4ports/1) |
| --msgbox       | In progress |                                               |
| --passwordbox  | In progress | implemented via --mixedform, todo \<init\>    |
| --passwordform | In progress | implemented via --mixedform                   |
| --pause        | In progress |                                               |
| --prgbox       | In progress | add command opts                              |
| --programbox   | Coding      |                                               |
| --progressbox  | Coding      |                                               |
| --radiolist    | In progress |                                               |
| --rangebox     | In progress | todo PAGE-UP/PAGE-DOWN/HOME/END keys          |
| --tailbox      | Coding      | todo via --textbox or fseek?                  |
| --tailboxbg    |             |                                               |
| --textbox      | In progress | todo navigation keys                          |
| --timebox      | In progress |                                               |
| --treeview     | In progress |                                               |
| --yesno        | In progress | dialog renames yes/no -> ok/cancel with --extra-button --help-button |

