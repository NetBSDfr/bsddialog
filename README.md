# BSDDialog

The **bsddialog** utility build UI Windgets in a terminal; the interface is
inspired by the **dialog** project (https://invisible-island.net/dialog/),
however **bsddialog** is released under the terms of the *BSD-2-Clause License*,
see LICENSE.

**Getting Started**

```
% git clone https://gitlab.com/alfix/bsddialog.git
% cc bsddialog.c -o bsddialog -lncurses
% ./bsddialog --title BSD --msgbox "Hello World!" 6 24
```
Output:

![screenshot](screenshot.png)  


