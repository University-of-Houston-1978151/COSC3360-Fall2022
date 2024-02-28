#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cmath>

double calcFx(double fx, double probability)
{
    double result = 0;
    result = fx + probability;

    return result;
}

struct symbolInfo
{
    std::string symbol;
    double probability;
    double fxRes;
    double fbarRes;
    std::string binary;
};

int getStringLengthWithoutSpaces(std::string str)
{
    int length = 0;
    for (int i = 0; i < str.length(); i++)
        if (str[i] != ' ')
            length++;

    return length;
}

void *threadThingy(void *myStruct)
{
    struct symbolInfo *mySymbolPtr = (struct symbolInfo *)myStruct;

    // Calculating fbar(x)
    mySymbolPtr->fbarRes = (mySymbolPtr->probability / 2) + (mySymbolPtr->fxRes - mySymbolPtr->probability);

    // Calculating length(x)
    int length = ceil(log2(1 / mySymbolPtr->probability) + 1);

    // Calculating binary
    double result = 0;
    double fbar = mySymbolPtr->fbarRes;
    for (int i = 0; i < length; i++)
    {
        result = fbar * 2;

        if (result >= 1)
        {
            mySymbolPtr->binary += "1";
            result = result - 1;
            fbar = result;
        }
        else
        {
            mySymbolPtr->binary += "0";
            fbar = result;
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    std::vector<symbolInfo> symVec;

    std::string symbols;
    double probability;
    std::getline(std::cin, symbols);
    int symLen = getStringLengthWithoutSpaces(symbols); // symLen = # threads to create
    pthread_t tid[symLen];

    // Store symbols in struct/vector
    for (int i = 0; i < symbols.length(); i++)
    {
        if (symbols[i] != ' ')
        {
            symbolInfo mySymbol;
            mySymbol.symbol = symbols[i];
            symVec.push_back(mySymbol);
        }
    }

    for (int i = 0; i < symLen; i++)
    {
        // Storing probability
        std::cin >> probability;
        symVec.at(i).probability = probability;

        // Storing f(x)
        if (i == 0)
        {
            symVec.at(i).fxRes = calcFx(0, symVec.at(i).probability);
        }
        else
        {
            symVec.at(i).fxRes = calcFx(symVec.at(i - 1).fxRes, symVec.at(i).probability);
        }
    }

    // Use Threads
    for (int i = 0; i < symLen; i++)
    {
        pthread_create(&tid[i], nullptr, threadThingy, &symVec.at(i));
    }
    for (int i = 0; i < symLen; i++)
    {
        pthread_join(tid[i], nullptr);
    }

    std::cout << "SHANNON-FANO-ELIAS Codes:" << std::endl;
    for (int i = 0; i < symVec.size(); i++)
    {
        std::cout << "Symbol " << symVec.at(i).symbol;
        std::cout << ", Code: " << symVec.at(i).binary;
        std::cout << std::endl;
    }

    return 0;
}