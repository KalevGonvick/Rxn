#pragma once
#include "PooledResource.h"

namespace Rxn::Graphics::Pooled
{
    class RXN_ENGINE_API CommandListPool : PooledCommandResource<ID3D12GraphicsCommandList6>
    {
    public:

        explicit CommandListPool(D3D12_COMMAND_LIST_TYPE type);
        ~CommandListPool();
        
    public:

        void Create(ID3D12Device8 *pDevice) override;
        
        ID3D12GraphicsCommandList6 *Request(const uint32 fenceValue) override;
        ID3D12GraphicsCommandList6 *RequestAndReset(const uint32 fenceValue, ID3D12CommandAllocator *pCmdAlloc = nullptr);
        
        void Discard(const uint32 fenceValue, ID3D12GraphicsCommandList6 *pResource) override;
        void ExecuteAndDiscard(const uint32 fenceValue, ID3D12GraphicsCommandList6 *pResource, ID3D12CommandQueue *pCmdQueue = nullptr);

    };
}
