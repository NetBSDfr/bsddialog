/*-
 * SPDX-License-Identifier: CC0-1.0
 *
 * Written in 2021 by Alfonso Sabato Siciliano.
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty, see:
 *   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <bsddialog.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main()
{
	int output;
	unsigned int hh, mm, ss;
	struct bsddialog_conf conf;
	time_t clock;
	struct tm *localtm;

	time(&clock);
	localtm = localtime(&clock);
	hh = localtm->tm_hour;
	mm = localtm->tm_min;
	ss = localtm->tm_sec;

	setlocale(LC_ALL, ""); /* unicode arrows insted of ^^^ and vvv */
	if (bsddialog_init() == BSDDIALOG_ERROR) {
		printf("Error: %s\n", bsddialog_geterror());
		return (1);
	}

	bsddialog_initconf(&conf);
	conf.title = "timebox";
	output = bsddialog_timebox(&conf, "Example", 9, 35, &hh, &mm, &ss);

	bsddialog_end();

	switch (output) {
	case BSDDIALOG_OK:
		printf("Time: [%u:%u:%u]\n", hh, mm, ss);
		break;
	case BSDDIALOG_CANCEL:
		printf("Cancel\n");
		break;
	case BSDDIALOG_ERROR:
		printf("Error: %s\n", bsddialog_geterror());
		break;
	}

	return (output);
}