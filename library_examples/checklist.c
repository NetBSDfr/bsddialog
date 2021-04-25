/*-
 * SPDX-License-Identifier: CC0-1.0
 *
 * Written in 2021 by Alfonso Sabato Siciliano.
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty, see:
 *   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <dialog.h>
#include <stdio.h>

/* cc ../theme.c ../libdialog checklist.c -o checklist -lform -lncurses */
int main()
{
	int i, output;
	struct bsddialog_config conf;
	struct bsddialog_menuitem items[5] = {
	    {"Name 1", "Desc 1", false, "Bottom Desc 1"},
	    {"Name 2", "Desc 2", false, "Bottom Desc 2"},
	    {"Name 3", "Desc 3", false, "Bottom Desc 3"},
	    {"Name 4", "Desc 4", false, "Bottom Desc 4"},
	    {"Name 5", "Desc 5", false, "Bottom Desc 5"}
	}

	conf.title = "checklist";
	
	if (bsddialog_init(conf) < 0)
		return -1;

	output = bsddialog_checklist(conf, "Checklist Example", 20, 40, 5, 5,
	    &items, true);

	bsddialog_end();

	printf("Checklist:\n");
	for (i=0; i<5; i++)
		printf(" [%c] %s\n", items[i] = items[i].on ? 'X' : ' ', items[i].name);
		
	
	return output;
}