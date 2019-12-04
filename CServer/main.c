/* 
 * file: main.c
 * author: group 2
 * Date Last modified: 11/23/19
 * Compile with gcc -lpq -lpthread *.c
 */
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>	
#include <sys/socket.h>
#include <unistd.h>	
#include <netinet/in.h>
#include <libpq-fe.h>
#include <time.h>
#include "Decoder.h"
#include "Database.h"
#include <pthread.h>

#define MAX 800
#define PORT 12348

struct ConnectionInfo {
    char database[500];
    char user[500];
    char password[500];
    char host[500];
    char port[500];
};

void *connection_handler(void *);
void getDBConnectionInfo(char *connectionstr);

int main(int argc , char *argv[])
{
    printf("Starting Server\n");
    printf("Timestamp: %d\n",(int)time(NULL));
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("Current Time : %s\n", time_str);
	
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
	
    /*Create socket*/
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
	 printf("Could not create socket\n");
    }
	
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( PORT );
	
    /*Bind*/
    if( bind(socket_desc,(struct sockaddr *) &server , sizeof(server)) < 0)
    {
	 return 1;
    }
	
    listen(socket_desc , 3);
	
    /*Accept connection, create thread*/
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) &c)) )
        {
	    pthread_t sniffer_thread;
	    new_sock = malloc(1);
	    *new_sock = new_socket;
	    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
	    {
	        return 1;
	    }
	    pthread_join( sniffer_thread , NULL);
	}
	
	if (new_socket < 0)
	{
	    return 1;
	}	
	return 0;
}

void *connection_handler(void *socket_desc)
{
    /* Database Variables */
    char connectionStr[1000];
    getDBConnectionInfo(connectionStr);

    const char *conninfo = "user=postgres password=supersecretdbpassword1! dbname=cyberdb";
    PGconn *conn = connectToDB(conninfo);

    /*Get the socket descriptor*/
    int sock = *(int*)socket_desc;
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, MAX);
        n= read(sock, buff, sizeof(buff));
	if(n==0)
	   break;
        if(buff[0] == 8)
            break;
        char* myResponse;
        myResponse = handleOptions(buff, conn);
	int responseLength = findResponseLength(buff);
        bzero(buff, MAX);
        n = 0;
        /* and send that buffer to client */
        write(sock, myResponse, sizeof(char)*responseLength);
    }
		
    free(socket_desc);
    return 0;
}

void getDBConnectionInfo(char *connectionstr){
    struct ConnectionInfo connectionInfo;
    /* Get parent folder and append DBInfo folder to path */
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char* ptr = strrchr(cwd, '/');
        *(ptr + 1) = '\0';
        strncat(cwd,"DBInfo/",100);
    } else {
        perror("getcwd() error");
    }

    /* Open and read secret file */
    strncat(cwd, "secret.txt", 20);
    FILE *fp;
    fp = fopen(cwd, "r");
    if (fp == NULL){
        printf("Could not open secrets file! %s", cwd);
    } else {
        char line[500];

        /* Get DB Name */
        {
            fgets(line, sizeof(line), fp);
            char *ptr = strrchr(line, ':');
            strncpy(connectionInfo.database, ptr + 1,
                    sizeof(connectionInfo.database));
            for (int i = 0; i < strlen(connectionInfo.database); i++) {
                if (connectionInfo.database[i] == '\r' ||
                    connectionInfo.database[i] == '\n') {
                    connectionInfo.database[i] = '\0';
                }
            }
        }

        /* Get User */
        {
            fgets(line, sizeof(line), fp);
            char *ptr = strrchr(line, ':');
            strncpy(connectionInfo.user, ptr + 1,
                    sizeof(connectionInfo.user));
            for (int i = 0; i < strlen(connectionInfo.user); i++) {
                if (connectionInfo.user[i] == '\r' ||
                    connectionInfo.user[i] == '\n') {
                    connectionInfo.user[i] = '\0';
                }
            }
        }
        /* Get Password */
        {
            fgets(line, sizeof(line), fp);
            char *ptr = strrchr(line, ':');
            strncpy(connectionInfo.password, ptr + 1,
                    sizeof(connectionInfo.password));
            for (int i = 0; i < strlen(connectionInfo.password); i++) {
                if (connectionInfo.password[i] == '\r' ||
                    connectionInfo.password[i] == '\n') {
                    connectionInfo.password[i] = '\0';
                }
            }
        }

        /* Get Host */
        {
            fgets(line, sizeof(line), fp);
            char *ptr = strrchr(line, ':');
            strncpy(connectionInfo.host, ptr + 1,
                    sizeof(connectionInfo.host));
            for (int i = 0; i < strlen(connectionInfo.host); i++) {
                if (connectionInfo.host[i] == '\r' ||
                    connectionInfo.host[i] == '\n') {
                    connectionInfo.host[i] = '\0';
                }
            }
        }

        /* Get Port */
        {
            fgets(line, sizeof(line), fp);
            char *ptr = strrchr(line, ':');
            strncpy(connectionInfo.port, ptr + 1,
                    sizeof(connectionInfo.port));
            for (int i = 0; i < strlen(connectionInfo.port); i++) {
                if (connectionInfo.port[i] == '\r' ||
                    connectionInfo.port[i] == '\n') {
                    connectionInfo.port[i] = '\0';
                }
            }
        }

        /* Build Connection String */
        strncpy(connectionstr, "dbname=", 7);
        strncat(connectionstr, connectionInfo.database, strlen(connectionInfo.database));

        strncat(connectionstr, " user=", 6);
        strncat(connectionstr, connectionInfo.user, strlen(connectionInfo.user));

        strncat(connectionstr, " password=", 10);
        strncat(connectionstr, connectionInfo.password, strlen(connectionInfo.password));

        strncat(connectionstr, " host=", 6);
        strncat(connectionstr, connectionInfo.host, strlen(connectionInfo.host));

        strncat(connectionstr, " port=", 6);
        strncat(connectionstr, connectionInfo.port, strlen(connectionInfo.port));
    }
    fclose(fp);
}
