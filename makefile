sauve: sauve.c fctListe
	gcc -pthread sauve.c fctListe.o -o sauve

#scanner: buffer.h scanner.h scanner.c
#	gcc -c scanner.c

#analyser: buffer.h analyser.h analyser.c
#	gcc -c analyser.c

fctListe: buffer.h fctListe.h fctListe.c
	gcc -c fctListe.c


clean:
	rm *.o sauve