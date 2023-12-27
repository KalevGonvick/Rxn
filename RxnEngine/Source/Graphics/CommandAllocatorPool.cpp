#include "Rxn.h"
#include "CommandAllocatorPool.h"
#include <format>
#include <filesystem>

namespace Rxn::Graphics::Pooled
{
    CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type)
        : m_CommandListType(type)
    {
    }

    CommandAllocatorPool::~CommandAllocatorPool()
    {
        std::ranges::for_each(m_AllocatorPool.begin(), m_AllocatorPool.end(), [](auto &alloc) 
            {
                alloc->Release();
            });

        m_AllocatorPool.clear();
    }

    ComPointer<ID3D12CommandAllocator> &CommandAllocatorPool::RequestAllocator(const uint64 fenceValue)
    {
        std::scoped_lock<std::mutex> LockGuard(m_AllocatorMutex);

        ComPointer<ID3D12CommandAllocator> allocator;

        if (!m_ReadyAllocators.empty())
        {
            auto& [rdyFence, rdyAlloc] = m_ReadyAllocators.front();

            if (rdyFence <= fenceValue)
            {
                allocator = rdyAlloc;
                ThrowIfFailed(allocator->Reset());
                m_ReadyAllocators.pop();
                return rdyAlloc;
            }
        }

        m_Device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&allocator));
        String allocatorName = std::format("CommandAllocator[{}]", m_AllocatorPool.size());
        allocator->SetName(Core::Strings::StringToWideString(allocatorName).c_str());
        return m_AllocatorPool.emplace_back(allocator);
       
    }

    void CommandAllocatorPool::Create(const ComPointer<ID3D12Device8> device)
    {
        m_Device = device;
    }

    void CommandAllocatorPool::DiscardAllocator(const uint64 FenceValue, ComPointer<ID3D12CommandAllocator> Allocator)
    {
        std::scoped_lock<std::mutex> LockGuard(m_AllocatorMutex);
        m_ReadyAllocators.push(std::make_pair(FenceValue, Allocator));
    }
}
