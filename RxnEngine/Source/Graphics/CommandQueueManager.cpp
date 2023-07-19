#include "Rxn.h"
#include "CommandQueueManager.h"

namespace Rxn::Graphics
{
    CommandQueueManager::CommandQueueManager(uint32 num)
        : m_NumCommandQueues(num)
    {
    }

    CommandQueueManager::~CommandQueueManager() = default;

    void CommandQueueManager::CreateCommantQueues()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        for (int n = 0; n < m_NumCommandQueues; n++)
        {
            ComPointer<ID3D12CommandQueue> queue;
            m_CommandQueues.push_back(queue);
        }

        for (int n = 0; n < m_NumCommandQueues; n++)
        {
            RenderContext::GetGraphicsDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueues[n]));
        }

    }

    ComPointer<ID3D12CommandQueue> &CommandQueueManager::GetCommandQueue(int index)
    {
        return m_CommandQueues[index];
    }
}
