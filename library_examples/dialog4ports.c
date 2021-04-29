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

/* 
 * Actually this is an example for mixedmenu to reproduce dialog4ports(1)
 * cc ../theme.c ../libbsddialog.c dialog4ports.c -o dialog4ports -lform -lncurses
 */
int main()
{
	int i, j, output;
	struct config conf; //bsddialog_config conf;
	struct bsddialog_menuitem check[5] = {
	    { true,  0, "Name 1", "Desc 1", "+" },
	    { false, 0, "Name 2", "Desc 2", "+" },
	    { true,  0, "Name 3", "Desc 3", "+" },
	    { false, 0, "Name 4", "Desc 4", ""  },
	    { true,  0, "Name 5", "Desc 5", ""  }
	};
	struct bsddialog_menuitem sep[1] = {
	    { true, 0, "Radio", "list", "" }
	};
	struct bsddialog_menuitem radio[5] = {
	    { true,  0, "Name 1", "Desc 1", "" },
	    { false, 0, "Name 2", "Desc 2", ""  },
	    { false, 0, "Name 3", "Desc 3", ""  },
	    { false, 0, "Name 4", "Desc 4", "+" },
	    { false, 0, "Name 5", "Desc 5", "+" }
	};
	struct bsddialog_menugroup group[3] = {
	    { BSDDIALOG_PORTCHECKLIST, 5, check },
	    { BSDDIALOG_SEPARATOR,     5, sep   },
	    { BSDDIALOG_PORTRADIOLIST, 5, radio }
	};

	memset(&conf, 0, sizeof(struct config));
	conf.y = conf.x = -1;
	conf.shadow = true;
	conf.item_help = true;
	conf.title = "mixedmenu";
	
	if (bsddialog_init() < 0)
		return -1;

	output = bsddialog_mixedmenu(conf, "Example (dialog4ports)", 15, 30, 5,
	    5, items);

	bsddialog_end();

	printf("Mixedmenu (dialog4ports):\n");
	for (i=0; i<3; i++) {
		for (j=0; j<group.nitems; j++)
		if (group[i].type == BSDDIALOG_SEPARATOR)
			printf("----- %s -----\n", group[i][j].name);
		else if (group[i].type == BSDDIALOG_RADIOLIST)
			printf(" (%c) %s\n", items[i].on ? '*' : ' ', items[i].name);
		else /* BSDDIALOG_PORTCHECKLIST */
			printf(" [%c] %s\n", items[i].on ? 'X' : ' ', items[i].name);
	}
		
	
	return output;
}