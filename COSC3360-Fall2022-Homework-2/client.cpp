// Derived from Professor Carlos Rincon's client.cpp

#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <map>
#include <algorithm>

struct symbolInfo
{
    std::string symbol, binary, hostname;
    double probability, fxRes;
    int portno;
};

double calculateFx(double fx, double probability)
{
    double result = 0;
    result = fx + probability;

    return result;
}

void *threadThingy(void *myStruct)
{
    struct symbolInfo *mySymbolPtr = (struct symbolInfo *)myStruct;

    int sockfd, portno, probN, cumProbN, binaryN;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = mySymbolPtr->portno;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
    }

    server = gethostbyname(mySymbolPtr->hostname.c_str());
    if (server == NULL)
    {
        std::cerr << "ERROR, no such host\n";
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR connecting";
        exit(1);
    }

    double prob;
    prob = mySymbolPtr->probability;
    probN = write(sockfd, &prob, sizeof(double));
    if (probN < 0)
    {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }
    double fProb;
    fProb = mySymbolPtr->fxRes;
    cumProbN = write(sockfd, &fProb, sizeof(double));
    if (cumProbN < 0)
    {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }

    int bSize;
    binaryN = read(sockfd, &bSize, sizeof(int));
    if (binaryN < 0)
    {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }

    char *bBuffer = new char[bSize + 1];
    bzero(bBuffer, bSize + 1);
    binaryN = read(sockfd, bBuffer, bSize);
    
    mySymbolPtr->binary = bBuffer;

    delete[] bBuffer;

    close(sockfd);

    return NULL;
}

int main(int argc, char *argv[])
{
    std::vector<symbolInfo> symVec;

    if (argc < 3)
    {
        std::cerr << "usage " << argv[0] << "hostname port\n";
        exit(0);
    }

    std::string input;
    std::getline(std::cin, input);

    std::map<char, double> freqMap;
    for (auto i : input)
    {
        freqMap[i]++;
    }

    for (auto const &x : freqMap)
    {
        symbolInfo mySymbol;
        mySymbol.symbol = x.first;
        mySymbol.probability = x.second / input.size();
        mySymbol.hostname = argv[1];
        mySymbol.portno = atoi(argv[2]);
        symVec.push_back(mySymbol);
    }

    for (int i = 0; i < symVec.size(); i++)
    {
        if (i == 0)
        {
            symVec.at(i).fxRes = calculateFx(0, symVec.at(i).probability);
        }
        else
        {
            symVec.at(i).fxRes = calculateFx(symVec.at(i - 1).fxRes, symVec.at(i).probability);
        }
    }

    int nthreads = symVec.size();
    pthread_t tid[nthreads];

    for (int i = 0; i < nthreads; i++)
    {
        pthread_create(&tid[i], nullptr, threadThingy, &symVec.at(i));
    }
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    std::cout << "SHANNON-FANO-ELIAS Codes:" << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < nthreads; i++)
    {
        std::cout << "Symbol " << symVec.at(i).symbol;
        std::cout << ", Code: " << symVec.at(i).binary;
        std::cout << std::endl;
    }

    return 0;
}
