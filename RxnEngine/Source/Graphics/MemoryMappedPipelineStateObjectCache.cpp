#include "Rxn.h"
#include "MemoryMappedPipelineStateObjectCache.h"

namespace Rxn::Graphics::Mapped
{
    void MemoryMappedPipelineStateObjectCache::Update(ID3DBlob *pBlob)
    {
        if (pBlob)
        {

            if (pBlob->GetBufferSize() > UINT_MAX)
            {
                RXN_LOGGER::Error(L"Current blob buffer size is larger than uint_max (0xffffffff).");
                throw FileException("Current blob buffer size is larger than uint_max (0xffffffff).");
            }

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
                if (neededSize > GetCurrentFileSize())
                {
                    RXN_LOGGER::Error(L"Needed size exceeds current file size.");
                    throw FileException("Needed size exceeds current file size.");
                }

                MemoryMappedFile::SetSize(blobSize);
                memcpy(GetData(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
            }
        }
    }
}
