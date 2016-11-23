#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sauve.h"

struct buffer dossier;


//TODO add
//TODO rm

void* scanner(void*)
{

}

void* analyser(void*)
{

}

int main(int argc,char* argv[])
{
	//TODO controle des arguments

	int nbScanner=2;
	int nbAnalyser=2;

	struct maillon racine=creerMaillon(...);
	addBuff(racine,dossier);

	int* tidScanner=(int)malloc(nbScanner*sizeof(int));
	int* tidAnalyser=(int)malloc(nbAnalyser*sizeof(int));

	int i;
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_create(tidScanner[i],NULL,scanner,NULL)!=0)
		{
			perror("Erreur création de thread scanneur");
			exit(EXIT_FAILURE);
		}
	}

	int j;
	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_create(tidAnalyser[i],NULL,analyser,NULL)!=0)
		{
			perror("Erreur creation de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}
