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
	pthread_mutex_t mut;
	pthread_cond_t cond;
};


struct bufferDossier* bufferDossier=NULL;
struct bufferFichier* bufferFichier=NULL;
int scannerActif=0;
int analyserActif=0;






int main(int argc,char* argv[])
{
	//TODO controle des arguments

	int nbScanner=1;
	int nbAnalyser=2;

	int tailleBufferFichier=10;

	struct maillon* racine=creerMaillonDossier(argv[1]);
	bufferDossier=(struct bufferDossier*)malloc(sizeof(struct bufferDossier));
	bufferDossier->dernier=NULL;
	bufferDossier->liste=NULL;
	addBuffDossier(racine,bufferDossier);

	bufferFichier=(struct bufferFichier*)malloc(sizeof(struct bufferFichier));
	bufferFichier->chemin=(char**)malloc(tailleBufferFichier*sizeof(char*));
	bufferFichier->taille=tailleBufferFichier;
	bufferFichier->idxLecteur=0;
	bufferFichier->idxEcrivain=0;

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

	argScanner.destination=argv[2];
	argAnalyser.destination=argv[2];

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
	//struct maillon* cheminCourant=extractBuff(bufferDossier);

	pthread_mutex_lock(&argument->mut);
	while(1)
	{
		while(bufferDossier->liste==NULL && scannerActif!=0)
		{
			pthread_cond_wait(&argument->cond,&argument->mut);
		}
		if(bufferDossier->liste==NULL && scannerActif==0)
		{
			pthread_mutex_unlock(&argument->mut);
			pthread_cond_broadcast(&argument->cond);
			pthread_exit(NULL);
		}
		else
		{
			scannerActif++;

			struct maillon* extrait;
			extrait=extractBuffDossier(bufferDossier);

			pthread_mutex_unlock(&argument->mut);

			executionScanner(extrait,argument->destination);

			pthread_mutex_lock(&argument->mut);

			scannerActif--;
		}
	}
}

void* analyser(void* arg)
{
}
