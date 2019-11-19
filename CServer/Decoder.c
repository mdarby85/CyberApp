#include <time.h>
#include "Decoder.h"
#include "Database.h"

char CODE_SIGN_IN = 0;
char CODE_LOC_GET = 1;
char CODE_LOC_UPDATE = 2;
char CODE_LOC_FRIEND = 3;
char CODE_FRIEND_GET = 4;
char CODE_FRIEND_SEND = 5;
char CODE_FRIEND_ACCEPT = 6;
char CODE_FRIEND_REMOVE = 7;
char CODE_EXIT = 8;
static int TOKEN_SIZE = 64;


/*VERIFIED TO WORK*/
char* handleSignIn(char* myArray, PGconn *conn) {
    int len = strlen(myArray+1);
    char *myUserName = (char *) malloc(sizeof(char) * len);
    char *myHashedPassword = (char *) malloc(sizeof(char) * 32);
    strncpy(myUserName, myArray + 1, len);
    strncpy(myHashedPassword, myArray + len + 2, 32);
    char myResponse = 1;
    /*CONNOR If the username and hashed password is in the database, SET MyRESPONSE = 1*/
    /*Otherwise it's 0 */

//    myUserName = "test@email.com";
//    myHashedPassword = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

    /* Checks if user + password exist in DB, updating myResponse accordingly*/
    char query[500 + sizeof(myUserName) + sizeof(myHashedPassword) + TOKEN_SIZE];
    snprintf(query, sizeof(query), "SELECT * FROM people WHERE email = '%s' AND pwhash = '%s'", myUserName, myHashedPassword);
    /* Is the number of rows returned > 0? */
    myResponse = (processQuery(conn, query) == 0 ? '0' : '1');

    char *serverResponse = (char *) malloc(sizeof(char) * (TOKEN_SIZE+1));
    char* token = (char *) malloc(sizeof(char) * TOKEN_SIZE);
    strncpy(token, generateToken(TOKEN_SIZE), TOKEN_SIZE);
    strncpy(serverResponse + 1, token, TOKEN_SIZE);
    serverResponse[0] = myResponse;
    if (myResponse == '1') {
        /*CONNOR
         * If the username was authenticated, UPDATE the sign-in token in the database
         * Otherwise, do nothing and SET MyResponse = 0
         */
        /* Had to add the \0 to insert into the DB */
        char token_copy[65];
        snprintf(token_copy, TOKEN_SIZE+1, "%s", token);
        token_copy[64] = '\0';
        snprintf(query, sizeof(query), "UPDATE people SET signintoken = '%s', tokentime = '%d' WHERE email = '%s' AND pwhash = '%s'", token_copy, (int)time(NULL), myUserName, myHashedPassword);
        processQuery(conn, query);
    }
    if(myUserName)
        free(myUserName);
    if(myHashedPassword)
        free(myHashedPassword);
    return serverResponse;
}

/* Verified to work*/
char* handleGetLocation(char* myArray, PGconn *conn){
    char* signInToken = (char *) (malloc(sizeof(char)*65));
    char* myResult = (char *) (malloc(sizeof(char)*9));
    strncpy(signInToken, myArray + 1, 64);
    signInToken[65] = '\0';

    char myResponse = 1;
    union
    {
        char bytes[4];
        float val;
    } latitude;
    union
    {
        char bytes[4];
        float val;
    } longitude;
    latitude.val = 0;
    longitude.val = 0;
    /*CONNOR*/
    /* RETRIEVE appropriate values for MyResponse, longitude.val, and latitude.val variables*/
    /* I had to use union to do this, so sorry*/
    /* If the signin token matches the user, RETRIEVE LONGITUDE.val and LATITUDE.val, SET myResponse = 1*/
    /* If error, SET lattitude and longitude = 0, SET myResponse = 0*/
    struct Position position = getPosition(conn, signInToken);
    latitude.val = position.lat;
    longitude.val = position.lon;
    myResponse = position.response;

    myResult[0] = myResponse;
    latitude.val=reverseFloat(latitude.val);
    longitude.val=reverseFloat(longitude.val);
    strncpy(myResult+1,latitude.bytes,4);
    strncpy(myResult+5,longitude.bytes,4);
    if(signInToken)
        free(signInToken);
    return myResult;
}

char* handleUpdateLocation(char* myArray, PGconn *conn){
    char* signInToken = (char *) (malloc(sizeof(char)*64));

    union
    {
        char bytes[4];
        float val;
    } latitude;

    union
    {
        char bytes[4];
        float val;
    } longitude;

    latitude.val=0;
    longitude.val=0;
    strncpy(latitude.bytes,myArray+65, 4);
    strncpy(longitude.bytes,myArray+69, 4);
    latitude.val = reverseFloat(latitude.val);
    longitude.val = reverseFloat(longitude.val);
    strncpy(signInToken, myArray + 1, 64);
    char myResponse = 0;
    /*CONNOR*/
    /* Set appropriate values for MyResponse*/
    /* Find the sign-in token matching a user, UPDATE location, or ADD a location if one does not exist*/
    /* If a sign-in token does not exist, SET lattitude and longitude = 0,SET  myResponse = 0*/

//    signInToken = "dCcst6w?nF8pGh-Iiw0-JJvhGSpeZ-e3zgf0fJYAgQPM6Ufgi#dZGy!ejoqatu30\0";
    if (updateLocation(conn, signInToken, latitude.val, longitude.val) == 1){
        myResponse = '1';
    }

    if(signInToken)
        free(signInToken);

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleGetFriendLocation(char* myArray){
    int length = strlen(myArray+65);
    char* signInToken = (char *) (malloc(sizeof(char)*64));
    char* friendName = (char*) (malloc(sizeof(char)*length));
    char* myResult = (char *) (malloc(sizeof(char)*9));
    strncpy(signInToken, myArray + 1, 64);
    strncpy(signInToken, myArray + 65, length);
    union
    {
        char bytes[4];
        float val;
    } latitude;
    union
    {
        char bytes[4];
        float val;
    } longitude;
    latitude.val = 0;
    longitude.val = 0;
    char myResponse = 1;

    /*CONNOR*/
    /* SET appropriate values for MyResponse, longitude, and lattitude*/
    /* If the signin token matches the friend of the user, SET longitude and latitude, myResponse = 1*/
    /* If error, SET lattitude and longitude = 0, SET myResponse = 0*/
    myResult[0] = myResponse;
    latitude.val=reverseFloat(latitude.val);
    longitude.val=reverseFloat(longitude.val);
    strncpy(myResult+1,latitude.bytes,4);
    strncpy(myResult+5,longitude.bytes,4);

    if(signInToken)
        free(signInToken);
    if(friendName)
        free(friendName);
    return myResult;
}

char* handleGetFriendRequest(char* myArray){
    char* signInToken;
    char* result;
    int length;
    /*CONNOR I haven't done this yet, just write the query for the following:
     * You are given the signintoken. You must set two variables:
     * char[] result
     * int length
     *
     * For example if I have 2 friends (Garth and Matt)
     * result would be Garth\0Matt\0
     * and length would be 2
     */
}

char* handleSendFriendRequest(char* myArray){
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char friendName[len - TOKEN_SIZE];

    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - TOKEN_SIZE);
    /*CONNOR  Search the database for the username with the signin token provided
     * If friendName is not a friend of user, UPDATE Friend request table, SET myResponse = 1
     * If the friendName is already a friend of the user, SET MYRESPONSE =  0
     * If the friendName already has a friend request not accepted, SET MYRESPONSE = 2
     */
    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleAcceptFriendRequest(char* myArray){
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char friendName[len - TOKEN_SIZE];
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - TOKEN_SIZE);
    /* CONNOR Search the database for the username with the signin token provided
     * If friendName has an active friend request, accept the friend (update table), SET myResponse = 1
     * If the friendName did not send an active friend request, SET myResponse = 0
     * If the friendName is already a friend of the user, SET myResponse = 2 (i.e. Don't accept someone who's already your friend)
     */
    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleRemoveFriend(char* myArray){
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char friendName[len - TOKEN_SIZE];
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - TOKEN_SIZE);
    /* CONNOR Search the database for the username with the signin token provided
     * If friendName is a friend of user, REMOVE the friend, SET myResponse = 1
     * If the friendName is not a friend of the user, or sign-in token validation, SET MYRESPONSE= 0
     */
    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleOptions(char* myArray){
    char option = myArray[0];
    char* myReturn;
    if(option == CODE_SIGN_IN)
        myReturn = handleSignIn(myArray, NULL);
    if(option == CODE_LOC_GET)
//        myReturn = handleGetLocation(myArray);
    if(option == CODE_LOC_UPDATE)
//        myReturn = handleUpdateLocation(myArray);
    if(option == CODE_LOC_FRIEND)
        myReturn = handleGetFriendLocation(myArray);
    if(option == CODE_FRIEND_GET)
        myReturn = handleGetFriendRequest(myArray);
    if(option == CODE_FRIEND_SEND)
        myReturn = handleSendFriendRequest(myArray);
    if(option == CODE_FRIEND_ACCEPT)
        myReturn = handleAcceptFriendRequest(myArray);
    if(option == CODE_FRIEND_REMOVE)
        myReturn = handleRemoveFriend(myArray);
    return myReturn;

}

int findResponseLength(char* myArray){
    char option = myArray[0];
    int myReturn;
    if(option == CODE_SIGN_IN)
        myReturn = 65;
    if(option == CODE_LOC_GET)
        myReturn = 9;
    if(option == CODE_LOC_UPDATE)
        myReturn = 1;
    if(option == CODE_LOC_FRIEND)
        myReturn = 9;
    if(option == CODE_FRIEND_GET)
        myReturn = -1;
    if(option == CODE_FRIEND_SEND)
        myReturn = 1;
    if(option == CODE_FRIEND_ACCEPT)
        myReturn = 1;
    if(option == CODE_FRIEND_REMOVE)
        myReturn = 1;
    return myReturn;
}

char* generateToken(int length){
    srand(time(0));
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#?!";
    char *randomString = NULL;
    if (length) {
        randomString = (char*) malloc(sizeof(char) * (length));
        if (randomString) {
            for (int n = 0;n < length;n++) {
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }
        }
    }
    return randomString;
}

float reverseFloat(const float inFloat)
{
    float retVal;
    char *floatToConvert = ( char* ) & inFloat;
    char *returnFloat = ( char* ) & retVal;

    // swap the bytes into a temporary buffer
    returnFloat[0] = floatToConvert[3];
    returnFloat[1] = floatToConvert[2];
    returnFloat[2] = floatToConvert[1];
    returnFloat[3] = floatToConvert[0];

    return retVal;
}