/*
 * commons.h
 *
 *  Created on: Jan 30, 2016
 *      Author: francisco
 */

#ifndef COMMONS_H_
#define COMMONS_H_

#define MSG_LEN 500
#define BASIC_PERMISSIONS 0666
#define N 20

// CONSTANTES

extern const char *mkfifoError;
extern const char *selectError;
extern const char *openError;
extern const char *rvError;
extern const char *argNumError;
extern const char *argOrdError;
extern const char *termSizeError;
extern const char *writeToMySelf;

extern const char *LogOutMessage;
extern const char *LogOutServerMessage;
extern const char *noUserSelectedMessage;
extern const char *successMessage;
extern const char *welcomeMessage;
extern const char *serverStartMessage;
extern const char *defaultStatus;
extern const char *defaultServer;
extern const char *defaultUsername;
extern const char *loginResultMessage;
extern const char *helpMenu;


// Ordenes Cliente->Servidor

extern const char *ordenQuien;
extern const char *ordenEscribir;
extern const char *ordenEstoy;
extern const char *ordenSalir;
extern const char *ordenAyuda;
extern const char *ordenInvalida;
extern const char *ordenCambiarConversacion;

extern const char *userNotFoundMessage;
extern const char *userNameNotAvaible;


// FUNCIONES

char* getErrorMessage(const char* errorMessage,int line, char* file);
char* getWord(char* string,char* delimeter,int index);
int openFifo(const char *fifo_name);
void writeFull(char *token, char dst[]);


#endif /* COMMONS_H_ */
