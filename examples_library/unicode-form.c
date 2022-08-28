/*-
 * SPDX-License-Identifier: CC0-1.0
 *
 * Written in 2022 by Alfonso Sabato Siciliano.
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty, see:
 *   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <bsddialog.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	int output;
	struct bsddialog_conf conf;
	struct bsddialog_formitem item = { "Label:", 0, 0, "あいうえお", 0, 6, 38,
	    50, NULL, BSDDIALOG_FIELDCURSOREND, "" };

	setlocale(LC_ALL, "");

	if (bsddialog_init() == BSDDIALOG_ERROR) {
		printf("Error: %s\n", bsddialog_geterror());
		return (1);
	}
	bsddialog_initconf(&conf);
	conf.title = "unicode form";
	conf.button.always_active = true;
	output = bsddialog_form(&conf, "Example", 10, 50, 1, 1, &item);
	bsddialog_end();

	switch (output) {
	case BSDDIALOG_ERROR:
		printf("Error: %s", bsddialog_geterror());
		break;
	case BSDDIALOG_CANCEL:
		printf("Cancel\n");
		break;
	case BSDDIALOG_OK:
		printf("%s \"%s\"\n", item.label, item.value);
		free(item.value);
		break;
	}

	return (output);
}