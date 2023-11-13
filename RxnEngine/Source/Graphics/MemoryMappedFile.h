#pragma once
#include <iostream>

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedFile
    {
    protected:

        MemoryMappedFile();
        ~MemoryMappedFile();

    public:

        bool IsMapped() const;

    protected:

        void InitFile(const WString &fileName, uint32 filesize = DefaultFileSize);
        void DestroyFile(bool deleteFile);
        void GrowMapping(uint32 size);
        void SetSize(uint32 size);

        void *GetData();
        uint32 GetSize() const;

        uint32 GetCurrentFileSize() const;


    private:

        static const uint32 DefaultFileSize = 64;

        HANDLE m_MapFile = INVALID_HANDLE_VALUE;
        HANDLE m_File = INVALID_HANDLE_VALUE;

        LPVOID m_MapAddress = nullptr;

        WString m_Filename;
        uint32 m_currentFileSize = 0;
    };
}
