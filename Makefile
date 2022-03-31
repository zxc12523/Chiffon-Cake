all:
	gcc host.c -o host
	gcc player.c -o player
clear:
	rm -f host
	rm -f player