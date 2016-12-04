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
#include "argument.h"



//Déclaration des deux buffers et du compteur de scanner actif
struct bufferDossier* bufferDossier=NULL;
struct bufferFichier* bufferFichier=NULL;
int scannerActif=0;


/* Cree un nouveau chemin compose de deux chaine de caractere separe pour un
 * slash
 */
char* creerChemin(char* A, char* B)
{
	char* chemin;
	if((chemin=(char*)malloc(strlen(A) + strlen(B) + 2))==NULL)
	{
		perror("Erreur allocation chemin");
		exit(EXIT_FAILURE);
	}
	sprintf(chemin,"%s/%s",A,B);
	return chemin;
}


/* Fonction d'execution des threads scanner qui va scanner un repertoire passe en
 * parametre et ajouter les elements composant ce repertoire soit dans le buffer
 * de fichier soit dans le buffer de dossier selon la nature de l'element.
 * Si c'est un dossier il s'occuper aussi de creer celui-ci dans l'arborescence copie
 */
void executionScanner(struct maillon* dossierTraiter,struct argument* arg)
{
	/* Le chemin contenu dans le buffer etant le chemin relatif par
	 * rapport a la racine source, il faut creer le chemin complet
	 * etant le chemin de la racine et le chemin relatif a cette racine
	 */
	char* cheminSource;
	cheminSource=creerChemin(arg->source,dossierTraiter->chemin);

	DIR* dossier;
	if((dossier=opendir(cheminSource))==NULL)
	{
		perror("Erreur ouverture d'un repertoire impossible");
		exit(EXIT_FAILURE);
	}
	struct dirent entree;
	struct dirent* resultat;
	int retour_read;

	while((retour_read=readdir_r(dossier,&entree,&resultat))==0 && resultat!=NULL)
	{
		if(strcmp(entree.d_name,".")!=0 && strcmp(entree.d_name,"..")!=0)
		{
			struct stat* info;
			if((info=(struct stat*)malloc(sizeof(struct stat)))==NULL)
			{
				perror("Erreur allocation de la structure stat d'une entree");
				exit(EXIT_FAILURE);
			}

			//Creation du chemins des elements du repertoire source
			char* newCheminSrc;
			newCheminSrc=creerChemin(cheminSource,entree.d_name);


			if(stat(newCheminSrc,info)==-1)
			{
				perror("Erreur stat");
				exit(EXIT_FAILURE);
			}

			
			/* Chemin de l'element du repertoire relatif a la racine
			 * (ne contient ni le chemin de la racine ni le chemin de
			 * la destination
			 */
			char* suffixeEntreeSuivante;
			suffixeEntreeSuivante=creerChemin(dossierTraiter->chemin,entree.d_name);

			// Si l'element est un fichier
			if(S_ISREG(info->st_mode)!=0)
			{
				//Ajout du chemin relatif de l'element dans le buffer de fichier
				addBuffFichier(suffixeEntreeSuivante,bufferFichier,arg);
			}

			// Si l'element est un dossier
			if(S_ISDIR(info->st_mode)!=0)
			{
				/* Creation du chemin de copie de l'element: c'est a dire, le chemin
				 * compose du chemin de la racine de copie et du chemin relatif de
				 * l'element
				 */
				char* cheminDossierSuivant;
				cheminDossierSuivant=creerChemin(arg->destination,suffixeEntreeSuivante);

				//Si le programme n'est pas en mode verbeux
				if(arg->verbeux==0)
				{
					//Creation du dossier dans l'arborescence de copie
					if(mkdir(cheminDossierSuivant,info->st_mode)==-1)
					{
						perror("Erreur création d'un dossier");
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					//Si on est en mode verbeux
					printf("Creation dossier %s\n",cheminDossierSuivant);
				}

				/*On ajoute l'element dans le buffer des dossier
				 * pour qu'il soit scanner
				 */
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
	if(retour_read!=0)
	{
		perror("Erreur readdir_t");
		exit(EXIT_FAILURE);
	}

	closedir(dossier);
	free(cheminSource);
}



void* scanner(void* arg)
{
	struct argument* argument=(struct argument*)arg;

	/* Verouillage du buffer de dossier et du compteur
	 * de scanner actif
	 */
	pthread_mutex_lock(&argument->mut_scanner);
	pthread_mutex_lock(&argument->mut_compt);
	while(1)
	{
		/* Si le buffer dossier est vide mais qu'il reste
		 * des scanner actif
		 */
		while(bufferDossier->liste==NULL && scannerActif!=0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_wait(&argument->cond_scanner,&argument->mut_scanner);
			pthread_mutex_lock(&argument->mut_compt);
		}

		/* Si le buffer de dossier est vide mais qu'il n'y
		 * a plus de scanner actif
		 */
		if(bufferDossier->liste==NULL && scannerActif==0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_broadcast(&argument->cond_scanner);
			pthread_mutex_unlock(&argument->mut_scanner);
			return (NULL);
		}

		// Si il y a des dossier dans le buffer de dossier
		else
		{
			scannerActif++;
			pthread_mutex_unlock(&argument->mut_compt);

			//Extraction du repertoire a traiter
			struct maillon* extrait;
			extrait=extractBuffDossier(bufferDossier);

			pthread_mutex_unlock(&argument->mut_scanner);

			//Traitement du repertoire à scanner
			executionScanner(extrait,argument);

			/* Avertissement des scanner et des analyser qu'il
			 * potentiellement de nouveau element dans les buffer
			 */
			pthread_cond_broadcast(&argument->cond_scanner);
			pthread_cond_broadcast(&argument->cond_analyser);
			pthread_mutex_lock(&argument->mut_scanner);

			pthread_mutex_lock(&argument->mut_compt);
			rmMaillonDossier(extrait);
			scannerActif--;
		}
	}
}


/***************************************************************/

/* Copie le contenu d'un fichier source vers un fichier destination
 */
void copie(int src,int dest)
{
	char* bufferCopie[1024];
	int nbLu;

	while((nbLu=read(src,bufferCopie,1024))!=0)
	{
		write(dest,bufferCopie,nbLu);
	}
}


/* Prepare la copie d'un fichier source vers un fichier destination.
 */
void copieComplete(char* src,char* dest,struct stat* statSource,struct argument* arg)
{
	// Si le programme est en mode verbeux
	if(arg->verbeux==1)
	{
		printf("Copie fichier %s vers %s\n",src,dest);
	}
	else
	{
		// Creation du fichier destination et ouverture de celui ci
		int fichierDestination;
		if((fichierDestination=open(dest,O_WRONLY|O_CREAT,statSource->st_mode))==-1)
		{
			perror("Erreur ouverture fichier destination");
			exit(EXIT_FAILURE);
		}

		//Ouverture du fichier source
		int fichierSource;
		if((fichierSource=open(src,O_RDONLY))==-1)
		{
			perror("Erreur ouverture fichier source");
			exit(EXIT_FAILURE);
		}

	
		//Copie du fichier source vers le fichier destination
		copie(fichierSource,fichierDestination);

		struct utimbuf date;
		date.actime=statSource->st_atime;
		date.modtime=statSource->st_mtime;

		//Mise a jour de la date
		if(utime(dest,&date)!=0)
		{
			perror("Erreur changement de date");
			exit(EXIT_FAILURE);
		}

		close(fichierSource);
		close(fichierDestination);
	}
}


void executionAnalyser(char* suffixeCheminFichier,struct argument* arg)
{
	/* Creation du chemin source complet (chemin de la racine 
	 * et du chemin relatif a la racine)
	 */
	char* cheminSource;
	cheminSource=creerChemin(arg->source,suffixeCheminFichier);

	/* Creation du chemin destination complet (chemin de la racine 
	 * et du chemin relatif a la racine)
	 */
	char* cheminDestination;
	cheminDestination=creerChemin(arg->destination,suffixeCheminFichier);

	struct stat statSource;
	if(stat(cheminSource,&statSource)!=0)
	{
		perror("Erreur stat source");
		exit(EXIT_FAILURE);
	}


	//Si le programme est en mode incremental
	if(arg->incremental==1)
	{
		/* Creation du chemin contenant la precedente sauvegarde
		 */
		char* cheminSauvegarde;
		cheminSauvegarde=creerChemin(arg->sauvegarde,suffixeCheminFichier);

		struct stat statSauvegarde;
	
		// Si le fichier n'etait pas present dans la precedente sauvegarde
		if(access(cheminSauvegarde,F_OK)!=0)
		{
			//On copie le fichier
			copieComplete(cheminSource,cheminDestination,&statSource,arg);
		}
		// Si le fichier etait deja present dans la precedente sauvegarde
		else
		{

			if(stat(cheminSauvegarde,&statSauvegarde)!=0)
			{
				perror("Erreur fstat sauvegarde");
				exit(EXIT_FAILURE);
			}
	
			/* On compare la taille, la date de derniere modification et les permission
			 * du fichier de la sauvegarde et du fichier source
			 */
			if(statSource.st_size == statSauvegarde.st_size && statSource.st_mtime == statSauvegarde.st_mtime && statSource.st_mode == statSauvegarde.st_mode)
			{// Si ils sont identique
				
				//Si le programme n'est pas en mode verbeux
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
					printf("Création du lien entre %s et %s\n",cheminDestination,cheminSauvegarde);
				}
			}
			else
			{// Si ils ne sont pas identique
				copieComplete(cheminSource,cheminDestination,&statSource,arg);
			}
		}

		free(cheminSauvegarde);
	}
	//Si le programme n'est pas en mode incrementale
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
		/* Si le buffer de fichier est vide mais qu'il y a
		 * encore des scanner actif
		 */
		while(bufferFichier->interIdx==0 && scannerActif!=0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_wait(&argument->cond_analyser,&argument->mut_analyser);
			pthread_mutex_lock(&argument->mut_compt);
		}

		/* Si le buffer de fichier est vide et qu'il n'y a
		 * plus de scanner actif
		 */
		if(bufferFichier->interIdx==0 && scannerActif==0)
		{
			pthread_mutex_unlock(&argument->mut_compt);
			pthread_cond_broadcast(&argument->cond_analyser);
			pthread_mutex_unlock(&argument->mut_analyser);
			return (NULL);
		}
		//Si le buffer contient ou moins un fichier a traiter
		else
		{
			pthread_mutex_unlock(&argument->mut_compt);

			//Extraction du fichier a traiter
			char* extrait=extractBuffFichier(bufferFichier/*,argument*/);
			pthread_cond_broadcast(&argument->cond_analyser);

			pthread_mutex_unlock(&argument->mut_analyser);

			//Traitement du fichier
			executionAnalyser(extrait,arg);

			pthread_mutex_lock(&argument->mut_analyser);
			pthread_mutex_lock(&argument->mut_compt);
			free(extrait);
		}
	}
}



/*******************************************************************************************/




int main(int argc,char* argv[])
{
	int argument;

	struct argument arg;
	arg.verbeux=0;

	/*Valeur par defaut du nombre de scanner, d'annalyser
	 * et de la taille du buffer de fichier
	 */
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
				break;
			case 's':
				nbScanner=atoi(optarg);
				break;
			case 'a':
				nbAnalyser=atoi(optarg);
				break;
			case 'f':
				tailleBufferFichier=atoi(optarg);
				break;
		}
	}
	switch(argc-optind)
	{
		case 2:
			arg.source=argv[optind++];
			arg.destination=argv[optind];
			arg.incremental=0;
			break;
		case 3:
			arg.source=argv[optind++];
			arg.sauvegarde=argv[optind++];
			arg.destination=argv[optind];
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
		rmMaillonDossier(racine);
		exit(EXIT_FAILURE);
	}
	bufferDossier->dernier=NULL;
	bufferDossier->liste=NULL;
	addBuffDossier(racine,bufferDossier);


	//Initialisation du buffer de fichier
	if((bufferFichier=(struct bufferFichier*)malloc(sizeof(struct bufferFichier)))==NULL)
	{
		perror("Erreur allocation structure bufferFichier");
		free(bufferDossier);
		rmMaillonDossier(racine);
		exit(EXIT_FAILURE);
	}

	if((bufferFichier->chemin=(char**)malloc(tailleBufferFichier*sizeof(char*)))==NULL)
	{
		perror("Erreur allocation buffer de fichier");
		free(bufferDossier);
		free(bufferFichier);
		rmMaillonDossier(racine);
		exit(EXIT_FAILURE);
	}

	bufferFichier->taille=tailleBufferFichier;
	bufferFichier->idxLecteur=0;
	bufferFichier->idxEcrivain=0;
	bufferFichier->interIdx=0;

	//Initialisation des threads
	pthread_t* tidScanner;
	if((tidScanner=(pthread_t*)malloc(nbScanner*sizeof(pthread_t)))==NULL)
	{
		perror("Erreur allocation du tableau de thread scanner");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		rmMaillonDossier(racine);
		exit(EXIT_FAILURE);
	}

	pthread_t* tidAnalyser;
	if((tidAnalyser=(pthread_t*)malloc(nbAnalyser*sizeof(pthread_t)))==NULL)
	{
		perror("Erreur allocation du tableau de thread analyser");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		free(tidScanner);
		rmMaillonDossier(racine);
		exit(EXIT_FAILURE);
	}


	//Initialisation des arguments à passer aux threads

	if(pthread_mutex_init(&arg.mut_scanner,NULL)!=0)
	{
		perror("Erreur creation mutex Scanner");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		free(tidScanner);
		free(tidAnalyser);
		rmMaillonDossier(racine);

		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&arg.mut_analyser,NULL)!=0)
	{
		perror("Erreur creation mutex analyser");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		free(tidScanner);
		free(tidAnalyser);
		rmMaillonDossier(racine);

		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&arg.mut_compt,NULL)!=0)
	{
		perror("Erreur creation mutex analyser");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		free(tidScanner);
		free(tidAnalyser);
		rmMaillonDossier(racine);

		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&arg.cond_scanner,NULL)!=0)
	{
		perror("Erreur creation condition scanner");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		free(tidScanner);
		free(tidAnalyser);
		rmMaillonDossier(racine);

		exit(EXIT_FAILURE);
	}

	if(pthread_cond_init(&arg.cond_analyser,NULL)!=0)
	{
		perror("Erreur creation condition analyser");
		free(bufferDossier);
		free(bufferFichier->chemin);
		free(bufferFichier);
		free(tidScanner);
		free(tidAnalyser);
		rmMaillonDossier(racine);

		exit(EXIT_FAILURE);
	}

	//Traitement du dossier racine
	if(arg.verbeux==0)
	{
		struct stat statSource;
		if(stat(arg.source,&statSource)!=0)
		{
			perror("Erreur stat racine");
			free(bufferDossier);
			free(bufferFichier->chemin);
			free(bufferFichier);
			free(tidScanner);
			free(tidAnalyser);
			rmMaillonDossier(racine);
			exit(EXIT_FAILURE);
		}

		if(mkdir(arg.destination,statSource.st_mode)!=0)
		{
			perror("Erreur creation dossier racine");
			free(bufferDossier);
			free(bufferFichier->chemin);
			free(bufferFichier);
			free(tidScanner);
			free(tidAnalyser);
			rmMaillonDossier(racine);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("Copie dossier %s\n",arg.destination);
	}


	//Creation des threads
	int i;
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_create(&tidScanner[i],NULL,scanner,&arg)!=0)
		{
			perror("Erreur création de thread scanneur");
			free(bufferDossier);
			free(bufferFichier->chemin);
			free(bufferFichier);
			free(tidScanner);
			free(tidAnalyser);
			rmMaillonDossier(racine);
			exit(EXIT_FAILURE);
		}
	}

	int j;

	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_create(&tidAnalyser[j],NULL,analyser,&arg)!=0)
		{
			perror("Erreur creation de thread analyseur");
			free(bufferDossier);
			free(bufferFichier->chemin);
			free(bufferFichier);
			free(tidScanner);
			free(tidAnalyser);
			rmMaillonDossier(racine);
			exit(EXIT_FAILURE);
		}
	}


	//Attente de la terminaison des threads
	for(i=0;i<nbScanner;i++)
	{
		if(pthread_join(tidScanner[i],NULL)!=0)
		{
			perror("Erreur terminaison de thread scanneur");
			free(bufferDossier);
			free(bufferFichier->chemin);
			free(bufferFichier);
			free(tidScanner);
			free(tidAnalyser);
			rmMaillonDossier(racine);
			exit(EXIT_FAILURE);
		}
	}


	for(j=0;j<nbAnalyser;j++)
	{
		if(pthread_join(tidAnalyser[j],NULL)!=0)
		{
			perror("Erreur terminaison de thread analyseur");
			free(bufferDossier);
			free(bufferFichier->chemin);
			free(bufferFichier);
			free(tidScanner);
			free(tidAnalyser);
			rmMaillonDossier(racine);
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
