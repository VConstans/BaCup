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
	int argument;

	struct argument arg;
	arg.verbeux=0;

	int nbScanner=5;
	int nbAnalyser=5;
	int tailleBufferFichier=10;

	extern char* optarg;
	extern int optind;
	while((argument=getopt(argc,argv,"ns:a:f:"))!=-1)
	{
		switch(argument)
		{
			case 'n':
				arg.verbeux=1;
			//	printf("verbeux\n");
				break;
			case 's':
				nbScanner=atoi(optarg);
			//	printf("nb scanner %d\n",atoi(optarg));
				break;
			case 'a':
				nbAnalyser=atoi(optarg);
			//	printf("nb analyser %d\n",atoi(optarg));
				break;
			case 'f':
				tailleBufferFichier=atoi(optarg);
			//	printf("taille buffer fichier %d\n",atoi(optarg));
				break;
		}
	}
	switch(argc-optind)
	{
		case 2:
			arg.source=argv[optind++];
			arg.destination=argv[optind];
		//	printf("Source %s	destination %s\n",arg.source,arg.destination);
			arg.incremental=0;
			break;
		case 3:
			arg.source=argv[optind++];
			arg.sauvegarde=argv[optind++];
			arg.destination=argv[optind];
		//	printf("Source %s	sauvegarde %s	destination %s\n",arg.source,arg.sauvegarde,arg.destination);
			arg.incremental=1;
			break;
		default:
			fprintf(stderr,"Usage: %s [-n] [-s n] [-a n] [-f n] source [precedent] destination\n",argv[0]);
			exit(EXIT_FAILURE);
	}


	//Initialisation du buffer de dossier

	struct maillon* racine=creerMaillonDossier(".");
	if((bufferDossier=(struct bufferDossier*)malloc(sizeof(struct bufferDossier)))==NULL)
	{
		perror("Erreur allocation bufferDossier");
		exit(EXIT_FAILURE);
	}
	bufferDossier->dernier=NULL;
	bufferDossier->liste=NULL;
	addBuffDossier(racine,bufferDossier);


	//Initialisation du buffer de fichier
	if((bufferFichier=(struct bufferFichier*)malloc(sizeof(struct bufferFichier)))==NULL)
	{
		perror("Erreur allocation structure bufferFichier");
		exit(EXIT_FAILURE);
	}

	if((bufferFichier->chemin=(char**)malloc(tailleBufferFichier*sizeof(char*)))==NULL)
	{
		perror("Erreur allocation buffer de fichier");
		exit(EXIT_FAILURE);
	}

	bufferFichier->taille=tailleBufferFichier;
	bufferFichier->idxLecteur=0;
	bufferFichier->idxEcrivain=0;
	bufferFichier->interIdx=0;

	pthread_t* tidScanner;
	if((tidScanner=(pthread_t*)malloc(nbScanner*sizeof(pthread_t)))==NULL)
	{
		perror("Erreur allocation du tableau de thread scanner");
		exit(EXIT_FAILURE);
	}

	pthread_t* tidAnalyser;
	if((tidAnalyser=(pthread_t*)malloc(nbAnalyser*sizeof(pthread_t)))==NULL)
	{
		perror("Erreur allocation du tableau de thread analyser");
		exit(EXIT_FAILURE);
	}


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

	if(arg.verbeux==0)
	{
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
	}
	else
	{
		printf("Copie dossier %s\n",arg.destination);
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

	char* cheminSource;
	if((cheminSource=(char*)malloc(lgPrefixeSource + lgAncienChemin + 2))==NULL)
	{
		perror("Erreur allocation chaine de caractère pour chemin source");
		exit(EXIT_FAILURE);
	}
	sprintf(cheminSource,"%s/%s",arg->source,dossierTraiter->chemin);
	int lgcheminSource=strlen(cheminSource);	//XXX peut changer car connait la taille

	DIR* dossier;
	if((dossier=opendir(cheminSource))==NULL)
	{
		perror("Erreur ouverture d'un repertoire impossible");
		exit(EXIT_FAILURE);
	}
	struct dirent* entree;
	if((entree=(struct dirent*)malloc(sizeof(struct dirent)))==NULL)
	{
		perror("Erreur allocation d'un entree dans le repertoire");
		exit(EXIT_FAILURE);
	}

	while((entree=readdir(dossier))!=NULL)
	{
		if(strcmp(entree->d_name,".")!=0 && strcmp(entree->d_name,"..")!=0)
		{
			struct stat* info;
			if((info=(struct stat*)malloc(sizeof(struct stat)))==NULL)
			{
				perror("Erreur allocation de la structure stat d'une entree");
				exit(EXIT_FAILURE);
			}

			int lgNom=strlen(entree->d_name);

			char* newCheminSrc;
			if((newCheminSrc=(char*)malloc(lgcheminSource + lgNom +2))==NULL)
			{
				perror("Erreur allocation chaine de caractère du chemin d'une entree");
				exit(EXIT_FAILURE);
			}
			sprintf(newCheminSrc,"%s/%s",cheminSource,entree->d_name);


			if(stat(newCheminSrc,info)==-1)
			{
				perror("Erreur stat");
				exit(EXIT_FAILURE);
			}

			char* suffixeEntreeSuivante;
			if((suffixeEntreeSuivante=(char*)malloc(lgAncienChemin + lgNom + 2))==NULL)
			{
				perror("Erreur allocation chaine de caractere du suffixe de l'entree");
				exit(EXIT_FAILURE);
			}
			sprintf(suffixeEntreeSuivante,"%s/%s",dossierTraiter->chemin,entree->d_name);

			if(S_ISREG(info->st_mode)!=0)
			{
				addBuffFichier(suffixeEntreeSuivante,bufferFichier,arg);
			}
			if(S_ISDIR(info->st_mode)!=0)
			{
				
				int lgSuffixeEntreeSuivante=strlen(suffixeEntreeSuivante);

				char* cheminDossierSuivant;
				if((cheminDossierSuivant=(char*)malloc(lgSuffixeEntreeSuivante + lgPrefixeDest + 2))==NULL)
				{
					perror("Erreur allocation chaine de caractere pour le chemin complet de l'entree");
					exit(EXIT_FAILURE);
				}
				sprintf(cheminDossierSuivant,"%s/%s",arg->destination,suffixeEntreeSuivante);

				if(arg->verbeux==0)
				{
					//TODO tester
					mkdir(cheminDossierSuivant,info->st_mode);
								}
				else
				{
					printf("Copie dossier %s\n",cheminDossierSuivant);
				}
				struct maillon* maillonDossierSuivant=creerMaillonDossier(suffixeEntreeSuivante);
				pthread_mutex_lock(&arg->mut_scanner);
				addBuffDossier(maillonDossierSuivant,bufferDossier);
				pthread_mutex_unlock(&arg->mut_scanner);



				free(cheminDossierSuivant);
			}
			free(suffixeEntreeSuivante);

			free(newCheminSrc);
			free(info);
		}
	
	}

	free(entree);
	closedir(dossier);
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

void copieComplete(char* src,char* dest,struct stat* statSource,struct argument* arg)
{
	if(arg->verbeux==1)
	{
		printf("Creation fichier %s\n",dest);
	}
	else
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
			copieComplete(cheminSource,cheminDestination,&statSource,arg);
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
				copieComplete(cheminSource,cheminDestination,&statSource,arg);
			}
		}

		free(cheminSauvegarde);
	}
	else
	{
		copieComplete(cheminSource,cheminDestination,&statSource,arg);
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
		//	printf("extrait du buffer %s",extrait);
			pthread_mutex_unlock(&argument->mut_analyser);
			pthread_cond_broadcast(&argument->cond_analyser);
			executionAnalyser(extrait,arg);

			pthread_mutex_lock(&argument->mut_analyser);
			pthread_mutex_lock(&argument->mut_compt);
			free(extrait);
		}
	}
}
