#pragma once
#include "PooledResource.h"

namespace Rxn::Graphics::Pooled
{
    class RXN_ENGINE_API CommandQueuePool : PooledCommandResource<ID3D12CommandQueue>
    {
    public:

        explicit CommandQueuePool(D3D12_COMMAND_LIST_TYPE type);
        ~CommandQueuePool();

    public:

        void Create(ID3D12Device8 *pDevice) override;
        ID3D12CommandQueue *Request(const uint32 fenceValue) override;
        void Discard(const uint32 fenceValue, ID3D12CommandQueue *pResource) override;
    };
}
