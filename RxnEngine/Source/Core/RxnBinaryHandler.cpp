#include "Rxn.h"
#include "RxnBinaryHandler.h"

namespace Rxn::Core
{
    inline static const RxnBinaryHandler Instance;

    RxnBinaryHandler::RxnBinaryHandler() = default;
    RxnBinaryHandler::~RxnBinaryHandler() = default;

    void RxnBinaryHandler::WriteRxnFile(const String &outputFile)
    {
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open())
        {
            RXN_LOGGER::Error(L"Unable to open file for writing...");
            throw std::runtime_error("");
        }

        RxnVertexEntry vertexEntry;
        vertexEntry.vertex = VertexPositionColour{ { -1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ {  1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ {  1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ { -1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ { -1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ {  1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ {  1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);
        vertexEntry.vertex = VertexPositionColour{ { -1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } };
        WriteEntry(outFile, RXN_VERTEX_TYPE, vertexEntry);

        RxnIndexEntry indexEntry;
        indexEntry.index = 0;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 1;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 3;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        indexEntry.index = 1;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 2;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 3;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);


        indexEntry.index = 3;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 2;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 7;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        indexEntry.index = 6;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 7;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 2;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);


        indexEntry.index = 2;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 1;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 6;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        indexEntry.index = 5;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 6;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 1;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);


        indexEntry.index = 1;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 0;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 5;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        indexEntry.index = 4;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 5;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 0;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);


        indexEntry.index = 0;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 3;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 4;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        indexEntry.index = 7;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 4;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 3;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);


        indexEntry.index = 7;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 6;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 4;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        indexEntry.index = 5;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 4;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);
        indexEntry.index = 6;
        WriteEntry(outFile, RXN_INDEX_TYPE, indexEntry);

        outFile.close();


    }

    void RxnBinaryHandler::ReadRxnFile(std::vector<VertexPositionColour> &vertexData, std::vector<uint32> &indexData, const String &inputFile)
    {
        std::ifstream inFile(inputFile, std::ios::binary);

        if (!inFile.is_open()) {
            RXN_LOGGER::Error(L"Cannot open input file...");
            return;
        }

        while (true)
        {
            char type;
            inFile.read(&type, sizeof(char));

            if (inFile.eof()) {
                // End of file reached
                break;
            }
            
            switch (type)
            {
            case RXN_VERTEX_TYPE: {
                RxnVertexEntry entry;
                inFile.read(std::bit_cast<char *>(&entry), sizeof(RxnVertexEntry));

                vertexData.emplace_back(entry.vertex);
                break;
            }
            case RXN_INDEX_TYPE: {
                RxnIndexEntry entry;
                inFile.read(std::bit_cast<char *>(&entry), sizeof(RxnIndexEntry));
                indexData.emplace_back(entry.index);
                break;
            }
            case RXN_QUAD_TYPE: {
                RxnQuadEntry entry;
                inFile.read(std::bit_cast<char *>(&entry), sizeof(RxnQuadEntry));
                // TODO
                break;
            }
                
            default:
                throw std::runtime_error("");
            }
        }

        inFile.close();
    }

 
}


