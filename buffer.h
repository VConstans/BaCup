#ifndef _SAUVE_H_
#define _SAUVE_H_


struct bufferDossier
{
	char* nom;
	char* chemin;
	struct bufferDossier* suivant;
};


struct bufferFichier
{
	char** chemin;
	int taille;
	int idxLecteur;
	int idxEcrivain;
};

#endif
