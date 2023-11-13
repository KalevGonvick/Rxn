#include "Rxn.h"
#include "MemoryMappedPipelineStateObjectCache.h"

namespace Rxn::Graphics::Mapped
{
    void MemoryMappedPipelineStateObjectCache::InitObjectCache(const WString &filename)
    {
        InitFile(filename);
    }

    void MemoryMappedPipelineStateObjectCache::DestroyCache(bool deleteFile)
    {
        DestroyFile(deleteFile);
    }

    void MemoryMappedPipelineStateObjectCache::Update(ID3DBlob *pBlob)
    {
        if (pBlob)
        {
            assert(pBlob->GetBufferSize() <= UINT_MAX);    // Code below casts to UINT.
            const auto blobSize = static_cast<uint32>(pBlob->GetBufferSize());
            if (blobSize > 0)
            {
                // Grow the file if needed.
                const size_t neededSize = sizeof(uint32) + blobSize;
                if (neededSize > GetCurrentFileSize())
                {
                    MemoryMappedFile::GrowMapping(blobSize);
                }

                // Save the size of the blob, and then the blob itself.
                assert(neededSize <= GetCurrentFileSize());
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
