#include <stdio.h>
#include <string.h>	//strlen
#include <stdlib.h>	//strlen
#include <sys/socket.h>
#include <unistd.h>	//write
#include <netinet/in.h>
#include <libpq-fe.h>
#include <time.h>
#include "Decoder.h"
#include "Database.h"
#include <pthread.h> //for threading , link with lpthread
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
	char *message;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket\n");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *) &server , sizeof(server)) < 0)
	{
		return 1;
	}
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
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
		
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
	
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
    char *array = "0username\012345678901234567890123456789012";

	//Get the socket descriptor
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
		printf("Response: %d\n",myResponse[0]);
		int responseLength = findResponseLength(buff);
        bzero(buff, MAX);
        n = 0;
        /* and send that buffer to client */
        write(sock, myResponse, sizeof(char)*responseLength);
    }
		
	//Free the socket pointer
	free(socket_desc);
	
	return 0;
}
