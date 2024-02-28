// Derived from Professor Carlos Rincon's server.cpp

#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/wait.h>
#include <cmath>
#include <sstream>

void fireman(int)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int probN, cumProbN, binaryN;

    signal(SIGCHLD, fireman);
    if (argc < 2)
    {
        std::cerr << "ERROR, no port provided\n";
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR on binding";
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0)
        {
            if (newsockfd < 0)
            {
                std::cerr << "ERROR on accept";
                exit(1);
            }

            double prob;
            probN = read(newsockfd, &prob, sizeof(double));
            if (probN < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }
            double fProb;
            cumProbN = read(newsockfd, &fProb, sizeof(double));
            if (cumProbN < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }

            double fbar = (prob / 2) + (fProb - prob);

            std::string binary = "";
            int length = ceil(log2(1 / prob) + 1);
            double result = 0;
            for (int i = 0; i < length; i++)
            {
                result = fbar * 2;
                if (result >= 1)
                {
                    binary += "1";
                    result = result - 1;
                    fbar = result;
                }
                else
                {
                    binary += "0";
                    fbar = result;
                }
            }

            char binaryArr[binary.length()];
            strcpy(binaryArr, binary.c_str());

            int bMessage = strlen(binaryArr);
            binaryN = write(newsockfd, &bMessage, sizeof(int));
            if (binaryN < 0)
            {
                std::cerr << "ERROR writing to socket";
                exit(1);
            }
            binaryN = write(newsockfd, binaryArr, bMessage);
            if (binaryN < 0)
            {
                std::cerr << "ERROR writing to socket";
                exit(1);
            }
            close(newsockfd);

            _exit(0);
        }
    }
    close(sockfd);
    return 0;
}
