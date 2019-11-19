//
// Created by maran on 11/18/2019.
//

#ifndef GROUP_PROJECT_DATABASE_H
#define GROUP_PROJECT_DATABASE_H

#include <libpq-fe.h>

struct Position {
    float lat;
    float lon;
    char response;
};

PGconn* connectToDB(const char *conninfo);
void closeDBConnection(PGconn *conn);
int processQuery(PGconn *conn, const char *query);
void printQuery(PGresult *res);
struct Position getPosition(PGconn *conn, const char *token);
int updateLocation(PGconn *conn, const char *token, float lat, float lon);

#endif //GROUP_PROJECT_DATABASE_H
