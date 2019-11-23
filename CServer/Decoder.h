#ifndef GROUP_PROJECT_DECODER_H
#define GROUP_PROJECT_DECODER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libpq-fe.h>

char *handleSignIn(char *myArray, PGconn *pConn);
char* handleGetLocation(char* myArray, PGconn *conn);
char* handleUpdateLocation(char* myArray, PGconn *conn);
char* handleGetFriendLocation(char* myArray, PGconn *conn);
char* handleGetFriendRequest(PGconn *conn,char* myArray);
char* handleSendFriendRequest(char* myArray, PGconn *conn);
char* handleAcceptFriendRequest(char* myArray, PGconn *conn);
char* handleRemoveFriend(char* myArray, PGconn *conn);
char* handleOptions(char* myArray, PGconn *conn);

char* generateToken(int length);
int findResponseLength(char* myArray);
float reverseFloat(const float inFloat);

#endif //GROUP_PROJECT_DECODER_H
