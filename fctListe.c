#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "buffer.h"
#include "argument.h"
#include "fctListe.h"

void addBuffDossier(struct maillon* maillon,struct bufferDossier* buff)
{
	if(buff->liste==NULL)
	{
		buff->liste=maillon;
	}
	else
	{
		buff->dernier->suivant=maillon;
	}
	buff->dernier=maillon;
}


void rmBuffDossier(struct bufferDossier* buff)
{
	struct maillon* tmp=buff->liste;
	buff->liste=buff->liste->suivant;
	free(tmp->chemin);
	free(tmp);
}


struct maillon* extractBuffDossier(struct bufferDossier* buff)
{
	struct maillon* tmp = buff->liste;
	if(buff->liste==buff->dernier)
	{
		buff->dernier=NULL;
	}
	buff->liste=buff->liste->suivant;
	return tmp;
}

struct maillon* creerMaillonDossier(char* path)
{
	struct maillon* maillon;
	if((maillon=(struct maillon*)malloc(sizeof(struct maillon)))==NULL)
	{
		perror("Erreur malloc maillon");
		exit(EXIT_FAILURE);
	}

	if((maillon->chemin=(char*)malloc(strlen(path)+1))==NULL)
	{
		perror("Erreur malloc chemin maillon");
		exit(EXIT_FAILURE);
	}
	strcpy(maillon->chemin,path);
	maillon->suivant=NULL;

	return maillon;
}


//TODO lock
void rmMaillonDossier(struct maillon* maillon)
{
	free(maillon->chemin);
	free(maillon);
}


/**********************************************************************/


void addBuffFichier(char* chemin,struct bufferFichier* buff,struct argument* arg)
{
	pthread_mutex_lock(&arg->mut_analyser);
	while(buff->interIdx>=buff->taille)
	{
		pthread_cond_wait(&arg->cond_analyser,&arg->mut_analyser);
	}

	buff->chemin[buff->idxEcrivain]=(char*)malloc(strlen(chemin)+1);
	strcpy(buff->chemin[buff->idxEcrivain],chemin);
//	printf("Dans buffer fichier %s\n",buff->chemin[buff->idxEcrivain]);

	buff->idxEcrivain=(buff->idxEcrivain+1)%buff->taille;
	buff->interIdx++;

	pthread_mutex_unlock(&arg->mut_analyser);
}

char* extractBuffFichier(struct bufferFichier* buff,struct argument* arg)
{
	while(buff->interIdx<=0)
	{
		pthread_cond_wait(&arg->cond_analyser,&arg->mut_analyser);
	}
	char* tmp=buff->chemin[buff->idxLecteur];
	buff->idxLecteur=(buff->idxLecteur+1)%buff->taille;
	buff->interIdx--;
	return tmp;
}
