#include "Rxn.h"
#include "Shape.h"

namespace Rxn::Graphics::Basic
{
    Shape::Shape() = default;
    Shape::~Shape() = default;

    void Shape::ReadDataFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices)
    {
        m_Vertices = vertices;
        m_Indices = indices;
        CalculateBufferInfo(vertices, m_VertexStrideSize, m_VertexDataSize);
        CalculateBufferInfo(indices, m_IndexStrideSize, m_IndexDataSize);
        m_VertexIndexBufferSize = m_VertexDataSize + m_IndexDataSize;
    }

    HRESULT Shape::LoadFromFile(const wchar_t *filename)
    {
        return E_NOTIMPL;
    }

    HRESULT Shape::UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList)
    {
        Renderable::CreateCommittedBufferDestinationResource(device, m_VertexIndexBuffer, m_VertexIndexBufferSize);
        Renderable::CreateCommittedUploadBufferResource(device, m_VertexIndexUpload, m_VertexIndexBufferSize);

#ifdef _DEBUG
        NAME_D3D12_OBJECT(m_VertexIndexBuffer);
#endif

        uint8 *heap = Renderable::MapAndGetHeapLocationFromBuffer(m_VertexIndexUpload);
        Renderable::CopyDataToHeap(&heap, m_Vertices);
        Renderable::CopyDataToHeap(&heap, m_Indices);
        Renderable::AddCopyRegionTransition(cmdList, m_VertexIndexBuffer, m_VertexIndexBufferSize, m_VertexIndexUpload);

        m_VertexBufferView.BufferLocation = m_VertexIndexBuffer.Get()->GetGPUVirtualAddress();
        m_VertexBufferView.SizeInBytes = m_VertexDataSize;
        m_VertexBufferView.StrideInBytes = sizeof(VertexPositionColour);

        m_IndexBufferView.BufferLocation = m_VertexBufferView.BufferLocation + m_VertexDataSize;
        m_IndexBufferView.SizeInBytes = m_IndexDataSize;
        m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;

        return S_OK;
    }
}
