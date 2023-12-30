#include "Rxn.h"
#include "DynamicConstantBuffer.h"

namespace Rxn::Graphics::Buffer
{
    DynamicConstantBuffer::DynamicConstantBuffer() = default;
    DynamicConstantBuffer::~DynamicConstantBuffer()
    {
        m_ConstantBuffer->Unmap(0, nullptr);
    }

    void DynamicConstantBuffer::Create(ID3D12Device *pDevice, uint32 constantSize, uint32 maxPipelineEffects, uint32 swapChainBuffers)
    {
        m_AlignedPerDrawConstantBufferSize = AlignConstantBuffer(constantSize);
        m_MaxOffsets = maxPipelineEffects;
        m_PerFrameConstantBufferSize = m_AlignedPerDrawConstantBufferSize * m_MaxOffsets;
        const uint32 bufferSize = m_PerFrameConstantBufferSize * swapChainBuffers;
        const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto buf = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        HRESULT result = pDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &buf,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_ConstantBuffer)
        );

        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for dynamic constant buffer...");
            throw DynamicConstantBufferException("Failed to create committed resource for dynamic constant buffer...");
        }

#ifdef _DEBUG
        NAME_D3D12_OBJECT(m_ConstantBuffer);
#endif

        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_ConstantBuffer->Map(0, &readRange, std::bit_cast<void **>(&m_MappedConstantBuffer)));
    }

    uint8 * DynamicConstantBuffer::GetMappedMemory(uint32 frameIndex, uint32 bufferOffset)
    {
        
        if (bufferOffset > m_MaxOffsets)
        {
            RXN_LOGGER::Error(L"Buffer offset '%d' exceeds max offset of '%d'.", bufferOffset, m_MaxOffsets);
            throw DynamicConstantBufferException(std::format("Buffer offset '{}' exceeds max offset of '{}'.", std::to_string(bufferOffset), std::to_string(m_MaxOffsets)));
        }

        uint32 constantBufferOffset = (frameIndex * m_PerFrameConstantBufferSize) + (bufferOffset * m_AlignedPerDrawConstantBufferSize);
        auto temp = std::bit_cast<uint8 *>(m_MappedConstantBuffer);
        temp += constantBufferOffset;

        return temp;
    }

    D3D12_GPU_VIRTUAL_ADDRESS DynamicConstantBuffer::GetGpuVirtualAddress(uint32 bufferOffset, uint32 frameIndex)
    {
        uint32 constantBufferOffset = (frameIndex * m_PerFrameConstantBufferSize) + (bufferOffset * m_AlignedPerDrawConstantBufferSize);
        return m_ConstantBuffer->GetGPUVirtualAddress() + constantBufferOffset;
    }
}
