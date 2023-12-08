#pragma once

namespace Rxn::Graphics::Buffer
{
    class RXN_ENGINE_API DynamicConstantBuffer
    {
    public:

        DynamicConstantBuffer(uint32 constantSize, uint32 maxDrawsPerFrame, uint32 frameCount);
        ~DynamicConstantBuffer();

    public:

        void Create(ID3D12Device *pDevice);
        void *GetMappedMemory(uint32 drawIndex, uint32 frameIndex);
        D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(uint32 drawIndex, uint32 frameIndex);

    private:

        ComPointer<ID3D12Resource> m_ConstantBuffer = nullptr;
        void *m_MappedConstantBuffer = nullptr;
        uint32  m_AlignedPerDrawConstantBufferSize;
        uint32  m_PerFrameConstantBufferSize;

        uint32 m_FrameCount;
        uint32 m_MaxDrawsPerFrame = 256;
    };
}
