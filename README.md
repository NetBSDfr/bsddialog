# BSDDialog

**Work In Progress!**

This project provides **bsddialog** and **libbsddialog**, an utility and a
library to build scripts and tools with *TUI Widgets*.

Description:
<https://www.freebsd.org/status/report-2021-04-2021-06/#_bsddialog_tui_widgets>


## Getting Started

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
% ./bsddialog --theme default --title msgbox --msgbox "Hello World!" 5 30
% ./bsddialog --begin-y 2 --title yesno --yesno "Hello World!" 5 30
% ./bsddialog --ascii-lines --pause "Hello World!" 8 50 5
% ./bsddialog --checklist "Space to select" 0 0 0 Name1 Desc1 off Name2 Desc2 on Name3 Desc3 off
% ./bsddialog --backtitle "TITLE" --title yesno --hline "bsddialog" --yesno "Hello World!" 5 25
% ./bsddialog --extra-button --help-button --defaultno --yesno "Hello World!" 0 0
```

Examples library:
```
% cd examples_library
% sh compile
% ./buildlist
% ./datebox
% ./form
% ./infobox
% ./menu
% ./mixedlist
% ./msgbox
% ./radiolist
% ./theme
% ./treeview
% ./yesno
```

Use Cases:

 - [portconfig](https://gitlab.com/alfix/portconfig)


## Features

**Common Options:**
 
--ascii-lines, --aspect *ratio* (for infobox, msgbox and yesno),
--backtitle *backtitle*, --begin-x *x* (--begin *y y*),
(--begin *y x*), --cancel-label *string*, -clear (test with multiple widgets),
--colors, --date-format *format*, --default-button *string*, --defaultno,
--default-item *string*, 
--exit-label *string*, --extra-button, --extra-label *string*,
--hfile *filename* (for completed widgets), n--help-button,
--help-label *string*, --help-status, --help-tags, --hline *string*, --ignore,
--insecure, --item-help, --max-input *size*, --no-cancel, --nocancel,
--no-label *string*, --no-items, --no-lines, --no-ok,
--nook, --no-shadow, --no-tags, --ok-label *string*, --output-fd *fd*,
--output-separator *string*, --print-version,
--print-size (todo move lib -> utility), --quoted (quotes all != dialog),
--print-maxsize, --shadow, --single-quoted (add --quote-with *ch*?), 
--separator *string* (alias --output-separator *string*),
--separate-output (rename --separate-output-withnl?), --sleep *secs*, --stderr,
--stdout, --theme *string* ("bsddialog", "dialog", "blackwhite" and "magenta"),
--time-format *format*, --title *title*, --version, --yes-label *string*.

**Widgets:**
 
 infobox (do not clear the screen), msgbox,
 yesno (dialog renames "yes/no" -> "ok/cancel" with --extra-button --help-button).
 checklist, radiolist, menu, mixedlist, treeview, textbox, mixedgauge and
 datebox.

## TODO


**bsdinstall:**

NULL string for *text*, ~~generic buttons for sade(8),~~ complete forms,
improve --timebox, add --column-separator


**Common Options:**

|  Option                      | Status      | Note                            |
| ---------------------------- | ----------- | ------------------------------- |
| --column-separator *string*  |             | menu                            |
| --cr-wrap                    | In progress | text                            |
| --help                       | In progress | utility                         |
| --input-fd *fd*              |             | gauge                           |
| --no-collapse                | In progress | text                            |
| --no-nl-expand               | In progress | text                            |
| --tab-correct                |             | textbox (and text)              |
| --tab-len *n*                |             | textbox (and text)              |
| --trim                       | In progress | text                            |


To evaluate / Not planned in the short term / not in bsdinstall:

--create-rc *file*, --iso-week, --no-mouse, --print-text-only *str h w*,
--print-text-size *str h w*, --reorder, -scrollbar, --separate-widget *string*,
--size-err, --timeout *secs*,--trace *filename*, --visit-items,
--week-start *day*, --keep-tite, --keep-window, --last-key, --no-kill


**Widgets:**

| Widget         | Status      | Note                                          |
|--------------- | ----------- | ----------------------------------------------|
| --buildlist    | In progress | todo autosize, resize, F1                     |
| --form         | In progress | todo autosize, resize, F1                     |
| --gauge        | In progress | todo autosize                                 |
| --inputbox     | In progress | todo autosize, resize, F1                     |
| --mixedform    | In progress | todo autosize, resize, F1                     |
| --passwordbox  | In progress | todo autosize, resize, F1                     |
| --passwordform | In progress | todo autosize, resize, F1                     |
| --pause        | In progress | todo autosize, resize, F1                     |
| --prgbox       | In progress | add command opts                              |
| --programbox   | Coding      |                                               |
| --progressbox  |             |                                               |
| --rangebox     | In progress | todo autosize, resize, F1, PAGE-UP/PAGE-DOWN/HOME/END keys |
| --timebox      | In progress | todo autosize, resize, F1                     |


To evaluate / Not planned in the short term:

tailbox (textbox/fseek), tailboxbg, dselect, fselect, inputmenu, editbox,
calendar (use datebox).
