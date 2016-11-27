#ifndef _SAUVE_H_
#define _SAUVE_H_

struct bufferDossier
{
	char* nom;
	char* chemin;
	struct dossier* suivant;
};

void addBuff(struct bufferDossier* maillon, struct bufferDossier* buff);
void rmBuff(struct bufferDossier* buff);
struct bufferDossier* creerMaillon(...); //TODO

#endif
