#!/bin/sh
#-
# SPDX-License-Identifier: CC0-1.0
#
# Written in 2021 by Alfonso Sabato Siciliano.
# To the extent possible under law, the author has dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY WARRANTY, SEE:
#     <http://creativecommons.org/publicdomain/zero/1.0/>.

input="A B C D E F G H"
total=`echo $input | awk '{print split($0, a)}'`
curr=1
for i in $input
do
	perc="$(expr $(expr $curr "*" 100 ) "/" $total )"
	curr=`expr $curr + 1`
	./bsddialog --sleep 1 --title " mixedgauge " --mixedgauge "Example" 25 50  $perc \
		"Hidden"     " -9"  \
		"Label  1"   " -1"  \
		"Label  2"   " -2"  \
		"Label  3"   " -3"  \
		"Label  4"   " -4"  \
		"Label  5"   " -5"  \
		"Label  6"   " -6"  \
		"Label  7"   " -7"  \
		"Label  8"   " -8"  \
		"Label  9"   " -10" \
		"Label 10"   " -11" \
		"Label  X"   $perc
	#sleep 1
done

