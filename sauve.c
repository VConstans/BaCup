#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "buffer.h"
#include "fctListe.h"
//#include "scanner.h"
//#include "analyser.h"


struct argument
{
	char* destination;
	pthread_mutex_t mut;
	pthread_cond_t cond;
};


struct bufferDossier* bufferDossier=NULL;
struct bufferFichier* bufferFichier=NULL;


void executionScanner(struct bufferDossier* dossierTraiter,char* dest)
{
	//TODO tester resultat
	DIR* dossier=opendir(dossierTraiter->chemin);
	struct dirent* entree=(struct dirent*)malloc(sizeof(struct dirent));

	while((entree=readdir(dossier))!=NULL)
	{
		if(strcmp(entree->d_name,".")!=0 && strcmp(entree->d_name,"..")!=0)
		{
			struct stat* info=(struct stat*)malloc(sizeof(struct stat));
	
			//TODO tester resultat
			stat(entree->d_name,info);

			int lgAncienChemin=strlen(dossierTraiter->chemin);
			int lgPrefixeDest=strlen(dest);
			int lgNom=strlen(entree->d_name);
//TODO a changer par fprintf
			char* newCheminSrc=(char*)malloc(lgAncienChemin + lgNom +2);
			memcpy(newCheminSrc,dossierTraiter->chemin,lgAncienChemin);
			newCheminSrc[lgAncienChemin]='/';

			memcpy(&newCheminSrc[lgAncienChemin+1],entree->d_name,lgNom);
			newCheminSrc[lgAncienChemin+1+lgNom]='\0';

			if(S_ISREG(info->st_mode))
			{
				addBuffFichier(newCheminSrc);
			}
			if(S_ISDIR(info->st_mode))
			{

			char* newPath=(char*)malloc(lgPrefixeDest + lgAncienChemin + lgNom + 3);
			memcpy(newPath,dest,lgPrefixeDest);
			newPath[lgPrefixeDest]='/';

			memcpy(&newPath[lgPrefixeDest+1],dossierTraiter->chemin,lgAncienChemin);
			newPath[lgPrefixeDest+1+lgAncienChemin]='/';

			memcpy(&newPath[lgPrefixeDest+2+lgAncienChemin],entree->d_name,lgNom);
			newPath[lgPrefixeDest+2+lgAncienChemin+lgNom]='\0';




				mkdir(newPath,info->st_mode);
	
				struct bufferDossier* newDossier=creerMaillonDossier(newCheminSrc);
				addBuffDossier(newDossier,&bufferDossier);
			}
		}
	}
	
}



void* scanner(void* arg)
{
	struct argument* argument=(struct argument*)arg;
	//struct bufferDossier* cheminCourant=extractBuff(bufferDossier);

	pthread_mutex_lock(&argument->mut);
	while(1)
	{
		while(bufferDossier==NULL && threadActif!=0)
		{
			pthread_cond_wait(...);
		}
		if(bufferDossier==NULL && threadActif==0)
		{
			//TODO unlock
			//TODO breoadcast
			pthread_exit(.....);
		}
		else
		{
			threadActif++;
			extractBuffDossier(..);
			pthread_mutex_unlock(&argument->mut);
			executionScanner(...);
			pthread_mutex_lock(&argument->mut);
			threadActif--;
		}
	}
}

void* analyser(void* mut)
{
}



int main(int argc,char* argv[])
{
	//TODO controle des arguments

	int nbScanner=2;
	int nbAnalyser=2;

	int tailleBufferFichier=10;

	struct bufferDossier* racine=creerMaillonDossier(argv[1]);
	addBuffDossier(racine,&bufferDossier);

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

	return 0;
}
