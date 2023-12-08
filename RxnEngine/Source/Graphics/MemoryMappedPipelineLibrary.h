#pragma once
#include "MemoryMappedFile.h"

namespace Rxn::Graphics::Mapped
{
    class MemoryMappedPipelineLibrary : public MemoryMappedFile
    {
    public :

        bool InitPipelineLibrary(ID3D12Device *pDevice, const WString &filename);
        void DestroyPipelineLibrary(bool deleteFile);
        ID3D12PipelineLibrary *GetPipelineLibrary();

    private:

        ComPointer<ID3D12PipelineLibrary> m_pipelineLibrary;
    };
}

