#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineStateObjectCache : public MemoryMappedFile
    {
    public:

        void Init(std::wstring filename);
        void Destroy(bool deleteFile);
        void Update(ID3DBlob *pBlob);

        size_t GetCachedBlobSize() const;

        void *GetCachedBlob();
    };
}
