#pragma once

namespace Rxn::Graphics::Buffer
{
    class RXN_ENGINE_API DynamicConstantBuffer
    {
    public:

        DynamicConstantBuffer(UINT constantSize, UINT maxDrawsPerFrame, UINT frameCount);
        ~DynamicConstantBuffer();

    public:

        void Create(ID3D12Device *pDevice);
        void *GetMappedMemory(UINT drawIndex, UINT frameIndex);
        D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(UINT drawIndex, UINT frameIndex);

    private:

        ComPointer<ID3D12Resource> m_ConstantBuffer = nullptr;
        void *m_MappedConstantBuffer = nullptr;
        UINT  m_AlignedPerDrawConstantBufferSize;
        UINT  m_PerFrameConstantBufferSize;

        UINT m_FrameCount;
        UINT m_MaxDrawsPerFrame = 256;
    };
}
