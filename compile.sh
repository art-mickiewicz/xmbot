#!/bin/sh
case `uname` in
Linux)
	gcc -ggdb -rdynamic -ldl -o xmbot src/*.c
	gcc -ggdb -o sulci.so -shared src/modules/sulci/*.c
	gcc -ggdb -o misc.so -shared src/modules/misc/*.c
	gcc -ggdb -o access.so -shared src/modules/access/*.c
;;
FreeBSD)
	gcc -D FREEBSD4 -rdynamic -L/usr/local/lib -I/usr/local/include -liconv -o xmbot src/*.c
	gcc -o sulci.so -shared src/modules/sulci/*.c
	gcc -o misc.so -shared src/modules/misc/*.c
	gcc -o access.so -shared src/modules/access/*.c
;;
esac
