#!/bin/sh

./bsddialog --title menu --menu "Hello World!" 12 30 6 \
	"Tag 1"	"DESC 1 xyz" \
	"Tag 2"	"DESC 2 xyz" \
	"Tag 3"	"DESC 3 xyz" \
	"Tag 4"	"DESC 4 xyz" \
	"Tag 5"	"DESC 5 xyz" \
	2>out.txt ; cat out.txt ; rm out.txt
