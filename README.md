# BSDDialog

**Work In Progress!**

This project provides **bsddialog** and **libbsddialog**, an utility and a
library to build scripts and tools with TUI dialogs and widgets.


## Intro

Briefly:
<https://www.freebsd.org/status/report-2021-04-2021-06/#_bsddialog_tui_widgets>

Utility:
<https://alfonsosiciliano.gitlab.io/posts/2021-12-07-bsddialog.html>

Library:
<https://alfonsosiciliano.gitlab.io/posts/2022-01-16-libbsddialog.html>

Screenshots:
<https://www.flickr.com/photos/alfonsosiciliano/albums/72157720215006074>


## Getting Started

FreeBSD:

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make
% ./bsddialog --msgbox "Hello World!" 8 20
```

Output:

![screenshot](screenshot.png)


## Utility


**Common Options:**
 
--ascii-lines, --backtitle *backtitle*, --begin-x *x*, --begin-y *y*,
--cancel-label *string*, --clear, --colors, --cr-wrap, --date-format *format*,
--defaultno, --default-button *label*, --default-no, --default-item *name*,
--disable-esc, --esc-cancelvalue, --exit-label *label*, --extra-button,
--extra-label *label*, --generic1-button *label*, --generic2-button *label*,
--help, --help-button, --help-label *label*, --help-status, --help-tags,
--hfile *filename*, --hline *string*, --hmsg *string*, --ignore, --insecure,
--item-depth, --item-help, --items-prefix, --max-input *size*, --no-cancel,
--nocancel, --no-collapse, --no-items, --no-label *label*, --no-lines,
--no-nl-expand, --no-ok, --nook, --no-shadow, --no-tags, --ok-label *label*,
--output-fd *fd*, --output-separator *sep*, --print-maxsize, --print-size,
--print-version, --quoted, --separate-output, --separator *sep*, --shadow,
--single-quoted, --sleep *secs*, --stderr, --stdout, --tab-len *spaces*,
--theme *blackwhite|bsddialog|dialog*, --time-format *format*, --title *title*,
--trim, --version, --yes-label *label*.


**Dialogs:**

--checklist, --datebox, --form, --gauge, --inputbox, --menu, --mixedform,
--mixedgauge, --msgbox, --passwordbox, --passwordform, --pause, --radiolist,
--rangebox, --textbox, --timebox, --treeview, --yesno.


**Examples**:

```
% ./bsddialog --backtitle "TITLE" --title msgbox --msgbox "Hello World!" 5 30
% ./bsddialog --theme blackwhite --title msgbox --msgbox "Hello World!" 5 30
% ./bsddialog --begin-y 2 --default-no --title yesno --yesno "Hello World!" 5 30
% ./bsddialog --ascii-lines --pause "Hello World!" 8 50 10
% ./bsddialog --checklist "Space to select" 0 0 0 Name1 Desc1 off Name2 Desc2 on
% ./bsddialog --title yesno --hline "bsddialog" --yesno "Hello World!" 5 25
% ./bsddialog --extra-button --help-button --yesno "Hello World!" 0 0
```

and [Examples](https://gitlab.com/alfix/bsddialog/-/tree/main/examples_utility)
in the _Public Domain_ to build new projects:
```
% sh ./examples_utility/checklist.sh
% sh ./examples_utility/form.sh
% sh ./examples_utility/gauge.sh
% sh ./examples_utility/infobox.sh
% sh ./examples_utility/inputbox.sh
% sh ./examples_utility/menu.sh
% sh ./examples_utility/mixedform.sh
% sh ./examples_utility/mixedgauge.sh
% sh ./examples_utility/msgbox.sh
% sh ./examples_utility/passwordbox.sh
% sh ./examples_utility/passwordform.sh
% sh ./examples_utility/radiolist.sh
% sh ./examples_utility/treeview.sh
% sh ./examples_utility/yesno.sh
```

## Library

**Manual**

 - [bsddialog(3)](https://alfonsosiciliano.gitlab.io/posts/2022-01-15-manual-libbsddialog.html)


**Examples**:

[Examples](https://gitlab.com/alfix/bsddialog/-/tree/main/examples_library)
in the _Public Domain_ to build new projects:
```
% cd examples_library
% sh compile
% ./datebox
% ./form
% ./infobox
% ./menu
% ./mixedlist
% ./msgbox
% ./pause
% ./radiolist
% ./rangebox
% ./theme
% ./timebox
% ./yesno
```
 
