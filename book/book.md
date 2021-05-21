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

helloworld.c

```c
#include <stdio.h>
#include <string.h>

#include <bsddialog.h>

int main()
{
	struct bsddialog_conf conf;
	
	if (bsddialog_init() < 0)
		return -1;

	memset(&conf, 0, sizeof(struct bsddialog_conf));
	bsddialog_msgbox(conf, "Hello World!", 7, 20);

	bsddialog_end();

	return 0;
}
```

**To compile**
```
% cc -I/usr/local/include helloworld.c -o helloworld -L/usr/local/lib/ -lbsddialog
% ./helloworld
```
**Output**:

## 2 Global tools

### 2.1 Starting

### 2.2 Ending

### 2.3 bsddialog\_conf

```c
struct bsddialog_conf {
};
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

