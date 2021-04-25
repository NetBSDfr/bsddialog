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

#include "../bsddialog.h"

/* cc ../theme.c ../libbsddialog.c menu.c -o menu -lform -lncurses */
int main()
{
	int i, output;
	struct config conf; //bsddialog_config conf;
	struct bsddialog_menuitem items[5] = {
	    {"Name 1", "Desc 1", false, "Bottom Desc 1"},
	    {"Name 2", "Desc 2", false, "Bottom Desc 2"},
	    {"Name 3", "Desc 3", false, "Bottom Desc 3"},
	    {"Name 4", "Desc 4", false, "Bottom Desc 4"},
	    {"Name 5", "Desc 5", false, "Bottom Desc 5"}
	};

	memset(&conf, 0, sizeof(struct config));
	conf.y = conf.x = -1;
	conf.shadow = true;
	conf.item_help = true;
	conf.cancel_label = "Quit";
	conf.ok_label = "Submit";
	conf.title = "menu";
	
	if (bsddialog_init() < 0)
		return -1;

	output = bsddialog_menu(conf, "Example", 15, 30, 5, 5, items);

	bsddialog_end();

	printf("Menu:\n");
	for (i=0; i<5; i++)
		printf(" [%c] %s\n", items[i].on ? 'X' : ' ', items[i].name);
		
	
	return output;
}