# BSDDialog Library

## 1 Intro

This project provides **bsddialog** and **libbsddialog**, an utility and a
library to build scripts and tools with a *Text User Interface Widgets*.

### 1.1 Getting started

*FreeBSD*:

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make -D PORTNCURSES
% ./bsddialog --title Message --msgbox "Hello World!" 5 24
```

*Linux*:

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cd bsddialog
% make -f GNUMakefile
% ./bsddialog --title Message --msgbox "Hello World!" 5 24
```

### 1.2 Hello World

msgbox.c

## 2 struct sddialog\_config

The struct:
```c
```

Common members:


## 3 "Info"

infobox

## 4 Button

msgbox
yesno

## 5 Menu

buildlist
checklist
menu
radiolist
treeview

## 6 Form

inputbox
passwordbox 
form
passwordform
mixedform

## 7 "Editor"

editbox(todo)

## 8 "Bar"

gauge
mixedgauge
rangebox
pause

## 9 "Time"

timebox
calendar

## 10 "Command" 

prgbox
programbox(todo)
progressbox(todo)

## 11 "Text"

tailbox
tailboxbg(todo)
textbox

## 12 "File"

dselect(todo)
fselect(todo)

## 13 Style

