#ifndef _FCTLISTE_H_
#define _FCTLISTE_H_

#include "argument.h"

void addBuffDossier(struct maillon* maillon, struct bufferDossier* buff);
void rmBuffDossier(struct bufferDossier* buff);
struct maillon* creerMaillonDossier(char* path);
struct maillon* extractBuffDossier(struct bufferDossier* buff);
void rmMaillonDossier(struct maillon* buff);
void addBuffFichier(char* chemin,struct bufferFichier* buff,struct argument* arg);
char* extractBuffFichier(struct bufferFichier* buff/*,struct argument* arg*/);
#endif
