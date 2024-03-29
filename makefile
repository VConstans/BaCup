sauve: sauve.c fctListe argument.h
	gcc -Wall -Wextra -g -pthread sauve.c fctListe.o -o sauve

fctListe: buffer.h fctListe.h fctListe.c
	gcc -Wall -Wextra -g -c fctListe.c


clean:
	rm *.o sauve

del:
	rm -r new
valgrind:
	valgrind --leak-check=full --show-leak-kinds=all ./sauve A new
