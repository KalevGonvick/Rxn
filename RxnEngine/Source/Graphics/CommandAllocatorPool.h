#pragma once

namespace Rxn::Graphics::Pooled
{
    class RXN_ENGINE_API CommandAllocatorPool
    {
    public:

        CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);
        ~CommandAllocatorPool();

    public:

        void Create(ComPointer<ID3D12Device> device);
        ComPointer<ID3D12CommandAllocator> &RequestAllocator(const uint64 fenceValue);
        void DiscardAllocator(const uint64 FenceValue, ComPointer<ID3D12CommandAllocator> Allocator);
    private:
        const D3D12_COMMAND_LIST_TYPE m_CommandListType;
        ComPointer<ID3D12Device> m_Device;
        std::vector<ComPointer<ID3D12CommandAllocator>> m_AllocatorPool;
        std::queue<std::pair<uint64_t, ComPointer<ID3D12CommandAllocator>>> m_ReadyAllocators;

        std::mutex m_AllocatorMutex;

    };
}
