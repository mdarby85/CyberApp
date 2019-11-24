/*
* filename:Database.c
* author: group2
* Date Last modified: 11/23/19
*/
#include <stdlib.h>
#include "Database.h"

/*
* This function connects to the database
* param: the Connection Info to connect properly
* return: A connection variable to the database
*/
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

/* Closes the database*/
void closeDBConnection(PGconn *conn){
    PQfinish(conn);
    printf("Database Connection Closed\n");
}

/*
 * Process a given query
 * param: The connection to the database
 * param: The query to be executed
 * return: 1 on success, 0 otherwise
 */
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

/*
 * Finds a postion of a user
 * param: The connection to the database
 * param: The token to be compared
 * return: Position associated with token
 */
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
            temp.response = PQntuples(res) > 0 ? 1 : 0;
        }
        printQuery(res);
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    PQclear(res);
    return temp;
}

/*
 * Finds a postion associated with an email
 * param: The connection to the database
 * param: The email to be compared
 * return: Position associated with email
 */
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

/*
 * Updates the location to a specific lat,ong
 * param: The connection to the database
 * param: The token of the authenticated user
 * param: The latitude to be updated
 * param: The longitude to be updated
 * return: 1 if success, 0 otherwise
 */
int updateLocation(PGconn *conn, const char *token, const float lat, const float lon){
    int success = 0;
    char query[400 + 64];
    char *email = getEmailFromToken(conn, token);
    if (email[0] == '\0'){
        return success;
    }
    printf("UPDATING LAT TO %f\n",lat);
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

/*
 * Finds a email associated with a token
 * param: The connection to the database
 * param: The token to be compared
 * return: Email associated with a token
   If no email was found, function returns empty char* -> "\0" 
 */
 char* getEmailFromToken(PGconn *conn, const char* token){
    char query[100 + 64];
    printf("My Token is %.64s\n", token); 
    snprintf(query, sizeof(query), "SELECT email FROM people WHERE signintoken = '%.64s';", token);
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

/*
 * Finds all friend requests associated with an email
 * param: The connection to the database
 * param: The email to be compared
 * return: Friend Requests associated with email
 */
char* viewFriendRequests(PGconn *conn, const char *userEmail){
    char query[164];
    snprintf(query, sizeof(query), "SELECT * FROM knownpeople WHERE status = 1 AND friendemail = '%s';", userEmail);
    char* myFriends =(malloc(sizeof(char)*200));
    PGresult *res;
    union myInt{
        char num[4];
	int val;
    } myInt;
    if ((res = PQexec(conn, query)) == NULL){
        printf("%s\n", PQerrorMessage(conn));
        PQclear(res);
        return myFriends;
    }

    if ((PQresultStatus(res) == PGRES_COMMAND_OK) ||
        (PQresultStatus(res) == PGRES_TUPLES_OK)){
        if (PQntuples(res) > 0) {
	    myInt.val = PQntuples(res);
	    int i = 0;
	    int len = 4;
	    strncpy(myFriends,"aaaa",4); 
	    for(i=0;i<PQntuples(res);i++){
                strcat(myFriends+len,PQgetvalue(res, i, 0));
	        len = strlen(myFriends+len) + 1+ len;
	    }
            myFriends[0]=myInt.num[0];
            myFriends[1]=myInt.num[1];
            myFriends[2]=myInt.num[2];
            myFriends[3]=myInt.num[3];

        } else {
	    myFriends[0]=0;
	    myFriends[1]=0;
	    myFriends[2]=0;
	    myFriends[3]=0;
            PQclear(res);
            return myFriends;
        }
    } else {
        printf("%s\n", PQresStatus(PQresultStatus(res)));
        printf("%s\n", PQresultErrorMessage(res));
    }
    PQclear(res);
    return myFriends;
}

/*
 * Finds the friend status associated with user,friend
 * param: The connection to the database
 * param: The email to be compared (user)
 * param: The email to be compared (friend)
 * return: Friend Status
 */
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

/*
 * Initiates a friend request between friends
 * param: The connection to the database
 * param: The email to be compared (user)
 * param: The email to be compared (friend)
 * return: Friend Status
 */
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

/*
 * Accepts a friend request between friends
 * param: The connection to the database
 * param: The email to be compared (user)
 * param: The email to be compared (friend)
 * return: Friend Status
 */
int acceptFriendRequest(PGconn *conn, const char *userEmail, const char *friendEmail){
    int success = 0;
    char query[200 + sizeof(userEmail) + sizeof(friendEmail)];

    PGresult *res;
     snprintf(query, sizeof(query), "UPDATE knownpeople SET status = %i WHERE email = '%s' AND friendemail = '%s';", IS_FRIEND, friendEmail, userEmail);
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

/*
 * Removes a friend
 * param: The connection to the database
 * param: The email to be compared (user)
 * param: The email to be compared (friend)
 * return: Friend Status
 */
int removeFriend(PGconn *conn, const char *userEmail, const char *friendEmail){
    int success = 0;
    char query[100 + sizeof(userEmail) + sizeof(friendEmail)];

    PGresult *res;
    snprintf(query, sizeof(query), "DELETE FROM knownpeople WHERE email = '%s' AND friendemail = '%s';", friendEmail, userEmail);
    printf("%s\n",query);
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

/*Prints the Query*/
void printQuery(PGresult *res){
    PQprintOpt opt = {0};
    opt.header = 1;
    opt.align = 1;
    opt.fieldSep = "|";
    PQprint(stdout, res, &opt);
}
