#ifndef _SAUVE_H_
#define _SAUVE_H_

struct maillon
{
	char* nom;
	char* chemin;
	struct dossier* suivant;
};

struct bufferDossier
{
	struct dossier* nextLire;
	struct dossier* nextAjout;
};

void addBuff(struct buffer maillon, struct buffer* buff);
struct buffer* rmBuff(struct buffer* buff);
struct maillon creerMaillon(...); //TODO

#endif
