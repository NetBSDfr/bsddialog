#!/bin/sh

dialog --insecure --title mixedform --mixedform "Hello World!" 20 50 10 \
	Label1:	1	1	Value1		1	9	10	15	0 \
	Label2:	2	1	Value2		2	9	10	15	1 \
	Label3:	3	1	Value3		3	9	10	15	2 \
	Label4:	4	1	Value4		4	9	10	15	3 \
	Label5:	5	1	Value5		5	9	10	15	4 \
	2>out.txt ; cat out.txt ; rm out.txt