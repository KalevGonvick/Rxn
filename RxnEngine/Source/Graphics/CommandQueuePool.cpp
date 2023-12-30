#include "Rxn.h"
#include "CommandQueuePool.h"

namespace Rxn::Graphics::Pooled
{
    CommandQueuePool::CommandQueuePool(D3D12_COMMAND_LIST_TYPE type)
        : PooledCommandResource<ID3D12CommandQueue>(type)
    {}

    CommandQueuePool::~CommandQueuePool()
    {
        std::ranges::for_each(GetResourcePoolInternal().begin(), GetResourcePoolInternal().end(), [](auto &alloc)
            {
                alloc->Release();
            });

        GetResourcePoolInternal().clear();
    }

    void CommandQueuePool::Create(ID3D12Device8 *pDevice)
    {
        CreateInternal(pDevice);
    }

    ID3D12CommandQueue *CommandQueuePool::Request(const uint32 fenceValue)
    {
        ComPointer<ID3D12CommandQueue> commandQueue = RequestInternal(fenceValue);

        if (commandQueue)
        {
            return commandQueue;
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = GetCommandListTypeInternal();
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;

        GetDeviceInternal()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
        return PushInternal(commandQueue);
    }

    void CommandQueuePool::Discard(const uint32 fenceValue, ID3D12CommandQueue *pResource)
    {
        DiscardInternal(fenceValue, pResource);
    }
}
