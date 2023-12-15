#include "Rxn.h"
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    MemoryMappedFile::MemoryMappedFile() = default;
    MemoryMappedFile::~MemoryMappedFile() = default;

    bool MemoryMappedFile::IsMapped() const
    {
        return m_MapAddress != nullptr;
    }

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
            throw FileException("Create file error. Invalid handle was produced.");
        }

        LARGE_INTEGER realFileSize = {};
        if (!GetFileSizeEx(m_File, &realFileSize))
        {
            RXN_LOGGER::Error(L"\nError %ld occurred in GetFileSizeEx!", GetLastError());
            throw FileException("Error occurred in 'GetFileSizeEx'");
        }

        assert(realFileSize.HighPart == 0);

        m_CurrentFileSize = realFileSize.LowPart;

        if (m_CurrentFileSize == 0)
        {
            m_CurrentFileSize = DEFAULT_FILE_SIZE;
        }
        else if (fileSize > m_CurrentFileSize)
        {
            m_CurrentFileSize = fileSize;
        }

        m_MapFile = CreateFileMapping(m_File, nullptr, PAGE_READWRITE, 0, m_CurrentFileSize, nullptr);

        if (m_MapFile == nullptr)
        {
            RXN_LOGGER::Error(L"Map file handle is null. Error during 'CreateFileMapping': %d\n", GetLastError());
            throw FileMapException("Mapping a file using the given file address returned an error.");
        }

        m_MapAddress = MapViewOfFile(m_MapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_CurrentFileSize);

        if (m_MapAddress == nullptr)
        {
            RXN_LOGGER::Error(L"Creating a map view of a file using the given map address returned an error. Error Code: %ld", GetLastError());
            throw FileMapException("Creating a map view of a file using the given map address returned an error.");
        }
    }
    void MemoryMappedFile::DestroyFile(bool deleteFile)
    {
        if (m_MapAddress)
        {

            if (!UnmapViewOfFile(m_MapAddress))
            {
                RXN_LOGGER::Error(L"Error %ld occurred unmapping the view!", GetLastError());
                throw FileMapException("Error occurred when unmap was called using the given map address.");
            }

            m_MapAddress = nullptr;


            if (!CloseHandle(m_MapFile))
            {
                RXN_LOGGER::Error(L"Error %ld occurred closing the mapping object!", GetLastError());
                throw FileMapException("Error %ld occurred closing the mapping object");
            }

            if (!CloseHandle(m_File))
            {
                RXN_LOGGER::Error(L"Error %ld occurred closing the file!", GetLastError());
                throw FileException("Error %ld occurred closing the file!");
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
        if (size <= m_CurrentFileSize)
        {
            return;
        }

        if (!FlushViewOfFile(m_MapAddress, 0))
        {
            RXN_LOGGER::Error(L"Error %ld occurred flushing the mapping object!", GetLastError());
            throw FileException("Grow failed! Exception occurred during flush.");
        }

        DestroyFile(false);
        m_CurrentFileSize = size;
        InitFile(m_Filename, m_CurrentFileSize);
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
        return m_CurrentFileSize;
    }

    void MemoryMappedFile::SetSize(uint32 size)
    {
        if (m_MapAddress)
        {
            static_cast<uint32 *>(m_MapAddress)[0] = size;
        }
    }

    uint32 *MemoryMappedFile::GetData()
    {
        if (m_MapAddress)
        {
            return &static_cast<uint32 *>(m_MapAddress)[1];
        }
        return nullptr;
    }
}
