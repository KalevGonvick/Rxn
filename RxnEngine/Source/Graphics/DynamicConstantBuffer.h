/*****************************************************************//**
 * \file   DynamicConstantBuffer.h
 * \brief  
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once

namespace Rxn::Graphics::Buffer
{
    class RXN_ENGINE_API DynamicConstantBuffer
    {
    public:

        DynamicConstantBuffer(uint32 constantSize, uint32 maxDrawsPerFrame, uint32 frameCount);
        ~DynamicConstantBuffer();

    public:

        /**
         * .
         * 
         * \param pDevice
         */
        void Create(ID3D12Device *pDevice);

        /**
         * .
         * 
         * \param drawIndex
         * \param frameIndex
         * \return 
         */
        uint8 *GetMappedMemory(uint32 drawIndex, uint32 frameIndex);

        /**
         * .
         * 
         * \param drawIndex
         * \param frameIndex
         * \return 
         */
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
