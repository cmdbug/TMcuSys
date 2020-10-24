#ifndef	_MESSAGE_BOX_USER_H_
#define	_MESSAGE_BOX_USER_H_

#include "GUI.h"
#include "DIALOG.h"

void MESSAGEBOX_Create_User(char *sMessage, const char *sCaption, int Flags, void(*fun)(void));
void MESSAGEBOX_Create_User_Modal(char *sMessage, const char *sCaption);
void MESSAGEBOX_Create_User_Modal_With_SureBTN(char *sMessage, const char *sCaption, void(*fun)(void));

#endif
