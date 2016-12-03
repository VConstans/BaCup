#ifndef _SAUVE_H_
#define _SAUVE_H_


struct maillon
{
	char* chemin;
	struct maillon* suivant;
};

struct bufferDossier
{
	struct maillon* dernier;
	struct maillon* liste;
};



struct bufferFichier
{
	int taille;
	int idxLecteur;
	int idxEcrivain;
	int interIdx;
	char** chemin;
};

#endif
