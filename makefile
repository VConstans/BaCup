sauve: sauve.c fctListe scanner.h analyser.h argument.h
	gcc -Wall -Wextra -g -pthread sauve.c fctListe.o -o sauve

#scanner: buffer.h scanner.h scanner.c
#	gcc -c scanner.c

#analyser: buffer.h analyser.h analyser.c
#	gcc -c analyser.c

fctListe: buffer.h fctListe.h fctListe.c
	gcc -Wall -Wextra -g -c fctListe.c


clean:
	rm *.o sauve
