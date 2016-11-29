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
	char** chemin;
	int taille;
	int idxLecteur;
	int idxEcrivain;
};

#endif
