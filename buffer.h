#ifndef _SAUVE_H_
#define _SAUVE_H_

struct bufferDossier
{
	char* nom;
	char* chemin;
	struct bufferDossier* suivant;
};

#endif
