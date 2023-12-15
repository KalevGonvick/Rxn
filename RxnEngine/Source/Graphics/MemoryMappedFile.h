#pragma once
#include <iostream>

namespace Rxn::Graphics::Mapped
{
    struct FileException : std::runtime_error
    {
        explicit FileException(const String &msg) :runtime_error(msg) {};
    };

    struct FileMapException : std::runtime_error
    {
        explicit FileMapException(const String &msg) : runtime_error(msg) {};
    };

    class MemoryMappedFile
    {
    protected:

        MemoryMappedFile();
        ~MemoryMappedFile();

    public:

        /**
         * .
         * 
         * \return 
         */
        bool IsMapped() const;

        /**
         * .
         * 
         * \return 
         */
        uint32 *GetData();

        /**
         * .
         * 
         * \return 
         */
        uint32 GetCurrentFileSize() const;

        /**
         * .
         * 
         * \return 
         */
        uint32 GetSize() const;

        /**
         * .
         * 
         * \param deleteFile
         */
        void DestroyFile(bool deleteFile);

        /**
         * .
         * 
         * \param fileName
         * \param filesize
         */
        void InitFile(const WString &fileName, uint32 filesize = DEFAULT_FILE_SIZE);

    protected:

        /**
         * .
         * 
         * \param size
         */
        void GrowMapping(uint32 size);

        /**
         * .
         * 
         * \param size
         */
        void SetSize(uint32 size);
        
    private:

        static const uint32 DEFAULT_FILE_SIZE = 64;

        HANDLE m_MapFile = INVALID_HANDLE_VALUE;
        HANDLE m_File = INVALID_HANDLE_VALUE;

        LPVOID m_MapAddress = nullptr;

        WString m_Filename;
        uint32 m_CurrentFileSize = 0;
    };
}
