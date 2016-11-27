#ifndef _SAUVE_H_
#define _SAUVE_H_

#define TAILLE 10

struct bufferDossier
{
	char* nom;
	char* chemin;
	struct bufferDossier* suivant;
};


struct bufferFichier
{
	char* chemin[TAILLE];
	int taille=TAILLE;
	int idxLecteur;
	int idxEcrivain;
};

#endif
