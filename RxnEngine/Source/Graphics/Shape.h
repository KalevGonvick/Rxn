#pragma once
#include "Renderable.h"

namespace Rxn::Graphics::Basic
{
    class RXN_ENGINE_API Shape : public Renderable
    {
    public:

        Shape();
        ~Shape();

    public:

        void ReadDataFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices);

        virtual HRESULT LoadFromFile(const wchar_t *filename) override;
        virtual HRESULT UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList) override;

        std::vector<VertexPositionColour> m_Vertices;
        std::vector<uint32> m_Indices;

        uint32 m_VertexIndexBufferSize = 0;
        uint32 m_IndexDataSize = 0;
        uint32 m_VertexDataSize = 0;
        uint32 m_IndexStrideSize = 0;
        uint32 m_VertexStrideSize = 0;

        ComPointer<ID3D12Resource> m_VertexIndexBuffer = nullptr;
        ComPointer<ID3D12Resource> m_VertexIndexUpload = nullptr;

        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
        D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;


    };
}
