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

void *connection_handler(void *);

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
    const char *conninfo = "user=postgres password=postgres dbname = cyberdb";
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
