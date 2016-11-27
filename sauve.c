#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "buffer.h"
#include "fctListe.h"
//#include "scanner.h"
//#include "analyser.h"


struct argument
{
	char* destination;
	pthread_mutex_t mut;
	pthread_cond_t cond;
}


struct bufferDossier* bufferDossier=NULL;




void* scanner(void* arg)
{
	struct argument* argument=(struct argument*)arg;
	//struct bufferDossier* cheminCourant=extractBuff(bufferDossier);

	do
	{
		pthread_mutex_lock(&argument->mut);
		struct bufferDossier* dossierSuivant=extractBuff(bufferDossier);
		if(dossierSuivant==NULL)
		{
			//TODO while si jamais le braodcast vient des analyser
			pthread_cond_wait(&argument->cond,&argument->mut);
		}
		else
		{
			//TODO traitement
			pthread_cond_broadcast(&argument->cond);
		}
		pthread_mutex_unlock(&argument->mut);
	} while(bufferDossier!=NULL);
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

	struct argument argScanner;
	struct argument argAnalyser;

	if(pthread_mutex_init(&argScanner.mut,NULL)!=0)
	{
		perror("Erreur creation mutex Scanner");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&argAnalyser.mut,NULL)!=0)
	{
		perror("Erreur creation mutex analyser");
		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&argScanner.cond,NULL)!=0)
	{
		perror("Erreur creation condition scanner");
		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&argAnalyser.cond,NULL)!=0)
	{
		perror("Erreur creation condition analyser");
		exit(EXIT_FAILURE);
	}

	//TODO mettre chemin de destination dans structure

	int i;
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_create(&tidScanner[i],NULL,scanner,&argScanner)!=0)
		{
			perror("Erreur création de thread scanneur");
			exit(EXIT_FAILURE);
		}
	}

	int j;
	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_create(&tidAnalyser[i],NULL,analyser,&argAnalyser)!=0)
		{
			perror("Erreur creation de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}

	//TODO arreter thread

	return 0;
}
