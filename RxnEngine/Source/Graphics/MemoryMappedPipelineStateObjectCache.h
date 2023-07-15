#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineStateObjectCache : public MemoryMappedFile
    {
    public:
        void Init(std::wstring filename) { MemoryMappedFile::Init(filename); }
        void Destroy(bool deleteFile) { MemoryMappedFile::Destroy(deleteFile); }
        void Update(ID3DBlob *pBlob);

        size_t GetCachedBlobSize() const { return GetSize(); }
        void *GetCachedBlob() { return GetData(); }
    };
}
