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

        void Init(std::wstring filename, UINT filesize = DefaultFileSize);
        void Destroy(bool deleteFile);
        void GrowMapping(UINT size);
        void SetSize(UINT size);

        // TODO: change all the void* s in mapped caching to unsigned char* s
        void *GetData();
        UINT GetSize() const;

    protected:

        static const UINT DefaultFileSize = 64;

        HANDLE m_MapFile;
        HANDLE m_File;

        LPVOID m_MapAddress;

        WString m_Filename;
        UINT m_currentFileSize;
    };
}
