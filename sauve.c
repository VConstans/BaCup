#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "buffer.h"
#include "fctListe.h"
//#include "scanner.h"
//#include "analyser.h"

struct bufferDossier* bufferDossier=NULL;
const char* destination=NULL;

void* scanner(void* mut)
{
	struct bufferDossier* cheminCourant=extractBuff(bufferDossier);
}

void* analyser(void* mut)
{
}



int main(int argc,char* argv[])
{
	//TODO controle des arguments

	int nbScanner=2;
	int nbAnalyser=2;

	struct bufferDossier* racine=creerMaillon(argv[1]);
	addBuff(racine,bufferDossier);

	//TODO verifier retour malloc
	pthread_t* tidScanner=(pthread_t*)malloc(nbScanner*sizeof(pthread_t));
	pthread_t* tidAnalyser=(pthread_t*)malloc(nbAnalyser*sizeof(pthread_t));

	pthread_mutex_t mutScanner;
	pthread_mutex_t mutAnalyser;

	if(pthread_mutex_init(&mutScanner,NULL)!=0)
	{
		perror("Erreur creation mutex Scanner");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&mutAnalyser,NULL)!=0)
	{
		perror("Erreur creation mutex analyser");
		exit(EXIT_FAILURE);
	}

	int i;
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_create(&tidScanner[i],NULL,scanner,&mutScanner)!=0)
		{
			perror("Erreur création de thread scanneur");
			exit(EXIT_FAILURE);
		}
	}

	int j;
	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_create(&tidAnalyser[i],NULL,analyser,&mutAnalyser)!=0)
		{
			perror("Erreur creation de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}

	//TODO arreter thread

	return 0;
}
