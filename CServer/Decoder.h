#ifndef GROUP_PROJECT_DECODER_H
#define GROUP_PROJECT_DECODER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libpq-fe.h>

char *handleSignIn(char *myArray, PGconn *pConn);
char* handleGetLocation(char* myArray, PGconn *conn);
char* handleUpdateLocation(char* myArray, PGconn *conn);
char* handleGetFriendLocation(char* myArray);
char* handleGetFriendRequest(char* myArray);
char* handleSendFriendRequest(char* myArray);
char* handleAcceptFriendRequest(char* myArray);
char* generateToken(int length);
char* handleOptions(char* myArray);
int findResponseLength(char* myArray);
float reverseFloat(const float inFloat);

#endif //GROUP_PROJECT_DECODER_H
