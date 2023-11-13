#include "Rxn.h"
#include "DynamicConstantBuffer.h"

namespace Rxn::Graphics::Buffer
{
    DynamicConstantBuffer::DynamicConstantBuffer(UINT constantSize, UINT maxDrawsPerFrame, UINT frameCount)
        : m_AlignedPerDrawConstantBufferSize(Align(constantSize))
        , m_FrameCount(frameCount)
        , m_MaxDrawsPerFrame(maxDrawsPerFrame)
    {
        m_PerFrameConstantBufferSize = m_AlignedPerDrawConstantBufferSize * m_MaxDrawsPerFrame;
    }

    DynamicConstantBuffer::~DynamicConstantBuffer()
    {
        m_ConstantBuffer->Unmap(0, nullptr);
    }

    void DynamicConstantBuffer::Init(ID3D12Device *pDevice)
    {
        const UINT bufferSize = m_PerFrameConstantBufferSize * m_FrameCount;
        const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto buf = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        ThrowIfFailed(pDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &buf,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_ConstantBuffer)
        ));

        NAME_D3D12_OBJECT(m_ConstantBuffer);

        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_ConstantBuffer->Map(0, &readRange, reinterpret_cast<void **>(&m_MappedConstantBuffer)));
    }

    void *DynamicConstantBuffer::GetMappedMemory(UINT drawIndex, UINT frameIndex)
    {
        assert(drawIndex < m_MaxDrawsPerFrame);
        UINT constantBufferOffset = (frameIndex * m_PerFrameConstantBufferSize) + (drawIndex * m_AlignedPerDrawConstantBufferSize);

        UINT8 *temp = reinterpret_cast<UINT8 *>(m_MappedConstantBuffer);
        temp += constantBufferOffset;

        return temp;
    }

    D3D12_GPU_VIRTUAL_ADDRESS DynamicConstantBuffer::GetGpuVirtualAddress(UINT drawIndex, UINT frameIndex)
    {
        UINT constantBufferOffset = (frameIndex * m_PerFrameConstantBufferSize) + (drawIndex * m_AlignedPerDrawConstantBufferSize);
        return m_ConstantBuffer->GetGPUVirtualAddress() + constantBufferOffset;
    }
}
