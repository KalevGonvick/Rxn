#pragma once
#include <list>

namespace Rxn::Core
{
    class RXN_ENGINE_API RxnBinaryData
    {
    public:

        RxnBinaryData() = delete;
        explicit RxnBinaryData(uint32 uuid) : m_UUID(uuid) {};
        ~RxnBinaryData() = default;

    public:

        virtual void WriteFile() = 0;
        virtual RxnBinaryData ReadFile() = 0;

    private:

        uint32 m_UUID;
    };

    struct RxnBinaryFormat
    {
        const std::array<uint8, 4> MagicNumber = { 0x72, 0x78, 0x6e, 0x62, };
        std::vector<RxnBinaryData> Data;
    };
    
    class RXN_ENGINE_API RxnBinaryHandler
    {
    public:

        RxnBinaryHandler();
        ~RxnBinaryHandler();

    public:

        static void WriteRxnFile(const String &outputFile);
        static void ReadRxnFile(std::vector<VertexPositionColour> &vertexData, std::vector<uint32> &indexData, const String &inputFile);

    private:

        inline static float GetRandomColour()
        {
            return (rand() % 100) / 100.0f;
        }

        template<typename T>
        inline static void ReadEntry(std::ifstream &in, T &outData)
        {
            in.read(reinterpret_cast<char *>(&outData), sizeof(T));
        }

        template<typename T>
        inline static void WriteEntry(std::ofstream &out, char type, const T &entry)
        {
            out.write(&type, sizeof(char));
            out.write(reinterpret_cast<const char*>(&entry), sizeof(T));
        }

    };
    
}