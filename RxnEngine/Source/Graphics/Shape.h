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

        /**
         * Takes vector of vertex positions + indices and creates our corresponsing buffers.
         * 
         * \param vertices - vector of vertices
         * \param indices - vector of indices
         */
        void ReadDataFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices);

        /**
         * Takes vertex and index data from a file.
         * 
         * \param filename - name of the file
         * \return 
         */
        HRESULT LoadFromFile(const wchar_t *filename) override;

        /**
         * Uploads the GPU resources.
         * 
         * \param device
         * \param cmdQueue
         * \param cmdAlloc
         * \param cmdList
         * \return 
         */
        HRESULT UploadGpuResources(ComPointer<ID3D12Device> device, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12CommandAllocator> cmdAlloc, ComPointer<ID3D12GraphicsCommandList> cmdList) override;

        /**
         * .
         * 
         * \param frameCmdList
         * \param instanceCount
         */
        void DrawInstanced(ComPointer<ID3D12GraphicsCommandList> frameCmdList, uint32 instanceCount) override;

    private:

        std::vector<VertexPositionColour> m_Vertices;
        std::vector<uint32> m_Indices;

        uint32 m_VertexIndexBufferSize = 0;
        uint32 m_IndexDataSize = 0;
        uint32 m_VertexDataSize = 0;
        uint32 m_IndexStrideSize = 0;
        uint32 m_VertexStrideSize = 0;

        ComPointer<ID3D12Resource> m_VertexIndexBuffer = nullptr;
        ComPointer<ID3D12Resource> m_VertexIndexUpload = nullptr;

        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};
        D3D12_INDEX_BUFFER_VIEW m_IndexBufferView{};


    };
}
