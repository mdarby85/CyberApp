#include <stdlib.h>
#include "Database.h"

PGconn* connectToDB(const char *conninfo){
    PGconn *conn = PQconnectdb(conninfo);
    printf("1: %p\n",conn);
    /* Check to see that the backend connection was successfully made */
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        closeDBConnection(conn);
        exit(1);
    } else {
        printf("Database Connection Successful!\n");
	printf("Check 1: %p\n",conn);
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
        PQclear(res);
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

struct Position getPositionFromToken(PGconn *conn, const char *token){

    struct Position temp;
    temp.lat = 0;
    temp.lon = 0;
    temp.response = '0';

    char query[100 + 64];
    snprintf(query, sizeof(query), "SELECT lat, long FROM position NATURAL JOIN people WHERE signintoken = '%.64s';", token);

    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
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

struct Position getPositionFromEmail(PGconn *conn, const char *email){
    struct Position temp;
    temp.lat = 0;
    temp.lon = 0;
    temp.response = '0';

    char query[100 + 64];
    snprintf(query, sizeof(query), "SELECT lat, long FROM position WHERE email = '%s';", email);

    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
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
    int success = 0;
    char query[200 + 64];
    char *email = getEmailFromToken(conn, token);
    if (email[0] == '\0'){
        return success;
    }
    PGresult *res;
    snprintf(query, sizeof(query), "INSERT INTO position (email, lat, long) VALUES ('%s', %f, %f) ON CONFLICT (email) DO UPDATE SET lat = %f, long = %f;", email, lat, lon, lat, lon);
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
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
    PQclear(res);
    return success;
}

/* If no email was found, function returns empty char* -> "\0" */
char* getEmailFromToken(PGconn *conn, const char* token){
    char query[100 + 64];
    snprintf(query, sizeof(query), "SELECT email FROM people WHERE signintoken = '%s';", token);
    static char email[100] = {0};

    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
        return email;
    }
    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){
        if (PQntuples(res) > 0) {
            snprintf(email, 100, "%s", PQgetvalue(res, 0, 0));
        } else {
            PQclear(res);
            return email;
        }
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    PQclear(res);
    return email;
}

int getFriendStatus(PGconn *conn, const char *userEmail, const char *friendEmail){
    int friendStatus = -1;
    char query[100 + sizeof(userEmail) + sizeof(friendEmail)];
    snprintf(query, sizeof(query), "SELECT status FROM knownpeople WHERE email = '%s' AND friendemail = '%s';", userEmail, friendEmail);
    char str_status[10];
    PGresult *res;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        friendStatus = NOT_FRIEND;
    }
    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){
        if (PQntuples(res) > 0) {
            snprintf(str_status, 10, "%s", PQgetvalue(res, 0, 0));
            friendStatus = atoi(str_status);
            if (friendStatus != IS_FRIEND && friendStatus != PEND_FRIEND && friendStatus != NOT_FRIEND){
                friendStatus = NOT_FRIEND;
            }

        } else {
            friendStatus = NOT_FRIEND;
        }
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    PQclear(res);
    return friendStatus;
}

int initFriendRequest(PGconn *conn, const char *userEmail, const char *friendEmail){
    int success = 0;
    char query[100 + sizeof(userEmail) + sizeof(friendEmail)];
    PGresult *res;
    snprintf(query, sizeof(query), "INSERT INTO knownpeople (email, friendemail, status) VALUES ('%s', '%s', %i);", userEmail, friendEmail, PEND_FRIEND);
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
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
    PQclear(res);
    return success;

}

int acceptFriendRequest(PGconn *conn, const char *userEmail, const char *friendEmail){
    int success = 0;
    char query[200 + sizeof(userEmail) + sizeof(friendEmail)];

    PGresult *res;
    snprintf(query, sizeof(query), "UPDATE knownpeople SET status = %i WHERE email = '%s' AND friendemail = '%s';", IS_FRIEND, userEmail, friendEmail);
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
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
    PQclear(res);
    return success;
}

int removeFriend(PGconn *conn, const char *userEmail, const char *friendEmail){
    int success = 0;
    char query[100 + sizeof(userEmail) + sizeof(friendEmail)];

    PGresult *res;
    snprintf(query, sizeof(query), "DELETE FROM knownpeople WHERE email = '%s' AND friendemail = '%s';", userEmail, friendEmail);
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
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
    PQclear(res);
    return success;
}

void printQuery(PGresult *res){
    PQprintOpt opt = {0};
    opt.header = 1;
    opt.align = 1;
    opt.fieldSep = "|";
    PQprint(stdout, res, &opt);
}
