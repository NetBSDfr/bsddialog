/*-
 * SPDX-License-Identifier: CC0-1.0
 *
 * Written in 2021 by Alfonso Sabato Siciliano.
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty, see:
 *   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <stdio.h>
#include <string.h>

#include <bsddialog.h>
#include <theme.h>

int main()
{
	int output;
	struct bsddialog_conf conf;
	enum bsddialog_default_theme theme;
	struct bsddialog_menuitem items[4] = {
	    {false, 0, "Dialog",  "Current dialog(1) theme", "BSDDIALOG_THEME_DIALOG" },
	    {false, 0, "Next",    "Future default theme",    "BSDDIALOG_THEME_DEFAULT"},
	    {false, 0, "Magenta", "Testing",                 "BSDDIALOG_THEME_MAGENTA"},
	    {false, 0, "Quit",    "Quit or <Cancel> to exit","Quit or Cancel to exit" }
	};

	memset(&conf, 0, sizeof(struct bsddialog_conf));
	conf.y = conf.x = -1;
	conf.shadow = true;
	conf.item_help = true;
	conf.title = " Theme ";
	
	if (bsddialog_init() < 0)
		return -1;

	while (true) {
		bsddialog_backtitle(conf, "Theme Example");

		output = bsddialog_menu(conf, "Choose theme", 15, 40, 4, 4, items);

		if (output != BSDDIALOG_YESOK || items[3].on)
			break;

		if (items[0].on) {
			theme = BSDDIALOG_THEME_DIALOG;
			conf.default_item = items[0].name;
		}
		else if (items[1].on) {
			theme = BSDDIALOG_THEME_DEFAULT;
			conf.default_item = items[1].name;
		}
		else if (items[2].on) {
			theme = BSDDIALOG_THEME_MAGENTA;
			conf.default_item = items[2].name;
		}

		bsddialog_set_default_theme(theme);
	}

	bsddialog_end();	

	return output;
}