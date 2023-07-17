#include "Rxn.h"
#include "Quad.h"

namespace Rxn::Graphics::Basic
{
    Quad::Quad()
    {
    }

    Quad::~Quad()
    {
    }

    void Quad::ReadDataFromRaw(std::vector<VertexPositionUV> &quads)
    {
        m_Quads = quads;
        CalculateBufferInfo(quads, m_StrideSize, m_DataSize);
    }


    HRESULT Quad::LoadFromFile(const wchar_t *filename)
    {
        return S_OK;
    }

    HRESULT Quad::UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList)
    {
        HRESULT result;

        result = Renderable::CreateCommittedBufferDestinationResource(device, m_DataBuffer, m_DataSize);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource destination buffer for quad.");
            return result;
        }

        result = Renderable::CreateCommittedUploadBufferResource(device, m_UploadBuffer, m_DataSize);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource upload buffer for quad.");
            return result;
        }

#ifdef _DEBUG
        NAME_D3D12_OBJECT(m_DataBuffer);
#endif

        uint8 *heap = Renderable::MapAndGetHeapLocationFromBuffer(m_UploadBuffer);
        Renderable::CopyDataToHeap(&heap, m_Quads);
        Renderable::AddCopyRegionTransition(cmdList, m_DataBuffer, m_DataSize, m_UploadBuffer);

        m_BufferView.BufferLocation = m_DataBuffer.Get()->GetGPUVirtualAddress();
        m_BufferView.SizeInBytes = m_DataSize;
        m_BufferView.StrideInBytes = m_StrideSize;

        return S_OK;
    }
}
