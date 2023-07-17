#include "Rxn.h"
#include "MemoryMappedPipelineStateObjectCache.h"

namespace Rxn::Graphics::Mapped
{
    void MemoryMappedPipelineStateObjectCache::Init(std::wstring filename)
    {
        MemoryMappedFile::Init(filename);
    }

    void MemoryMappedPipelineStateObjectCache::Destroy(bool deleteFile)
    {
        MemoryMappedFile::Destroy(deleteFile);
    }

    void MemoryMappedPipelineStateObjectCache::Update(ID3DBlob *pBlob)
    {
        if (pBlob)
        {
            assert(pBlob->GetBufferSize() <= UINT_MAX);    // Code below casts to UINT.
            const UINT blobSize = static_cast<UINT>(pBlob->GetBufferSize());
            if (blobSize > 0)
            {
                // Grow the file if needed.
                const size_t neededSize = sizeof(UINT) + blobSize;
                if (neededSize > m_currentFileSize)
                {
                    MemoryMappedFile::GrowMapping(blobSize);
                }

                // Save the size of the blob, and then the blob itself.
                assert(neededSize <= m_currentFileSize);
                MemoryMappedFile::SetSize(blobSize);
                memcpy(GetCachedBlob(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
            }
        }
    }

    size_t MemoryMappedPipelineStateObjectCache::GetCachedBlobSize() const
    {
        return GetSize();
    }

    void *MemoryMappedPipelineStateObjectCache::GetCachedBlob()
    {
        return GetData();
    }
}
