#include "Rxn.h"
#include "CommandListPool.h"

namespace Rxn::Graphics::Pooled
{
    CommandListPool::CommandListPool(D3D12_COMMAND_LIST_TYPE type)
        : PooledCommandResource<ID3D12GraphicsCommandList6>(type)
    {}

    CommandListPool::~CommandListPool()
    {
        std::ranges::for_each(GetResourcePoolInternal().begin(), GetResourcePoolInternal().end(), [](auto &alloc)
            {
                alloc->Release();
            });

        GetResourcePoolInternal().clear();
    }


    void CommandListPool::Create(ID3D12Device8 *pDevice)
    {
        CreateInternal(pDevice);
    }

    ID3D12GraphicsCommandList6 *CommandListPool::Request(const uint32 fenceValue)
    {
        throw PooledResourceException("Command list requires reset first.");
        return RequestInternal(fenceValue);
    }

    ID3D12GraphicsCommandList6 *CommandListPool::RequestAndReset(const uint32 fenceValue, ID3D12CommandAllocator *pCmdAlloc)
    {
        ComPointer<ID3D12GraphicsCommandList6> commandList = RequestInternal(fenceValue);

        if (commandList)
        {
            commandList->Reset(pCmdAlloc, nullptr);
            return commandList;
        }


        GetDeviceInternal()->CreateCommandList(0, GetCommandListTypeInternal(), pCmdAlloc, nullptr, IID_PPV_ARGS(&commandList));

#ifdef _DEBUG
        String commandListName = std::format("CommandList[{}]", GetResourcePoolInternal().size());
        commandList->SetName(Core::Strings::StringToWideString(commandListName).c_str());
#endif

        return GetResourcePoolInternal().emplace_back(commandList);
    }

    void CommandListPool::Discard(const uint32 fenceValue, ID3D12GraphicsCommandList6 *pResource)
    {
        throw PooledResourceException("Command list requires execution");
        DiscardInternal(fenceValue, pResource);
    }

    void CommandListPool::ExecuteAndDiscard(const uint32 fenceValue, ID3D12GraphicsCommandList6 *pResource, ID3D12CommandQueue *pCmdQueue)
    {
        if (!pCmdQueue)
        {
            RXN_LOGGER::Error(L"Command queue must not be null when discarding command lists!");
            throw Pooled::PooledResourceException("Command queue must not be null when discarding command lists!");
        }

        pResource->Close();
        std::vector<ID3D12CommandList *> cmdLists;
        cmdLists.push_back(pResource);
        pCmdQueue->ExecuteCommandLists(static_cast<uint32>(cmdLists.size()), cmdLists.data());
        DiscardInternal(fenceValue, pResource);
    }

}
