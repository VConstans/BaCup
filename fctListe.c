#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
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

	maillon->chemin=path;
	maillon->suivant=NULL;

	return maillon;
}

void rmMaillonDossier(struct maillon* maillon)
{
	free(maillon->chemin);
	free(maillon);
}


/**********************************************************************/

void addBuffFichier(char* chemin,struct bufferFichier* buff)
{
	buff->chemin[buff->idxEcrivain]=chemin;
	buff->idxEcrivain=(buff->idxEcrivain+1)%buff->taille;
}
