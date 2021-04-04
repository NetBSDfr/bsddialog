# BSDDialog

**Under development!**

The **bsddialog** utility builds UI Widgets in a terminal, this project is
inspired by **dialog** https://invisible-island.net/dialog/, however
**bsddialog** is released under the terms of the *BSD-2-Clause License*,
see LICENSE.

**Getting Started**

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make -D PORTNCURSES
% ./bsddialog --title BSD --msgbox "Hello World!" 6 24
```
Output:

![screenshot](screenshot.png)  


Examples:
```
% bsddialog --title msgbox --msgbox "Hello World!" 5 30
% bsddialog --title yesno --yesno "Hello World!" 5 30
% bsddialog --backtitle "BSD-2-Clause License" --begin 5 5 --title yesno --hline "bsddialog" --yesno "Hello World!" 5 25
% bsddialog --ascii-lines --backtitle "BSD-2-Clause License" --title "yesno widget" --extra-button --help-button --defaultno --yesno "Hello World!" 10 50
```

**TODO**

 - Split bsddialog.c: bsddialog.c - bsddialog.h - libbsddialog.c
 - bsddialog.1 and bsddialog.3
 - Common Options:

|  Option                      | Status      | Note                            |
| ---------------------------- | ----------- | ------------------------------- |
| --ascii-lines                | Completed   |                                 |
| --aspect *ratio*             |             |                                 |
| --backtitle *backtitle*      | Completed   |                                 |
| --begin y x                  | Completed   |                                 |
| --cancel-label *string*      | Completed   |                                 |
| --clear                      |             |                                 |
| --colors                     |             |                                 |
| --column-separator *string*  |             |                                 |
| --cr-wrap                    |             |                                 |
| --create-rc *file*           |             |                                 |
| --date-format *format*       |             |                                 |
| --defaultno                  | Completed   |                                 |
| --default-button *string*    |             |                                 |
| --default-item *string*      |             |                                 |
| --exit-label *string*        | Completed   |                                 |
| --extra-button               | Completed   |                                 |
| --extra-label *string*       | Completed   |                                 |
| --help                       | In progress |                                 |
| --help-button                | Completed   |                                 |
| --help-label *string*        | Completed   |                                 |
| --help-status                |             |                                 |
| --help-tags                  |             |                                 |
| --hfile *filename*           |             |                                 |
| --hline *string*             | Completed   |                                 |
| --ignore                     |             |                                 |
| --input-fd *fd*              |             |                                 |
| --insecure                   |             |                                 |
| --item-help                  |             |                                 |
| --keep-tite                  |             |                                 |
| --keep-window                |             |                                 |
| --last-key                   |             |                                 |
| --max-input *size*           |             |                                 |
| --no-cancel                  | Completed   |                                 |
| --nocancel                   | Completed   |                                 |
| --no-collapse                |             |                                 |
| --no-items                   |             |                                 |
| --no-kill                    |             |                                 |
| --no-label *string*          | Completed   |                                 |
| --no-lines                   | Completed   |                                 |
| --no-mouse                   |             |                                 |
| --no-nl-expand               |             |                                 |
| --no-ok                      | Completed   |                                 |
| --nook                       | Completed   |                                 |
| --no-shadow                  | Completed   |                                 |
| --no-tags                    |             |                                 |
| --ok-label *string*          | Completed   |                                 |
| --output-fd *fd*             | In progress |                                 |
| --separator *string*         |             |                                 |
| --output-separator *string*  |             |                                 |
| --print-maxsize              | In progress |                                 |
| --print-size                 | In progress |                                 |
| --print-version              | Completed   |                                 |
| --quoted                     |             |                                 |
| --scrollbar                  |             |                                 |
| --separate-output            |             |                                 |
| --separate-widget *string*   |             |                                 |
| --shadow                     | Completed   |                                 |
| --single-quoted              |             |                                 |
| --size-err                   |             |                                 |
| --sleep *secs*               | Completed   |                                 |
| --stderr                     | In progress |                                 |
| --stdout                     | In progress |                                 |
| --tab-correct                |             |                                 |
| --tab-len *n*                |             |                                 |
| --time-format *format*       |             |                                 |
| --timeout *secs*             |             |                                 |
| --title *title*              | Completed   |                                 |
| --trace *filename*           |             |                                 |
| --trim                       |             |                                 |
| --version                    | Completed   |                                 |
| --visit-items                |             |                                 |
| --yes-label *string*         | Completed   |                                 |


 - Widgets:

| Widget       | Status      | Note                                            |
|------------- | ----------- | ----------------------------------------------- |
| buildlist    |             |                                                 |
| calendar     |             |                                                 |
| checklist    | In progress |                                                 |
| dselect      |             |                                                 |
| editbox      |             |                                                 |
| form         |             |                                                 |
| fselect      |             |                                                 |
| gauge        |             |                                                 |
| infobox      | In progress | Do not clear the screen                         |
| inputbox     | In progress |                                                 |
| inputmenu    |             |                                                 |
| menu         |             |                                                 |
| mixedform    |             |                                                 |
| mixedgauge   |             |                                                 |
| msgbox       | In progress |                                                 |
| passwordbox  |             |                                                 |
| passwordform |             |                                                 |
| pause        | In progress |                                                 |
| prgbox       |             |                                                 |
| programbox   |             |                                                 |
| progressbox  |             |                                                 |
| radiolist    |             |                                                 |
| rangebox     |             |                                                 |
| tailbox      |             |                                                 |
| tailboxbg    |             |                                                 |
| textbox      |             |                                                 |
| timebox      |             |                                                 |
| treeview     |             |                                                 |
| yesno        | In progress | dialof rename yes/no -> ok/cancel with --extra-button --help-button                              g                 |

