#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    // Native, hardware-specific, PSO cache using a Pipeline Library.
    // Pipeline Libraries allow applications to explicitly group PSOs which are expected to share data.
    class MemoryMappedPipelineLibrary : public MemoryMappedFile
    {
    public:
        bool Init(ID3D12Device *pDevice, std::wstring filename);
        void Destroy(bool deleteFile);

        ID3D12PipelineLibrary *GetPipelineLibrary() { return m_pipelineLibrary.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineLibrary> m_pipelineLibrary;
    };
}

