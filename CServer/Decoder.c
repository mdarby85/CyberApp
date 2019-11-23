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

char* handleSignIn(char* myArray, PGconn *conn) {
    printf("The pointer points to %p\n",conn);
    int len = strlen(myArray+1);
    char *myUserName = (char *) malloc(sizeof(char) * len);
    char *myHashedPassword = (char *) malloc(sizeof(char) * 33);
    strncpy(myUserName, myArray + 1, len);
    strncpy(myHashedPassword, myArray + len + 2, 32);
    char myResponse = 1;
    myHashedPassword[32]='\0';
    printf("%s %s",myUserName,myHashedPassword);
    char query[500 + sizeof(myUserName) + sizeof(myHashedPassword) + TOKEN_SIZE];
    snprintf(query, sizeof(query), "SELECT * FROM people WHERE email = '%s' AND pwhash = '%.32s'", myUserName, myHashedPassword);
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
    signInToken[64] = '\0';
    printf("Hello %s\n", signInToken);
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
    printf("lat is %f\n",longitude.val);
    strncpy(signInToken, myArray + 1, 64);
    char myResponse = 0;
    if (updateLocation(conn, signInToken, latitude.val, longitude.val) == 1){
        myResponse = '1';
    }

    if(signInToken)
        free(signInToken);

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleGetFriendLocation(char* myArray, PGconn *conn){
    int length = strlen(myArray+65);
    char* signInToken = (char *) (malloc(sizeof(char)*64));
    char* friendName = (char*) (malloc(sizeof(char)*length));
    char* myResult = (char *) (malloc(sizeof(char)*9));
    strncpy(signInToken, myArray + 1, 64);
    strncpy(friendName, myArray + 65, length);
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
    printf("what's up\n");
    if (userEmail[0] == '\0'){
        return userEmail;
    } else if (strcmp(userEmail, friendName) == 0){
        myResponse = 1;
    }
    printf("My email is %s\n", userEmail);
    printf("My firend's email is %s\n",friendName);

    if (getFriendStatus(conn, userEmail, friendName) == IS_FRIEND){
	printf("YEP WE ARE FRIENDS\n");
        struct Position friendPosition = getPositionFromEmail(conn, friendName);
        latitude.val = friendPosition.lat;
        longitude.val = friendPosition.lon;
        myResponse = 49;
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

//TODO: handleGetFriendRequest
char* handleGetFriendRequest(PGconn *conn, char* myArray){
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
    int myResponse = 0;
    char* myToken = (char*) (malloc(sizeof(char)*64));
    int len = strlen(myArray+1);
    char* friendName = (char*) (malloc(sizeof(char)*(len - 64)));
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    printf("MY TOKEN IS %.64s\n",myToken);

    /*CONNOR  Search the database for the username with the signin token provided
     * If friendName is not a friend of user, UPDATE Friend request table, SET myResponse = 1
     * If the friendName is already a friend of the user, SET MYRESPONSE =  0
     * If the friendName already has a friend request not accepted, SET MYRESPONSE = 2
     */

    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    printf("MY EMAIL IS %s\n",userEmail);
    char* friendList = viewFriendRequests(conn,userEmail);
    union myInt{
	char bytes[4];
	int i;
    }myInt;
    int j;
    for(j=0;j<4;j++){
        myInt.bytes[j]=friendList[j];
    }
    printf("%d\n",myInt.i);
    if (myInt.i == 0){
        myResponse = 48;
    } else {
        myResponse = 49;
    }

    char* myReturn = (char*) (malloc(sizeof(char)*200));
    myReturn[0] = myResponse;
    int numNull=0;
    int index=4;
    printf("STARTING with i of %d\n",myInt.i);
    while(numNull < myInt.i){
     myReturn[index+1]=friendList[index];
     printf("%c",myReturn[index+1]);
     if(friendList[index] =='\0'){
         printf("NULL FOUND at index %d\n",index);
	 numNull++;
     }
	index++;
    }
    printf("\nFINISHED\n");
    for(index=1;index<5;index++){
         myReturn[index]=myInt.bytes[4-index];
    } 
    return myReturn;

}

char* handleSendFriendRequest(char* myArray, PGconn *conn){
    int myResponse = -1;
    char* myToken = (char*) (malloc(sizeof(char)*64));
    int len = strlen(myArray+1);
    char* friendName = (char*) (malloc(sizeof(char)*(len - 64)));

    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    strncpy(friendName, myArray + TOKEN_SIZE + 1, len - TOKEN_SIZE);
    printf("MY TOKEN IS %.64s\n",myToken);
    printf("my friendName is %s\n",friendName);
    /*CONNOR  Search the database for the username with the signin token provided
     * If friendName is not a friend of user, UPDATE Friend request table, SET myResponse = 1
     * If the friendName is already a friend of the user, SET MYRESPONSE =  0
     * If the friendName already has a friend request not accepted, SET MYRESPONSE = 2
     */

    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    printf("MY EMAIL IS %s\n",userEmail);
    int friendStatus = getFriendStatus(conn, userEmail, friendName);
    if (friendStatus == NOT_FRIEND){
        initFriendRequest(conn, userEmail, friendName);
        myResponse = 49;
    } else if (friendStatus == IS_FRIEND){
        myResponse = 48;
    } else if (friendStatus == PEND_FRIEND){
        myResponse = 50;
    } else {
        /* Default case, should not hit */
        return NULL;
    }

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleAcceptFriendRequest(char* myArray, PGconn *conn){
    printf("MYARRAY %s\n",myArray);
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char *friendName = (char*) (malloc(sizeof(char)*len-65));
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    printf("LENTH IS %d\n", len);
    strncpy(friendName, myArray + 64 + 1, len - 63);
    /* CONNOR Search the database for the username with the signin token provided
     * If friendName has an active friend request, accept the friend (update table), SET myResponse = 1
     * If the friendName did not send an active friend request, SET myResponse = 0
     * If the friendName is already a friend of the user, SET myResponse = 2 (i.e. Don't accept someone who's already your friend)
     */
    printf("Token %.64s\n",myToken);
    printf("friendName %s\n",friendName);
    char *userEmail = getEmailFromToken(conn, myToken);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    int friendStatus = getFriendStatus(conn, userEmail, friendName);
    if (friendStatus == PEND_FRIEND){
        if (acceptFriendRequest(conn, userEmail, friendName)){
            myResponse = 49;
        }
    } else if (friendStatus == NOT_FRIEND){
        myResponse = 48;
    } else if (friendStatus == IS_FRIEND){
        myResponse = 50;
    } else {
        /* Default case, should not hit */
        return NULL;
    }

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

char* handleRemoveFriend(char* myArray, PGconn *conn){
    int myResponse;
    char myToken[TOKEN_SIZE];
    int len = strlen(myArray+1);
    char *friendName = (char*) (malloc(sizeof(char)*len-65));
    strncpy(myToken, myArray + 1, TOKEN_SIZE);
    printf("LENTH IS %d\n", len);
    strncpy(friendName, myArray + 64 + 1, len - 63);
    /* CONNOR Search the database for the username with the signin token provided
     * If friendName is a friend of user, REMOVE the friend, SET myResponse = 1
     * If the friendName is not a friend of the user, or sign-in token validation, SET MYRESPONSE= 0
     */
    char *userEmail = getEmailFromToken(conn, myToken);
    printf("USEREMAIL %s\n", userEmail);
    printf("REMOVE FRIEND MY FRIEND'S NAME IS %s\n",friendName);
    if (userEmail[0] == '\0'){
        /*invalid token, return empty string */
        return NULL;
    }
    int friendStatus = getFriendStatus(conn, friendName, userEmail);
    if (friendStatus == IS_FRIEND){
        if (removeFriend(conn, userEmail, friendName)){
            myResponse = 49;
        }
    } else {
        myResponse = 48;
    }

    char* myReturn = (char*) (malloc(1));
    myReturn[0] = myResponse;
    return myReturn;
}

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

float reverseFloat(const float inFloat){
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
