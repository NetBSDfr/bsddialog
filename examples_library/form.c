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

#define HIDDEN  BSDDIALOG_ITEMHIDDEN
#define RO      BSDDIALOG_ITEMREADONLY

int main()
{
	int i, output;
	struct bsddialog_conf conf;
	char *v1, *v2, *v3, *v4, *v5, *v6, *v7, *v8;
	struct bsddialog_formitem items[4] = {
		{"Label 1:", 1, 1, "init 1", 1, 10, 50, 20, v1, v2, 0},
		{"Label 2:", 2, 1, "init 2", 2, 10, 50, 20, v3, v4, RO},
		{"Label 3:", 3, 1, "init 3", 3, 10, 50, 20, v5, v6, HIDDEN},
		{"Label 4:", 4, 1, "init 4", 4, 10, 50, 20, v7, v8, HIDDEN | RO}
	};

	bsddialog_initconf(&conf);
	conf.title = "form";
	
	if (bsddialog_init() < 0)
		return -1;

	output = bsddialog_form(conf, "Forms", 20, 50, 4, 4, items);

	bsddialog_end();
	
	if (output == BSDDIALOG_ERROR)
		printf("Error: %s", bsddialog_geterror());

	printf("Values:\n");
	for (i=0; i<4; i++)
		printf("%s|-|%s|-|%s|\n", items[i].label, items[i].newvalue1, items[i].newvalue2);
	
	return output;
}
