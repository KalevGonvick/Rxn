#include "Rxn.h"
#include "Quad.h"

namespace Rxn::Graphics::Basic
{
    Quad::Quad() = default;
    Quad::~Quad() = default;

    void Quad::ReadDataFromRaw(const std::vector<VertexPositionUV> &quads)
    {
        m_Quads = quads;
        CalculateBufferInfo(quads, m_QuadStrideSize, m_QuadDataSize);
    }


    HRESULT Quad::LoadFromFile(const wchar_t *filename)
    {
        return S_OK;
    }

    HRESULT Quad::UploadGpuResources(ComPointer<ID3D12Device8> device, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12CommandAllocator> cmdAlloc, ComPointer<ID3D12GraphicsCommandList6> cmdList)
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

    void Quad::DrawInstanced(ComPointer<ID3D12GraphicsCommandList6> frameCmdList, uint32 instanceCount)
    {
        frameCmdList->IASetVertexBuffers(0, 1, &m_QuadBufferView);
        frameCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        frameCmdList->DrawInstanced(static_cast<uint32>(m_Quads.size()), 1, 0, 0);
    }
}
