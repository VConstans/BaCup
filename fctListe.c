#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "fctListe.h"

void addBuffDossier(struct bufferDossier* maillon,struct bufferDossier** buff)
{
	if(buff==NULL)
	{
		*buff=maillon;
		return;
	}
	struct bufferDossier* tmp=*buff;
	while(tmp->suivant!=NULL)
	{
		tmp=tmp->suivant;
	}
	tmp->suivant=maillon;
}


void rmBuffDossier(struct bufferDossier** buff)
{
	struct bufferDossier* tmp=*buff;
	*buff=*buff->suivant;
	free(tmp->chemin);
	free(tmp);
}


struct bufferDossier* extractBuffDossier(struct bufferDossier** buff)
{
	struct bufferDossier* tmp = *buff;
	*buff=*buff->suivant;
	return tmp;
}

struct bufferDossier* creerMaillonDossier(char* path)
{
	struct bufferDossier* maillon;
	if((maillon=(struct bufferDossier*)malloc(sizeof(struct bufferDossier)))==NULL)
	{
		perror("Erreur malloc maillon");
		exit(EXIT_FAILURE);
	}

	maillon->chemin=path;
	maillon->suivant=NULL;

	return maillon;
}

void rmMaillonDossier(struct bufferDossier** buff)
{
	free(*buff->chemin);
	free(*buff);
}


/**********************************************************************/

void addBuffFichier(char* chemin,struct bufferFichier* buff)
{
	buff->chemin[buff->idxEcrivain]=chemin;
	buff->idxEcrivain=(buff->idxEcrivain+1)%buff->taille;
}
