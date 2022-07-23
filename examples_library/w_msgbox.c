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
#include <string.h>

int main()
{
	int output;
	struct bsddialog_conf conf;

	setlocale(LC_ALL, "");

	if (bsddialog_init() == BSDDIALOG_ERROR) {
		printf("Error: %s\n", bsddialog_geterror());
		return (1);
	}

	bsddialog_initconf(&conf);
	conf.title = "msgbox";
	output = bsddialog_msgbox(&conf, "あいうえお 漢字", 10, 50);

	bsddialog_end();

	switch (output) {
	case BSDDIALOG_ERROR:
		printf("Error %s\n", bsddialog_geterror());
		break;
	case BSDDIALOG_OK:
		printf("OK\n");
		break;
	}

	return (output);
}