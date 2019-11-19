//
// Created by maran on 11/18/2019.
//
#include <stdlib.h>
#include "Database.h"

PGconn* connectToDB(const char *conninfo){
    PGconn *conn = PQconnectdb(conninfo);
    /* Check to see that the backend connection was successfully made */
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        closeDBConnection(conn);
        exit(1);
    } else {
        printf("Database Connection Successful!\n");
        return conn;
    }
}

void closeDBConnection(PGconn *conn){
    PQfinish(conn);
    printf("Database Connection Closed\n");
}

int processQuery(PGconn *conn, const char *query){
    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        return 0;
    }
    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){

        printQuery(res);
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }

    int found_result = PQntuples(res);
    PQclear(res);
    return (found_result > 0 ? 1 : 0);
}

struct Position getPosition(PGconn *conn, const char *token){

    struct Position temp;
    temp.lat = 0;
    temp.lon = 0;
    temp.response = '0';

    char query[100 + 64];
    snprintf(query, sizeof(query), "SELECT lat, long FROM position NATURAL JOIN people WHERE signintoken = '%s';", token);

    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        return temp;
    }
    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){
        /* From query, we know data is in order lat, long */
        if (PQntuples(res) > 0) {
            temp.lat = atof(PQgetvalue(res, 0, 0));
            temp.lon = atof(PQgetvalue(res, 0, 1));
            temp.response = PQntuples(res) > 0 ? '1' : '0';
        }
        printQuery(res);
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    PQclear(res);
    return temp;
}

int updateLocation(PGconn *conn, const char *token, const float lat, const float lon){
    char query[200 + 64];
    snprintf(query, sizeof(query), "SELECT email FROM people WHERE signintoken = '%s';", token);
    char email[100];

    /* Check for email associated with token */
    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        return 0;
    }
    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){
        if (PQntuples(res) > 0) {
            snprintf(email, 100, "%s", PQgetvalue(res, 0, 0));
        } else {
            return 0;
        }
//        printQuery(res);
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    PQclear(res);

    int success = 0;
    snprintf(query, sizeof(query), "INSERT INTO position (email, lat, long) VALUES ('%s', %f, %f) ON CONFLICT (email) DO UPDATE SET lat = %f, long = %f;", email, lat, lon, lat, lon);
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        return 0;
    }
    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){
        success = 1;
        printQuery(res);
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    return success;
}

void printQuery(PGresult *res){
    PQprintOpt opt = {0};
    opt.header = 1;
    opt.align = 1;
    opt.fieldSep = "|";
    PQprint(stdout, res, &opt);
}