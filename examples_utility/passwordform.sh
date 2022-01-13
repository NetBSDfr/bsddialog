#!/bin/sh
#-
# SPDX-License-Identifier: CC0-1.0
#
# Written in 2021 by Alfonso Sabato Siciliano.
# To the extent possible under law, the author has dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY WARRANTY, SEE:
#     <http://creativecommons.org/publicdomain/zero/1.0/>.

./bsddialog --insecure --title " passwordform " --passwordform "Example" 12 40 5 \
	Password1:  1  1  ""  1  12  18  25 \
	Password2:  2  1  ""  2  12  18  25 \
	Password3:  3  1  ""  3  12  18  25 \
	Password4:  4  1  ""  4  12  18  25 \
	Password5:  5  1  ""  5  12  18  25 \
	2>out.txt ; cat out.txt ; rm out.txt
