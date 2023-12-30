#pragma once
#include "PooledResource.h"

namespace Rxn::Graphics::Pooled
{
    class RXN_ENGINE_API CommandAllocatorPool : PooledCommandResource<ID3D12CommandAllocator>
    {
    public:

        explicit CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);
        
        ~CommandAllocatorPool();

    public:

        void Create(ID3D12Device8 *pDevice) override;
        ID3D12CommandAllocator *Request(const uint32 fenceValue) override;
        void Discard(const uint32 fenceValue, ID3D12CommandAllocator *pResource) override;

    };
}
