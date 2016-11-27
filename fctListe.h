#ifndef _FCTLISTE_H_
#define _FCTLISTE_H_

void addBuff(struct bufferDossier* maillon, struct bufferDossier* buff);
void rmBuff(struct bufferDossier* buff);
struct bufferDossier* creerMaillon(char* path);
struct bufferDossier* extractBuff(struct bufferDossier* buff);
void rmMaillon(struct bufferDossier* buff);
#endif
