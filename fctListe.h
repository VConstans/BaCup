#ifndef _FCTLISTE_H_
#define _FCTLISTE_H_

void addBuffDossier(struct maillon* maillon, struct bufferDossier* buff);
void rmBuffDossier(struct bufferDossier* buff);
struct maillon* creerMaillonDossier(char* path);
struct maillon* extractBuffDossier(struct bufferDossier* buff);
void rmMaillonDossier(struct maillon* buff);
void addBuffFichier(char* chemin,struct bufferFichier* buff);
#endif
