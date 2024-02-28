#include <iostream>
#include <pthread.h>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <cmath>

struct bigStructBoi
{
    std::string symbol, binary;
    double px, fx, fbarx;
    int threadNumber, nthreads;
    int *turn;
    pthread_mutex_t *bsem;
    pthread_cond_t *waitTurn;
};

void *this_homework_is_painful(void *symbolVoidPtr)
{
    bigStructBoi *symbolStructPtr = (bigStructBoi *)symbolVoidPtr;

    // Check turn, else wait
    while (*symbolStructPtr->turn != symbolStructPtr->threadNumber)
    {
        pthread_cond_wait(symbolStructPtr->waitTurn, symbolStructPtr->bsem);
    }

    std::string symbol = symbolStructPtr->symbol;
    double px, fx, fbarx;
    px = symbolStructPtr->px;
    fx = symbolStructPtr->fx;
    fbarx = symbolStructPtr->fbarx;

    (*symbolStructPtr->turn)++;                  // Turn + 1
    pthread_mutex_unlock(symbolStructPtr->bsem); // Unlocked

    // Do calculations here

    // Calculate cumulative probability
    symbolStructPtr->fx += symbolStructPtr->px;
    // Calculate fbar(x)
    symbolStructPtr->fbarx = (symbolStructPtr->px / 2) + (symbolStructPtr->fx - symbolStructPtr->px);
    // Calculate length(x)
    int length = ceil(log2(1 / symbolStructPtr->px) + 1);
    // Calculate binary
    std::string binary = "";
    double result;
    double binaryfBar = symbolStructPtr->fbarx;
    for (int i = 0; i < length; i++)
    {
        result = binaryfBar * 2;

        if (result >= 1)
        {
            binary += "1";
            result--;
            binaryfBar = result;
        }
        else
        {
            binary += "0";
            binaryfBar = result;
        }
    }
    symbolStructPtr->binary = binary;

    // Print all
    std::cout << "Symbol " << symbol;
    std::cout << ", Code: " << symbolStructPtr->binary << std::endl;

    pthread_cond_broadcast(symbolStructPtr->waitTurn); // Broadcast

    return NULL;
}

int main()
{
    std::string input;
    std::getline(std::cin, input);

    std::map<char, double> frequencyMap; // Create map to store frequency
    for (auto i : input)
    {
        frequencyMap[i]++;
    }

    int nthreads = frequencyMap.size(); // Create NTHREADS

    static pthread_mutex_t bsem;                               // Semaphore
    static pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER; // Turn
    int turn = 0;
    pthread_t tid[nthreads];
    pthread_mutex_init(&bsem, NULL);

    bigStructBoi symbolStruct; // Create struct

    symbolStruct.bsem = &bsem;
    symbolStruct.waitTurn = &waitTurn;
    symbolStruct.turn = &turn;
    symbolStruct.fx = 0;
    // symbolStruct.inputLength = input.size();
    symbolStruct.binary = "";
    symbolStruct.nthreads = nthreads;

    // Create vector to store info from map
    std::vector<bigStructBoi> symbolVector;
    for (auto const &mapElem : frequencyMap)
    {
        bigStructBoi symbolStructVec;
        symbolStructVec.symbol = mapElem.first;
        symbolStructVec.px = mapElem.second / input.size();

        symbolVector.push_back(symbolStructVec);
    }

    std::cout << "SHANNON-FANO-ELIAS Codes:\n\n";

    for (int i = 0; i < nthreads; i++)
    {
        pthread_mutex_lock(&bsem); // Crit sec
        symbolStruct.threadNumber = i;

        symbolStruct.symbol = symbolVector.at(i).symbol;
        symbolStruct.px = symbolVector.at(i).px;

        if (pthread_create(&tid[i], NULL, this_homework_is_painful, &symbolStruct))
        {
            std::cerr << "Error creating thread\n";
            return 1;
        }
    }

    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
