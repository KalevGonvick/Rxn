#include "Rxn.h"
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    MemoryMappedFile::MemoryMappedFile() = default;

    bool MemoryMappedFile::IsMapped() const
    {
        return m_MapAddress != nullptr;
    }

    MemoryMappedFile::~MemoryMappedFile() = default;

    void MemoryMappedFile::InitFile(const WString &fileName, uint32 fileSize)
    {
        m_Filename = fileName;

        WIN32_FIND_DATA findFileData;
        HANDLE handle = FindFirstFileEx(fileName.c_str(), FindExInfoBasic, &findFileData, FindExSearchNameMatch, nullptr, 0);

        bool found = handle != INVALID_HANDLE_VALUE;
        if (found)
        {
            FindClose(handle);
        }

        m_File = CreateFile2(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, found ? OPEN_EXISTING : CREATE_NEW, nullptr);
        if (m_File == INVALID_HANDLE_VALUE)
        {
            RXN_LOGGER::Error(L"Create file error: %d", GetLastError());
            return;
        }

        LARGE_INTEGER realFileSize = {};
        if (!GetFileSizeEx(m_File, &realFileSize))
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
    void MemoryMappedFile::DestroyFile(bool deleteFile)
    {
        if (m_MapAddress)
        {

            if (!UnmapViewOfFile(m_MapAddress))
            {
                RXN_LOGGER::Error(L"Error %ld occurred unmapping the view!", GetLastError());
                assert(false);
            }

            m_MapAddress = nullptr;


            if (!CloseHandle(m_MapFile))
            {
                RXN_LOGGER::Error(L"Error %ld occurred closing the mapping object!", GetLastError());
                assert(false);
            }

            if (!CloseHandle(m_File))
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

    void MemoryMappedFile::GrowMapping(uint32 size)
    {
        size += sizeof(uint32);
        if (size <= m_currentFileSize)
        {
            return;
        }

        if (!FlushViewOfFile(m_MapAddress, 0))
        {
            RXN_LOGGER::Error(L"Error %ld occurred flushing the mapping object!", GetLastError());
            assert(false);
        }

        DestroyFile(false);
        m_currentFileSize = size;
        InitFile(m_Filename, m_currentFileSize);
    }

    uint32 MemoryMappedFile::GetSize() const
    {
        if (m_MapAddress)
        {
            return static_cast<uint32 *>(m_MapAddress)[0];
        }

        return 0;
    }

    uint32 MemoryMappedFile::GetCurrentFileSize() const
    {
        return m_currentFileSize;
    }

    void MemoryMappedFile::SetSize(uint32 size)
    {
        if (m_MapAddress)
        {
            static_cast<uint32 *>(m_MapAddress)[0] = size;
        }
    }

    void *MemoryMappedFile::GetData()
    {
        if (m_MapAddress)
        {
            return &static_cast<uint32 *>(m_MapAddress)[1];
        }
        return nullptr;
    }
}
