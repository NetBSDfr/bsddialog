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

#define HIDDEN  BSDDIALOG_FIELDHIDDEN
#define RO      BSDDIALOG_FIELDREADONLY

int main()
{
	int i, output;
	struct bsddialog_conf conf;
	struct bsddialog_formfield fields[3] = {
		{"Input:",    1, 1, "value",     1, 11, 50, 20, 0      },
		{"Input:",    2, 1, "read only", 2, 11, 50, 20, RO     },
		{"Password:", 3, 1, "",          3, 11, 50, 20, HIDDEN }
	};

	bsddialog_initconf(&conf);
	conf.title = "form";
	
	if (bsddialog_init() < 0)
		return -1;

	output = bsddialog_form(conf, "Forms", 20, 50, 3, 3, fields);

	bsddialog_end();
	
	if (output == BSDDIALOG_ERROR)
		printf("Error: %s", bsddialog_geterror());

	printf("Values:\n");
	for (i=0; i<3; i++)
		printf("%s|-|%s|\n", fields[i].label, fields[i].value);
	
	return output;
}
