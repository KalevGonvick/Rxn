#include "Rxn.h"
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    MemoryMappedFile::MemoryMappedFile()
        : m_MapFile(INVALID_HANDLE_VALUE)
        , m_File(INVALID_HANDLE_VALUE)
        , m_MapAddress(nullptr)
        , m_currentFileSize(0)
    {}

    bool MemoryMappedFile::IsMapped() const
    {
        return m_MapAddress != nullptr;
    }

    MemoryMappedFile::~MemoryMappedFile() = default;

    void MemoryMappedFile::Init(std::wstring fileName, UINT fileSize)
    {
        m_Filename = fileName;

        WIN32_FIND_DATA findFileData;
        HANDLE handle = FindFirstFileEx(fileName.c_str(), FindExInfoBasic, &findFileData, FindExSearchNameMatch, nullptr, 0);

        bool found = handle != INVALID_HANDLE_VALUE;
        if (found)
        {
            FindClose(handle);
        }

        m_File = CreateFile2(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, (found) ? OPEN_EXISTING : CREATE_NEW, nullptr);
        if (m_File == INVALID_HANDLE_VALUE)
        {
            std::cerr << (L"m_file is invalid. Error %ld.\n", GetLastError());
            return;
        }

        LARGE_INTEGER realFileSize = {};
        int flag = GetFileSizeEx(m_File, &realFileSize);
        if (!flag)
        {
            RXN_LOGGER::Error(L"\nError %ld occurred in GetFileSizeEx!", GetLastError());
            assert(false);
            return;
        }

        assert(realFileSize.HighPart == 0);

        m_currentFileSize = realFileSize.LowPart;

        if (m_currentFileSize == 0)
        {
            m_currentFileSize = DefaultFileSize;
        }
        else if (fileSize > m_currentFileSize)
        {
            m_currentFileSize = fileSize;
        }

        m_MapFile = CreateFileMapping(m_File, nullptr, PAGE_READWRITE, 0, m_currentFileSize, nullptr);

        if (m_MapFile == nullptr)
        {
            RXN_LOGGER::Error(L"m_mapFile is NULL: last error: %d\n", GetLastError());
            assert(false);
            return;
        }

        m_MapAddress = MapViewOfFile(m_MapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_currentFileSize);

        if (m_MapAddress == nullptr)
        {
            RXN_LOGGER::Error(L"m_mapAddress is NULL: last error: %d\n", GetLastError());
            assert(false);
            return;
        }
    }
    void MemoryMappedFile::Destroy(bool deleteFile)
    {
        if (m_MapAddress)
        {
            BOOL flag = UnmapViewOfFile(m_MapAddress);
            if (!flag)
            {
                RXN_LOGGER::Error(L"Error %ld occurred unmapping the view!", GetLastError());
                assert(false);
            }

            m_MapAddress = nullptr;

            flag = CloseHandle(m_MapFile);
            if (!flag)
            {
                RXN_LOGGER::Error(L"Error %ld occurred closing the mapping object!", GetLastError());
                assert(false);
            }

            flag = CloseHandle(m_File);
            if (!flag)
            {
                RXN_LOGGER::Error(L"Error %ld occurred closing the file!", GetLastError());
                assert(false);
            }
        }

        if (deleteFile)
        {
            DeleteFile(m_Filename.c_str());
        }
    }

    void MemoryMappedFile::GrowMapping(UINT size)
    {
        size += sizeof(UINT);
        if (size <= m_currentFileSize)
        {
            return;
        }

        int flag = FlushViewOfFile(m_MapAddress, 0);
        if (!flag)
        {
            RXN_LOGGER::Error(L"Error %ld occurred flushing the mapping object!", GetLastError());
            assert(false);
        }

        Destroy(false);
        m_currentFileSize = size;
        Init(m_Filename, m_currentFileSize);
    }

    UINT MemoryMappedFile::GetSize() const
    {
        if (m_MapAddress)
        {
            return static_cast<UINT *>(m_MapAddress)[0];
        }

        return 0;
    }

    void MemoryMappedFile::SetSize(UINT size)
    {
        if (m_MapAddress)
        {
            static_cast<UINT *>(m_MapAddress)[0] = size;
        }
    }

    void *MemoryMappedFile::GetData()
    {
        if (m_MapAddress)
        {
            return &static_cast<UINT *>(m_MapAddress)[1];
        }
        return nullptr;
    }
}
