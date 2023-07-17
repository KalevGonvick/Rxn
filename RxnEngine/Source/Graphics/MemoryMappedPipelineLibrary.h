#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineLibrary : public MemoryMappedFile
    {
    public:

        bool Init(ID3D12Device *pDevice, std::wstring filename);
        void Destroy(bool deleteFile);
        ID3D12PipelineLibrary *GetPipelineLibrary();

    private:

        ComPointer<ID3D12PipelineLibrary> m_pipelineLibrary;
    };
}

