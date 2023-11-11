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

        void ReadDataFromRaw(std::vector<VertexPositionColour> &vertices, std::vector<UINT> &indices);

        virtual HRESULT LoadFromFile(const wchar_t *filename) override;
        virtual HRESULT UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList) override;

        std::vector<VertexPositionColour> m_Vertices;
        std::vector<UINT> m_Indices;

        UINT m_VertexIndexBufferSize;
        UINT m_IndexDataSize;
        UINT m_VertexDataSize;
        UINT m_IndexStrideSize;
        UINT m_VertexStrideSize;

        ComPointer<ID3D12Resource> m_VertexIndexBuffer;
        ComPointer<ID3D12Resource> m_VertexIndexUpload;

        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
        D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;


    };
}
