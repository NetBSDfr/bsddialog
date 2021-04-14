#!/bin/sh

./bsddialog --title mixedform --mixedform "Hello World!" 12 30 5 \
	Label1:	1	1	Value1		1	9	18	25	0 \
	Label2:	2	1	Value2		2	9	18	25	1 \
	Label3:	3	1	Value3		3	9	18	25	2 \
	Label4:	4	1	Value4		4	9	18	25	3 \
	Label5:	5	1	Value5		5	9	18	25	4 \
	2>out.txt ; cat out.txt ; rm out.txt
