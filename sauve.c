#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "buffer.h"
#include "fctListe.h"
#include "scanner.h"
#include "analyser.h"

struct bufferDossier;






int main(int argc,char* argv[])
{
	//TODO controle des arguments

	int nbScanner=2;
	int nbAnalyser=2;

	struct maillon* racine=creerMaillon(...);
	addBuff(racine,dossier);

	//TODO verifier retour malloc
	int* tidScanner=(int)malloc(nbScanner*sizeof(int));
	int* tidAnalyser=(int)malloc(nbAnalyser*sizeof(int));

	pthread_mutex_t mutScanner;
	pthread_mutes_t mutAnalyser;

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
		if(pthread_create(tidScanner[i],NULL,scanner,&mutScanner)!=0)
		{
			perror("Erreur création de thread scanneur");
			exit(EXIT_FAILURE);
		}
	}

	int j;
	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_create(tidAnalyser[i],NULL,analyser,&mutAnalyser)!=0)
		{
			perror("Erreur creation de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}

	//TODO arreter thread

	return 0;
}
