#include "Rxn.h"
#include "CommandAllocatorPool.h"
#include <format>
#include <filesystem>

namespace Rxn::Graphics::Pooled
{
    CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type)
        : PooledCommandResource<ID3D12CommandAllocator>(type)
    {
    }

    CommandAllocatorPool::~CommandAllocatorPool()
    {
        std::ranges::for_each(GetResourcePoolInternal().begin(), GetResourcePoolInternal().end(), [](auto &alloc)
            {
                alloc->Release();
            });

        GetResourcePoolInternal().clear();
    }

    ID3D12CommandAllocator *CommandAllocatorPool::Request(const uint32 fenceValue)
    {

        ComPointer<ID3D12CommandAllocator> allocator = RequestInternal(fenceValue);

        if (allocator)
        {
            ThrowIfFailed(allocator->Reset());
            return allocator;
        }
       

        GetDeviceInternal()->CreateCommandAllocator(GetCommandListTypeInternal(), IID_PPV_ARGS(&allocator));

#ifdef _DEBUG
        String allocatorName = std::format("CommandAllocator[{}]", GetResourcePoolInternal().size());
        allocator->SetName(Core::Strings::StringToWideString(allocatorName).c_str());
#endif
        return GetResourcePoolInternal().emplace_back(allocator);
       
    }

    void CommandAllocatorPool::Create(ID3D12Device8 *pDevice)
    {
        CreateInternal(pDevice);
    }

    void CommandAllocatorPool::Discard(const uint32 fenceValue, ID3D12CommandAllocator *pResource)
    {
        DiscardInternal(fenceValue, pResource);
    }
}
