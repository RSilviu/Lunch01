build:
		gcc  cLunch01.c -o cLunch01
		gcc -pthread sLunch01.c -o sLunch01
		gcc -pthread clientLauncher.c -o clientLauncher

