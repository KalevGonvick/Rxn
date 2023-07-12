#include "Rxn.h"
#include<iostream>
#include<fstream>
#include<stdio.h>
#include <random>
#include <set>
#include <thread>

namespace Rxn::Core
{
    RxnBinaryHandler *RxnBinaryHandler::INSTANCE;

    RxnBinaryHandler::RxnBinaryHandler()
    {
        INSTANCE = this;
        INSTANCE->m_dRandomResult = Random();
    }

    RxnBinaryHandler::~RxnBinaryHandler()
    {
        delete Instance();
    }

    void RxnBinaryHandler::ReadRxnFile(const wchar_t *fileName, bool decrypt)
    {
        if (decrypt)
        {
            DecryptRxnFile(L"");
        }
    }

    void RxnBinaryHandler::WriteRxnFile(const wchar_t *fileName, const std::list<RXN_BASE_FILE> content, bool encrypt)
    {

    }

    void RxnBinaryHandler::ReadRxnEngineConfigFile()
    {

    }

    double RxnBinaryHandler::Random()
    {
        int seed = 20230525;
        std::mt19937 rng(seed);
        std::uniform_real_distribution<> param(-10, 10);
        double rand = param(rng);
        return rand;
    }

    bool RxnBinaryHandler::WriteRxnBundle(const wchar_t *fileName, std::list<RXN_BASE_FILE> content)
    {
        return false;
    }

    bool RxnBinaryHandler::WriteRxnSingleFile(const wchar_t *fileName, const RXN_BASE_FILE &content)
    {
        return false;
    }

    bool RxnBinaryHandler::EncryptRxnFile(const wchar_t *fileName)
    {
        char ch;
        std::fstream sourceFile, tempFile;

        sourceFile.open(fileName, std::fstream::in);

        if (!sourceFile)
        {
            RXN_LOGGER::Error(L"Error Occurred, Opening the Source File (to Read)!");
            return false;
        }

        tempFile.open("tmp.txt", std::fstream::out);

        if (!tempFile)
        {
            RXN_LOGGER::Error(L"Error Occurred, Opening/Creating the tmp File!");
            return false;
        }

        while (sourceFile >> std::noskipws >> ch)
        {
            //ch = ch + INSTANCE->m_dRandomResult;
            //tempFile << ch;
        }

        sourceFile.close();
        tempFile.close();
        sourceFile.open(fileName, std::fstream::out);

        if (!sourceFile)
        {
            RXN_LOGGER::Error(L"Error Occurred, Opening the Source File (to write)!");
            return false;
        }

        tempFile.open("tmp.txt", std::fstream::in);

        if (!tempFile)
        {
            RXN_LOGGER::Error(L"Error Occurred, Opening the tmp File!");
            return false;
        }

        while (tempFile >> std::noskipws >> ch)
        {
            sourceFile << ch;
        }

        sourceFile.close();
        tempFile.close();

        RXN_LOGGER::Trace(L"File '%s' was encrypted successfully.", fileName);

        return true;
    }

    bool RxnBinaryHandler::DecryptRxnFile(const wchar_t *fileName)
    {
        RXN_LOGGER::Debug(L"Random Result: %d", INSTANCE->m_dRandomResult);
        return true;
    }
}


