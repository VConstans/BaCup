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
			if(S_ISREG(info->st_mode))
			{
				
			}
			if(S_ISDIR(info->st_mode))
			{
				int lgAncienChemin=strlen(dossierTraiter->chemin);
				int lgPrefixeDest=strlen(dest);
				int lgNom=strlen(entree->d_name);
	
				char* newPath=(char*)malloc(lgPrefixeDest + lgAncienChemin + lgNom + 3);
				memcpy(newPath,dest,lgPrefixeDest);
				newPath[lgPrefixeDest]='/';
	
				memcpy(&newPath[lgPrefixeDest+1],dossierTraiter->chemin,lgAncienChemin);
				newPath[lgPrefixeDest+1+lgAncienChemin]='/';
	
				memcpy(&newPath[lgPrefixeDest+2+lgAncienChemin],entree->d_name,lgNom);
				newPath[lgPrefixeDest+2+lgAncienChemin+lgNom]='\0';
		
				mkdir(newPath,info->st_mode);
	
				char* newCheminSrc=(char*)malloc(lgAncienChemin + lgNom +2);
				memcpy(newCheminSrc,dossierTraiter->chemin,lgAncienChemin);
					newCheminSrc[lgAncienChemin]='/';
	
				memcpy(&newCheminSrc[lgAncienChemin+1],entree->d_name,lgNom);
				newCheminSrc[lgAncienChemin+1+lgNom]='\0';
	
				struct bufferDossier* newDossier=creerMaillon(newCheminSrc);
				addBuff(newDossier,bufferDossier);
			}
		}
	}
	
}



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
			executionScanner(dossierSuivant,argument->destination);
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
