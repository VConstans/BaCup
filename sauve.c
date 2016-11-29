#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "buffer.h"
#include "fctListe.h"
#include "scanner.h"
#include "analyser.h"


struct argument
{
	char* destination;
	pthread_mutex_t mut_scanner;
	pthread_cond_t cond_scanner;
	pthread_mutex_t mut_analyser;
	pthread_cond_t cond_analyser;

};


struct bufferDossier* bufferDossier=NULL;
struct bufferFichier* bufferFichier=NULL;
int scannerActif=0;






int main(int argc,char* argv[])
{
	//TODO controle des arguments

	int nbScanner=1;
	int nbAnalyser=2;


	//Initialisation du buffer de dossier
	int tailleBufferFichier=10;	//TODO changer

	struct maillon* racine=creerMaillonDossier(argv[1]);
	bufferDossier=(struct bufferDossier*)malloc(sizeof(struct bufferDossier));
	bufferDossier->dernier=NULL;
	bufferDossier->liste=NULL;
	addBuffDossier(racine,bufferDossier);


	//Initialisation du buffer de fichier
	bufferFichier=(struct bufferFichier*)malloc(sizeof(struct bufferFichier));
	bufferFichier->chemin=(char**)malloc(tailleBufferFichier*sizeof(char*));
	bufferFichier->taille=tailleBufferFichier;
	bufferFichier->idxLecteur=0;
	bufferFichier->idxEcrivain=0;
	bufferFichier->interIdx=0;

	//TODO verifier retour malloc
	pthread_t* tidScanner=(pthread_t*)malloc(nbScanner*sizeof(pthread_t));
	pthread_t* tidAnalyser=(pthread_t*)malloc(nbAnalyser*sizeof(pthread_t));


	//Initialisation des arguments à passer aux threads
	struct argument arg;

	if(pthread_mutex_init(&arg.mut_scanner,NULL)!=0)
	{
		perror("Erreur creation mutex Scanner");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&arg.mut_analyser,NULL)!=0)
	{
		perror("Erreur creation mutex analyser");
		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&arg.cond_scanner,NULL)!=0)
	{
		perror("Erreur creation condition scanner");
		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&arg.cond_analyser,NULL)!=0)
	{
		perror("Erreur creation condition analyser");
		exit(EXIT_FAILURE);
	}

	arg.destination=argv[2];

	int i;
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_create(&tidScanner[i],NULL,scanner,&arg)!=0)
		{
			perror("Erreur création de thread scanneur");
			exit(EXIT_FAILURE);
		}
	}

	int j;
	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_create(&tidAnalyser[i],NULL,analyser,&arg)!=0)
		{
			perror("Erreur creation de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}

	//TODO arreter thread
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_join(tidScanner[i],NULL)!=0)
		{
			perror("Erreur terminaison de thread scanneur");
			exit(EXIT_FAILURE);
		}
	}

	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_join(tidAnalyser[i],NULL)!=0)
		{
			perror("Erreur terminaison de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}


	return 0;
}








void executionScanner(struct maillon* dossierTraiter,char* dest)
{
	//TODO tester resultat
	DIR* dossier=opendir(dossierTraiter->chemin);
	struct dirent* entree=(struct dirent*)malloc(sizeof(struct dirent));

	while((entree=readdir(dossier))!=NULL)
	{
		if(strcmp(entree->d_name,".")!=0 && strcmp(entree->d_name,"..")!=0)
		{
			struct stat* info=(struct stat*)malloc(sizeof(struct stat));


			int lgAncienChemin=strlen(dossierTraiter->chemin);
			int lgPrefixeDest=strlen(dest);
			int lgNom=strlen(entree->d_name);
//TODO a changer par fprintf
			char* newCheminSrc=(char*)malloc(lgAncienChemin + lgNom +2);
			sprintf(newCheminSrc,"%s/%s",dossierTraiter->chemin,entree->d_name);


			if(stat(newCheminSrc,info)==-1)
			{
				perror("Erreur stat");
				exit(EXIT_FAILURE);
			}


			if(S_ISREG(info->st_mode)!=0)
			{
				//TODO lock buffer fichier
				addBuffFichier(newCheminSrc);
				printf("Ajout bufferfichier\n");
			}
			if(S_ISDIR(info->st_mode)!=0)
			{

				char* newPath=(char*)malloc(lgPrefixeDest + lgAncienChemin + lgNom + 3);
				sprintf(newPath,"%s/%s/%s",dest,dossierTraiter->chemin,entree->d_name);

			//	mkdir(newPath,info->st_mode);
				printf("Copie %s\n",newPath);
	
				struct maillon* newDossier=creerMaillonDossier(newCheminSrc);
				addBuffDossier(newDossier,bufferDossier);
			}
		}
	}
	
}



void* scanner(void* arg)
{
	struct argument* argument=(struct argument*)arg;

	pthread_mutex_lock(&argument->mut_scanner);
	while(1)
	{
		while(bufferDossier->liste==NULL && scannerActif!=0)
		{
			pthread_cond_wait(&argument->cond_scanner,&argument->mut_scanner);
		}
		if(bufferDossier->liste==NULL && scannerActif==0)
		{
			pthread_mutex_unlock(&argument->mut_scanner);
			pthread_cond_broadcast(&argument->cond_scanner);
			pthread_exit(NULL);
		}
		else
		{
			scannerActif++;

			struct maillon* extrait;
			extrait=extractBuffDossier(bufferDossier);

			pthread_mutex_unlock(&argument->mut_scanner);

			executionScanner(extrait,argument->destination);

			pthread_mutex_lock(&argument->mut_scanner);

			scannerActif--;
			pthread_cond_broadcast(&argument->cond_scanner);	//XXX utile?
			//TODO reveiller analyser?
		}
	}
}

void* analyser(void* arg)
{
	struct argument* argument=(struct argument*) arg;

	//TODO lock bufferDossier
	//TODO lock bufferFichier
	while(1)
	{
		while(bufferFichier->interIdx==0 && scannerActif!=0)
		{
			pthread_cond_wait(&argument->cond_analyser);
		}

		if(bufferFichier->interIdx==0 && scannerActif==0)
		{
			//TODO fin
		}
		else
		{
			extractBuffFichier(...);
			pthread_mutex_unlock(&argument->mut_analyser)
			executionAnalyser(...);

			pthread_mutex_lock(&argument->mut_analyser);
			
		}
	}
}
