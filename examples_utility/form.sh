#!/bin/sh
#-
# SPDX-License-Identifier: CC0-1.0
#
# Written in 2021 by Alfonso Sabato Siciliano.
# To the extent possible under law, the author has dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY WARRANTY, SEE:
#     <http://creativecommons.org/publicdomain/zero/1.0/>.

./bsddialog --title " form " --form "Hello World!" 12 40 5 \
	Label1:  1  1  Value1  1  9  18  25 \
	Label2:  2  1  Value2  2  9  18  25 \
	Label3:  3  1  Value3  3  9  18  25 \
	Label4:  4  1  Value4  4  9  18  25 \
	Label5:  5  1  Value5  5  9  18  25 \
	2>out.txt ; cat out.txt ; rm out.txt


