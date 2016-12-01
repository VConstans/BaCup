#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include "buffer.h"
#include "fctListe.h"
#include "scanner.h"
#include "analyser.h"
#include "argument.h"




struct bufferDossier* bufferDossier=NULL;
struct bufferFichier* bufferFichier=NULL;
int scannerActif=0;






int main(int argc,char* argv[])
{
	//TODO controle des arguments
//	char* argument;

	struct argument arg;

	extern char* optarg;
	extern int optind, opterr;
/*	while((argument=getopt(argc,argv,"ns:a:f:")))
	{
		switch(argument)
		{
			case 'n':
				if()
				arg.verbeux=
		}
	}
*/
	int nbScanner=5;
	int nbAnalyser=5;


	//Initialisation du buffer de dossier
	int tailleBufferFichier=1;	//TODO changer

	struct maillon* racine=creerMaillonDossier(".");
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

	if(pthread_mutex_init(&arg.mut_compt,NULL)!=0)
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

	arg.source=argv[1];	//TODO changer
	arg.destination=argv[2];
	arg.sauvegarde=argv[3];
	arg.incremental=0;

	struct stat statSource;
	if(stat(arg.source,&statSource)!=0)
	{
		perror("Erreur stat racine");
		exit(EXIT_FAILURE);
	}

	if(mkdir(arg.destination,statSource.st_mode)!=0)
	{
		perror("Erreur creation dossier racine");
		exit(EXIT_FAILURE);
	}


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
		if(pthread_create(&tidAnalyser[j],NULL,analyser,&arg)!=0)
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
		if(pthread_join(tidAnalyser[j],NULL)!=0)
		{
			perror("Erreur terminaison de thread analyseur");
			exit(EXIT_FAILURE);
		}
	}


	free(tidScanner);
	free(tidAnalyser);

	free(bufferDossier);
	free(bufferFichier->chemin);
	free(bufferFichier);


	return 0;
}








void executionScanner(struct maillon* dossierTraiter,struct argument* arg)
{
	int lgPrefixeSource=strlen(arg->source);
	int lgAncienChemin=strlen(dossierTraiter->chemin);
	int lgPrefixeDest=strlen(arg->destination);

	char* cheminSource=(char*)malloc(lgPrefixeSource + lgAncienChemin + 2);
	sprintf(cheminSource,"%s/%s",arg->source,dossierTraiter->chemin);
	int lgcheminSource=strlen(cheminSource);	//XXX peut changer car connait la taille

	//TODO tester resultat
	DIR* dossier=opendir(cheminSource);
	struct dirent* entree=(struct dirent*)malloc(sizeof(struct dirent));

	while((entree=readdir(dossier))!=NULL)
	{
		if(strcmp(entree->d_name,".")!=0 && strcmp(entree->d_name,"..")!=0)
		{
			struct stat* info=(struct stat*)malloc(sizeof(struct stat));

			int lgNom=strlen(entree->d_name);
			char* newCheminSrc=(char*)malloc(lgcheminSource + lgNom +2);
			sprintf(newCheminSrc,"%s/%s",cheminSource,entree->d_name);


			if(stat(newCheminSrc,info)==-1)
			{
				perror("Erreur stat");
				exit(EXIT_FAILURE);
			}


			if(S_ISREG(info->st_mode)!=0)
			{
				char* newSuffixeSrc=(char*)malloc(lgAncienChemin + lgNom +2);
				sprintf(newSuffixeSrc,"%s/%s",dossierTraiter->chemin,entree->d_name);

				addBuffFichier(newSuffixeSrc,bufferFichier,arg);
				free(newSuffixeSrc);
			}
			if(S_ISDIR(info->st_mode)!=0)
			{

				char* suffixeDossierSuivant=(char*)malloc(lgAncienChemin + lgNom + 2);
				sprintf(suffixeDossierSuivant,"%s/%s",dossierTraiter->chemin,entree->d_name);
				int lgSuffixeDossierSuivant=strlen(suffixeDossierSuivant);

				char* cheminDossierSuivant=(char*)malloc(lgSuffixeDossierSuivant + lgPrefixeDest + 2);
				sprintf(cheminDossierSuivant,"%s/%s",arg->destination,suffixeDossierSuivant);

				if(arg->verbeux==0)
				{
					mkdir(cheminDossierSuivant,info->st_mode);
					struct maillon* maillonDossierSuivant=creerMaillonDossier(suffixeDossierSuivant);
					pthread_mutex_lock(&arg->mut_scanner);
					addBuffDossier(maillonDossierSuivant,bufferDossier);
					pthread_mutex_unlock(&arg->mut_scanner);
				}
				else
				{
					printf("Copie dossier %s",suffixeDossierSuivant);
				}

				free(suffixeDossierSuivant);
				free(cheminDossierSuivant);
			}

			free(newCheminSrc);
			free(info);
		}
	
	}

	closedir(dossier);
	free(entree);
	free(cheminSource);
}


void* scanner(void* arg)
{
	struct argument* argument=(struct argument*)arg;

	pthread_mutex_lock(&argument->mut_scanner);
	pthread_mutex_lock(&argument->mut_compt);
	while(1)
	{
		while(bufferDossier->liste==NULL && scannerActif!=0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_wait(&argument->cond_scanner,&argument->mut_scanner);
			pthread_mutex_lock(&argument->mut_compt);
		}
		if(bufferDossier->liste==NULL && scannerActif==0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_broadcast(&argument->cond_scanner);
			pthread_mutex_unlock(&argument->mut_scanner);
			pthread_exit(NULL);
		}
		else
		{
			scannerActif++;
			pthread_mutex_unlock(&argument->mut_compt);

			struct maillon* extrait;
			extrait=extractBuffDossier(bufferDossier);

			pthread_mutex_unlock(&argument->mut_scanner);

			executionScanner(extrait,argument);

			pthread_mutex_lock(&argument->mut_scanner);

			pthread_mutex_lock(&argument->mut_compt);
			rmMaillonDossier(extrait);
			scannerActif--;
			pthread_cond_broadcast(&argument->cond_scanner);	//XXX utile?
			pthread_cond_broadcast(&argument->cond_analyser);
		}
	}
}


/***************************************************************/

void copie(int src,int dest)
{
	char* bufferCopie[1024];
	int nbLu;

	while((nbLu=read(src,bufferCopie,1024))!=0)
	{
		write(dest,bufferCopie,nbLu);
	}
}

void copieComplete(char* src,char* dest,struct stat* statSource)
{
	//TODO tester	
	int fichierDestination;
	if((fichierDestination=open(dest,O_WRONLY|O_CREAT,statSource->st_mode))==-1)
	{
		perror("Erreur ouverture fichier destination");
		exit(EXIT_FAILURE);
	}
	int fichierSource;
	if((fichierSource=open(src,O_RDONLY))==-1)
	{
		perror("Erreur ouverture fichier source");
		exit(EXIT_FAILURE);
	}

	
	
	copie(fichierSource,fichierDestination);

	struct utimbuf date;
	date.actime=statSource->st_atime;
	date.modtime=statSource->st_mtime;

	//TODO tester
	utime(dest,&date);

	close(fichierSource);
	close(fichierDestination);
}


void executionAnalyser(char* suffixeCheminFichier,struct argument* arg)
{
	int lgSuffixeChemin=strlen(suffixeCheminFichier);
	int lgPrefixeSource=strlen(arg->source);
	int lgPrefixeDest=strlen(arg->destination);

	char* cheminSource=(char*)malloc(lgPrefixeSource + lgSuffixeChemin + 2);
	sprintf(cheminSource,"%s/%s",arg->source,suffixeCheminFichier);

	char* cheminDestination=(char*)malloc(lgPrefixeDest + lgSuffixeChemin + 2);
	sprintf(cheminDestination,"%s/%s",arg->destination,suffixeCheminFichier);

	struct stat statSource;
	if(stat(cheminSource,&statSource)!=0)
	{
		perror("Erreur stat source");
		exit(EXIT_FAILURE);
	}


	if(arg->incremental==1)
	{
		int lgPrefixeSauvegarde=strlen(arg->sauvegarde);
		char* cheminSauvegarde=(char*)malloc(lgSuffixeChemin + lgPrefixeSauvegarde + 2);
		sprintf(cheminSauvegarde,"%s/%s",arg->sauvegarde,suffixeCheminFichier);

		struct stat statSauvegarde;
	
		if(access(cheminSauvegarde,F_OK)!=0)
		{
			copieComplete(cheminSource,cheminDestination,&statSource);
		}
		else
		{

			if(stat(cheminSauvegarde,&statSauvegarde)!=0)
			{
				perror("Erreur fstat sauvegarde");
				exit(EXIT_FAILURE);
			}
	
			if(statSource.st_size == statSauvegarde.st_size && statSource.st_mtime == statSauvegarde.st_mtime && statSource.st_mode == statSauvegarde.st_mode)
			{
				if(arg->verbeux==0)
				{
					if(link(cheminSauvegarde,cheminDestination)!=0)
					{
						perror("Erreur link");
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					printf("Création lien %s vers %s\n",cheminDestination,cheminSauvegarde);
				}
			}
			else
			{
				copieComplete(cheminSource,cheminDestination,&statSource);
			}
		}

		free(cheminSource);
	}
	else
	{
		copieComplete(cheminSource,cheminDestination,&statSource);
	}

	free(cheminSource);
	free(cheminDestination);
}


void* analyser(void* arg)
{
	struct argument* argument=(struct argument*) arg;

	
	pthread_mutex_lock(&argument->mut_analyser);
	pthread_mutex_lock(&argument->mut_compt);
	while(1)
	{
		while(bufferFichier->interIdx==0 && scannerActif!=0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_wait(&argument->cond_analyser,&argument->mut_analyser);
			pthread_mutex_lock(&argument->mut_compt);
		}

		if(bufferFichier->interIdx==0 && scannerActif==0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_broadcast(&argument->cond_analyser);
			pthread_mutex_unlock(&argument->mut_analyser);
			pthread_exit(NULL);
		}
		else
		{
			pthread_mutex_unlock(&argument->mut_compt);
			char* extrait=extractBuffFichier(bufferFichier,argument);
			pthread_cond_broadcast(&argument->cond_analyser);
			pthread_mutex_unlock(&argument->mut_analyser);
			executionAnalyser(extrait,arg);

			pthread_mutex_lock(&argument->mut_analyser);
			pthread_mutex_lock(&argument->mut_compt);
			free(extrait);
		}
	}
}
