#include "buffer.h"
#include "fctListe.h"

void addBuff(struct bufferDossier* maillon,struct bufferDossier* buff)
{
	struct buffer* tmp=buff;
	while(tmp->suivant!=NULL)
	{
		tmp=tmp->suivant;
	}
	tmp->suivant=maillon;
}


void rmBuff(struct buffer* buff)
{
	struct buffDossier* tmp=buff;
	buff=buff->suivant;
	free(tmp->chemin);
	free(tmp);
}


struct bufferDossier* creerMaillon(char* path)
{
	struct bufferDossier* maillon;
	if((maillon=(struct buffferDossier*)malloc(sizeof(struct bufferDossier)))==-1)
	{
		perror("Erreur malloc maillon");
		exit(EXIT_FAILURE);
	}

	maillon->chemin=(char*)malloc(strlen(path)*sizeof(char));
	strcpy(maillon->chemin,path);

	return maillon;
}
