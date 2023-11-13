#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineStateObjectCache : public MemoryMappedFile
    {
    public:

        void InitObjectCache(const WString &filename);
        void DestroyCache(bool deleteFile);
        void Update(ID3DBlob *pBlob);

        size_t GetCachedBlobSize() const;

        void *GetCachedBlob();
    };
}
