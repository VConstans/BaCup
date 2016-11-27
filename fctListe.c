#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "fctListe.h"

void addBuff(struct bufferDossier* maillon,struct bufferDossier* buff)
{
	struct bufferDossier* tmp=buff;
	while(tmp->suivant!=NULL)
	{
		tmp=tmp->suivant;
	}
	tmp->suivant=maillon;
}


void rmBuff(struct bufferDossier* buff)
{
	struct bufferDossier* tmp=buff;
	buff=buff->suivant;
	free(tmp->chemin);
	free(tmp);
}


struct bufferDossier* extractBuff(struct bufferDossier* buff)
{
	struct bufferDossier* tmp = buff;
	buff=buff->suivant;
	return tmp;
}

struct bufferDossier* creerMaillon(char* path)
{
	struct bufferDossier* maillon;
	if((maillon=(struct bufferDossier*)malloc(sizeof(struct bufferDossier)))==NULL)
	{
		perror("Erreur malloc maillon");
		exit(EXIT_FAILURE);
	}

	maillon->chemin=(char*)malloc(strlen(path)*sizeof(char));
	strcpy(maillon->chemin,path);

	return maillon;
}

void rmMaillon(struct bufferDossier* buff)
{
	free(buff->chemin);
	free(buff);
}
