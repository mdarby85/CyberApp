#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <time.h>
#include "Decoder.h"
#include "Database.h"

#define MAX 800
#define PORT 12347
#define SA struct sockaddr

void handleAccept(int);


int main()
{
    printf("Timestamp: %d\n",(int)time(NULL));
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("Current Time : %s\n", time_str);


    /* Database Variables */
    const char *conninfo = "user=postgres password=put_your_password_here dbname = cyberdb";
    PGconn *conn = connectToDB(conninfo);

    char *array = "0username\012345678901234567890123456789012";

    handleUpdateLocation(array, conn);


    exit(0);

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed...\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));

    /* assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    /* Binding newly created socket to given IP and verification*/
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        perror("socket bind failed...\n");
        exit(0);
    }

    /* Now server is ready to listen and verification*/
    if ((listen(sockfd, 5)) != 0) {
        perror("Listen failed...\n");
        exit(0);
    }

    len = sizeof(cli);
    while(1) {
        /* Accept the data packet from client and verification*/
        connfd = accept(sockfd, (SA *) &cli, &len);
        if (connfd < 0) {
            perror("server accept failed...\n");
            exit(0);
        }

        handleAccept(connfd);
    }
    close(sockfd);
    closeDBConnection(conn);
}

void handleAccept(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, MAX);

        read(sockfd, buff, sizeof(buff));
        if(buff[0] == 8)
            break;
        char* myResponse;
        myResponse = handleOptions(&buff);
        int responseLength = findResponseLength(&buff);
        bzero(buff, MAX);
        n = 0;
        /* and send that buffer to client */
        write(sockfd, myResponse, sizeof(char)*responseLength);
    }
}