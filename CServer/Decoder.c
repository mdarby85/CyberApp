/*
* filename:Decoder.c
* author: group2
* Date Last modified: 11/23/19
*/
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
static int PASSWORD_HASHSIZE=32;

/*
 * Checks if a user can be authenticated
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleSignIn(char* myArray, PGconn *conn) {
    int len = strlen(myArray+1);
    char *myUserName = (char *) malloc(sizeof(char) * len);
    char *myHashedPassword = (char *) malloc(sizeof(char) * (PASSWORD_HASHSIZE+1));
    strncpy(myUserName, myArray + 1, len);
    strncpy(myHashedPassword, myArray + len + 2, PASSWORD_HASHSIZE);
    char myResponse = 1;
    myHashedPassword[PASSWORD_HASHSIZE]='\0';
    char query[500 + sizeof(myUserName) + sizeof(myHashedPassword) + TOKEN_SIZE];
    myUserName[len]='\0';
    printf("\n%s\n",myHashedPassword);
    snprintf(query, sizeof(query), "SELECT * FROM people WHERE email = '%s' AND pwhash = '%.32s'", myUserName, myHashedPassword);
    printf("%s\n",query);
    query[strlen(query)]='\0';
    /* Is the number of rows returned > 0? */
    myResponse = (processQuery(conn, query) == 0 ? 0 : 1);
    char *serverResponse = (char *) malloc(sizeof(char) * (TOKEN_SIZE+1));
    char* token = (char *) malloc(sizeof(char) * TOKEN_SIZE);
    strncpy(token, generateToken(TOKEN_SIZE), TOKEN_SIZE);
    strncpy(serverResponse + 1, token, TOKEN_SIZE);
    serverResponse[0] = myResponse;
    if (myResponse == 1) {
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

/*
 * Gets the location of a user
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleGetLocation(char* myArray, PGconn *conn){
    char* signInToken = (char *) (malloc(sizeof(char)*(TOKEN_SIZE+1)));
    char* myResult = (char *) (malloc(sizeof(char)*9));
    strncpy(signInToken, myArray + 1, TOKEN_SIZE);
    signInToken[TOKEN_SIZE] = '\0';

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
    struct Position position = getPositionFromToken(conn, signInToken);
    latitude.val = position.lat;
    longitude.val = position.lon;
    myResponse = position.response;
    myResult[0] = myResponse;
    latitude.val=reverseFloat(latitude.val);
    longitude.val=reverseFloat(longitude.val);
    strncpy(myResult+1,latitude.bytes,4);
    strncpy(myResult+5,longitude.bytes,4);
    return myResult;
}

/*
 * Updates the location of a user to the database
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleUpdateLocation(char* myArray, PGconn *conn){
    char* signInToken = (char *) (malloc(sizeof(char)*TOKEN_SIZE));
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
    strncpy(latitude.bytes,myArray+TOKEN_SIZE+1, 4);
    strncpy(longitude.bytes,myArray+TOKEN_SIZE+5, 4);
    latitude.val = reverseFloat(latitude.val);
    longitude.val = reverseFloat(longitude.val);
    strncpy(signInToken, myArray + 1, TOKEN_SIZE);
    char myResponse = 0;
    if (updateLocation(conn, signInToken, latitude.val, longitude.val) == 1){
        myResponse = 1;
    }

    if(signInToken)
        free(signInToken);

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

/*
 * Gets the location a friend
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleGetFriendLocation(char* myArray, PGconn *conn){
    int length = strlen(myArray+TOKEN_SIZE+1);
    char* signInToken = (char *) (malloc(sizeof(char)*TOKEN_SIZE));
    char* friendName = (char*) (malloc(sizeof(char)*length));
    char* myResult = (char *) (malloc(sizeof(char)*9));
    strncpy(signInToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE+1, length);
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
    char myResponse = 0;
    char *userEmail = getEmailFromToken(conn, signInToken);
    /* Returns empty string if no email is associated with a token. */
    if (userEmail[0] == '\0'){
        return userEmail;
    } else if (strcmp(userEmail, friendName) == 0){
        myResponse = 1;
    }
    if (getFriendStatus(conn, userEmail, friendName) == IS_FRIEND){
        struct Position friendPosition = getPositionFromEmail(conn, friendName);
        latitude.val = friendPosition.lat;
        longitude.val = friendPosition.lon;
        myResponse = 1;
    }
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

/*
 * Gets all friend requests
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleGetFriendRequest(PGconn *conn, char* myArray){
    char* signInToken;
    char* result;
    int length;
    int myResponse = 0;
    char* myToken = (char*) (malloc(sizeof(char)*TOKEN_SIZE));
    int len = strlen(myArray+1);
    char* friendName = (char*) (malloc(sizeof(char)*(len - TOKEN_SIZE)));
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    char* friendList = viewFriendRequests(conn,userEmail);
    union myInt{
	char bytes[4];
	int i;
    } myInt;
    int j;
    for(j=0;j<4;j++){
        myInt.bytes[j]=friendList[j];
    }
    if (myInt.i == 0){
        myResponse = 0;
    } else {
        myResponse = 1;
    }

    char* myReturn = (char*) (malloc(sizeof(char)*200));
    myReturn[0] = myResponse;
    int numNull=0;
    int index=4;
    while(numNull < myInt.i){
     myReturn[index+1]=friendList[index];
     if(friendList[index] =='\0'){
	 numNull++;
     }
	index++;
    }
    for(index=1;index<5;index++){
         myReturn[index]=myInt.bytes[4-index];
    }
    if(friendName)
	 free(friendName);
    if(myToken)
	 free(myToken);
    return myReturn;
}

/*
 * Send a friend request
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleSendFriendRequest(char* myArray, PGconn *conn){
    int myResponse = -1;
    char* myToken = (char*) (malloc(sizeof(char)*TOKEN_SIZE));
    int len = strlen(myArray+1);
    char* friendName = (char*) (malloc(sizeof(char)*(len - TOKEN_SIZE)));

    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - TOKEN_SIZE);
  
    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    int friendStatus = getFriendStatus(conn, userEmail, friendName);
    if (friendStatus == NOT_FRIEND){
        initFriendRequest(conn, userEmail, friendName);
        myResponse = 1;
    } else if (friendStatus == IS_FRIEND){
        myResponse = 0;
    } else if (friendStatus == PEND_FRIEND){
        myResponse = 2;
    } else {
        /* Default case, should not hit */
        return NULL;
    }
    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

/*
 * Accept a friend request
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleAcceptFriendRequest(char* myArray, PGconn *conn){
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char *friendName = (char*) (malloc(sizeof(char)*len-(TOKEN_SIZE+1)));
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - (TOKEN_SIZE-1));
    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    int friendStatus = getFriendStatus(conn, userEmail, friendName);
    if (friendStatus == PEND_FRIEND){
        if (acceptFriendRequest(conn, userEmail, friendName)){
            myResponse = 1;
        }
    } else if (friendStatus == NOT_FRIEND){
        myResponse = 0;
    } else if (friendStatus == IS_FRIEND){
        myResponse = 2;
    } else {
        /* Default case, should not hit */
        return NULL;
    }

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

/*
 * Remove a friend
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleRemoveFriend(char* myArray, PGconn *conn){
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char *friendName = (char*) (malloc(sizeof(char)*len-(TOKEN_SIZE+1)));
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - (TOKEN_SIZE-1));
    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    int friendStatus = getFriendStatus(conn, friendName, userEmail);
    if (friendStatus == IS_FRIEND){
        if (removeFriend(conn, userEmail, friendName)){
            myResponse = 1;
        }
    } else {
        myResponse = 0;
    }

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

/*
 * Handle options based on the first byte of the client response
 * param: Encoded Message
 * param: Connection to database
 * return: Server Response
 */
char* handleOptions(char* myArray, PGconn *conn){
    char option = myArray[0];
    char* myReturn;
    if(option == CODE_SIGN_IN)
        myReturn = handleSignIn(myArray, conn);
    if(option == CODE_LOC_GET)
        myReturn = handleGetLocation(myArray, conn);
    if(option == CODE_LOC_UPDATE)
        myReturn = handleUpdateLocation(myArray, conn);
    if(option == CODE_LOC_FRIEND)
        myReturn = handleGetFriendLocation(myArray, conn);
    if(option == CODE_FRIEND_GET)
        myReturn = handleGetFriendRequest(conn, myArray);
    if(option == CODE_FRIEND_SEND)
        myReturn = handleSendFriendRequest(myArray, conn);
    if(option == CODE_FRIEND_ACCEPT)
        myReturn = handleAcceptFriendRequest(myArray, conn);
    if(option == CODE_FRIEND_REMOVE)
        myReturn = handleRemoveFriend(myArray, conn);
    return myReturn;

}

/*
 * Find the mapping of code to expected response length
 * param: Encoded Message
 * return: The expected length of the response
 */
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
        myReturn = 200;
    if(option == CODE_FRIEND_SEND)
        myReturn = 1;
    if(option == CODE_FRIEND_ACCEPT)
        myReturn = 1;
    if(option == CODE_FRIEND_REMOVE)
        myReturn = 1;
    return myReturn;
}

/*
 * Generates a random token
 * param: length of the token
 * return: Random String of the token
 */
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

/*
 * Reverse the float (C-Java encoding)
 * param: float to be reversed
 * return: Reversed Float
 */
float reverseFloat(const float inFloat){
    float retVal;
    char *floatToConvert = (char*) & inFloat;
    char *returnFloat = (char*) & retVal;

    /* swap the bytes into a temporary buffer*/
    returnFloat[0] = floatToConvert[3];
    returnFloat[1] = floatToConvert[2];
    returnFloat[2] = floatToConvert[1];
    returnFloat[3] = floatToConvert[0];

    return retVal;
}
