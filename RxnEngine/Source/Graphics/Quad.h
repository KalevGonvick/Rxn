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
        HRESULT UploadGpuResources(ID3D12Device8 *pDevice, ID3D12GraphicsCommandList6 *pCmdList) override;
        
        void DrawInstanced(ID3D12GraphicsCommandList6 *pCmdList, uint32 instanceCount) override;

        std::vector<VertexPositionUV> m_Quads;
        uint32 m_QuadStrideSize;
        uint32 m_QuadDataSize;

        ComPointer<ID3D12Resource> m_QuadDataBuffer;
        ComPointer<ID3D12Resource> m_QuadUploadBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_QuadBufferView;


    };
}
