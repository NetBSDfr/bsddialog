#!/bin/sh
#-
# SPDX-License-Identifier: CC0-1.0
#
# Written in 2021 by Alfonso Sabato Siciliano.
# To the extent possible under law, the author has dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY WARRANTY, SEE:
#     <http://creativecommons.org/publicdomain/zero/1.0/>.

characters="A B C D E F G"
total=`echo $characters | awk '{print split($0, a)}'`
i=1
for c in $characters
do
	sleep 1
	echo XXX
	echo "$(expr $(expr $i "*" 100 ) "/" $total )"
	echo "[$i/$total] Input: $c"
	echo XXX
	if [ $i -eq $total ]
	then
		echo EOF
	fi
	i=`expr $i + 1`
done | ./bsddialog --title " gauge " --gauge "[0/$total] Starting..." 10 70

