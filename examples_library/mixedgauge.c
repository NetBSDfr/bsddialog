/*-
 * SPDX-License-Identifier: CC0-1.0
 *
 * Written in 2023 by Alfonso Sabato Siciliano.
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty, see:
 *   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <bsddialog.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	int retval, i;
	struct bsddialog_conf conf;
	const char *minilabels[12] = {
		"Label  1",
		"Label  2",
		"Label  3",
		"Label  4",
		"Label  5",
		"Label  6",
		"Label  7",
		"Label  8",
		"Label  9",
		"Label 10",
		"Label 11",
		"Label  X",
	};
	int minipercs[12] = {
		BSDDIALOG_MG_SUCCEEDED,
		BSDDIALOG_MG_FAILED,
		BSDDIALOG_MG_PASSED,
		BSDDIALOG_MG_COMPLETED,
		BSDDIALOG_MG_CHECKED,
		BSDDIALOG_MG_DONE,
		BSDDIALOG_MG_SKIPPED,
		BSDDIALOG_MG_INPROGRESS,
		BSDDIALOG_MG_BLANK,
		BSDDIALOG_MG_NA,
		BSDDIALOG_MG_PENDING,
		0
	};

	if (bsddialog_init() == BSDDIALOG_ERROR) {
		printf("Error: %s\n", bsddialog_geterror());
		return (1);
	}

	bsddialog_initconf(&conf);
	conf.title = "mixedgauge";
	for (i = 0; i <= 10; i++) {
		minipercs[11] = i * 10;
		retval= bsddialog_mixedgauge(&conf, "Example", 20, 40,
		    50 + i * 5, 12, minilabels, minipercs);
    		if(retval == BSDDIALOG_ERROR) {
			bsddialog_end();
			printf("Error: %s\n", bsddialog_geterror());
			return (1);
		}
		sleep(1);
	}

	bsddialog_end();

	return (0);
}