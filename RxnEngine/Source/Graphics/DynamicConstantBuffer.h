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
    inline uint32 AlignConstantBuffer(uint32 location, uint32 align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
    {
        return (location + (align - 1)) & ~(align - 1);
    }

    struct DynamicConstantBufferException : std::runtime_error
    {
        explicit DynamicConstantBufferException(const String &msg) : std::runtime_error(msg) {};
    };

    class RXN_ENGINE_API DynamicConstantBuffer
    {
    public:

        DynamicConstantBuffer();
        ~DynamicConstantBuffer();

    public:


        void Create(ID3D12Device *pDevice, uint32 constantSize, uint32 maxPipelineEffects, uint32 swapChainBuffers);

        /**
         * .
         * 
         * \param drawIndex
         * \param frameIndex
         * \return 
         */
        uint8 *GetMappedMemory(uint32 frameIndex, uint32 bufferOffset);

        /**
         * .
         * 
         * \param drawIndex
         * \param frameIndex
         * \return 
         */
        D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(uint32 bufferOffset, uint32 frameIndex);

    private:

        ComPointer<ID3D12Resource> m_ConstantBuffer = nullptr;
        void *m_MappedConstantBuffer = nullptr;
        
        uint32 m_AlignedPerDrawConstantBufferSize = 0;
        uint32 m_PerFrameConstantBufferSize = 0;
        uint32 m_MaxOffsets = 0;
    };
}
