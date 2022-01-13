#!/bin/sh
#-
# SPDX-License-Identifier: CC0-1.0
#
# Written in 2021 by Alfonso Sabato Siciliano.
# To the extent possible under law, the author has dedicated all copyright
# and related and neighboring rights to this software to the public domain
# worldwide. THIS SOFTWARE IS DISTRIBUTED WITHOUT ANY WARRANTY, SEE:
#     <http://creativecommons.org/publicdomain/zero/1.0/>.

./bsddialog --insecure --title " password " --passwordbox "Hello World!" 12 40 \
	2>out.txt ; cat out.txt ; rm out.txt
