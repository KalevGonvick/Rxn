#pragma once
#include "Renderable.h"

namespace Rxn::Graphics::Basic
{
    class RXN_ENGINE_API Quad : public Renderable
    {
    public:

        Quad();
        ~Quad();

    public:

        void ReadDataFromRaw(const std::vector<VertexPositionUV> &quads);

        HRESULT LoadFromFile(const wchar_t *filename) override;
        HRESULT UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList) override;
        void DrawInstanced(ComPointer<ID3D12GraphicsCommandList> frameCmdList, uint32 instanceCount) override;

        std::vector<VertexPositionUV> m_Quads;
        uint32 m_QuadStrideSize;
        uint32 m_QuadDataSize;

        ComPointer<ID3D12Resource> m_QuadDataBuffer;
        ComPointer<ID3D12Resource> m_QuadUploadBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_QuadBufferView;


    };
}
