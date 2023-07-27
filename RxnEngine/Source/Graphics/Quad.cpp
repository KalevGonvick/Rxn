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
        CalculateBufferInfo(quads, m_QuadStrideSize, m_QuadDataSize);
    }


    HRESULT Quad::LoadFromFile(const wchar_t *filename)
    {
        return S_OK;
    }

    HRESULT Quad::UploadGpuResources(ID3D12Device *device, ID3D12CommandQueue *cmdQueue, ID3D12CommandAllocator *cmdAlloc, ID3D12GraphicsCommandList *cmdList)
    {
        HRESULT result;

        result = Renderable::CreateCommittedBufferDestinationResource(device, m_QuadDataBuffer, m_QuadDataSize);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource destination buffer for quad.");
            return result;
        }

        result = Renderable::CreateCommittedUploadBufferResource(device, m_QuadUploadBuffer, m_QuadDataSize);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource upload buffer for quad.");
            return result;
        }

#ifdef _DEBUG
        NAME_D3D12_OBJECT(m_QuadDataBuffer);
#endif

        uint8 *heap = Renderable::MapAndGetHeapLocationFromBuffer(m_QuadUploadBuffer);
        Renderable::CopyDataToHeap(&heap, m_Quads);
        Renderable::AddCopyRegionTransition(cmdList, m_QuadDataBuffer, m_QuadDataSize, m_QuadUploadBuffer);

        m_QuadBufferView.BufferLocation = m_QuadDataBuffer.Get()->GetGPUVirtualAddress();
        m_QuadBufferView.SizeInBytes = m_QuadDataSize;
        m_QuadBufferView.StrideInBytes = m_QuadStrideSize;

        return S_OK;
    }
}
