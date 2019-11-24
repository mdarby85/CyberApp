/*
* filename:Database.h
* author: group2
* Date Last modified: 11/23/19
*/
#ifndef GROUP_PROJECT_DATABASE_H
#define GROUP_PROJECT_DATABASE_H

#include <libpq-fe.h>
#include <string.h>
enum FRIEND_STATE {NOT_FRIEND, PEND_FRIEND, IS_FRIEND};

struct Position {
    float lat;
    float lon;
    char response;
};

PGconn* connectToDB(const char *conninfo);
void closeDBConnection(PGconn *conn);
int processQuery(PGconn *conn, const char *query);
void printQuery(PGresult *res);
struct Position getPositionFromToken(PGconn *conn, const char *token);
struct Position getPositionFromEmail(PGconn *conn, const char *email);
int updateLocation(PGconn *conn, const char *token, float lat, float lon);
char * getEmailFromToken(PGconn *conn, const char* token);
int getFriendStatus(PGconn *conn, const char *userEmail, const char *friendEmail);
int initFriendRequest(PGconn *conn, const char *userEmail, const char *friendEmail);
int acceptFriendRequest(PGconn *conn, const char *userEmail, const char *friendEmail);
int removeFriend(PGconn *conn, const char *userEmail, const char *friendEmail);
char* viewFriendRequests(PGconn *conn, const char *userEmail);

#endif //GROUP_PROJECT_DATABASE_H
