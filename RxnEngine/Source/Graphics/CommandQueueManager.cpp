#include "Rxn.h"
#include "CommandQueueManager.h"

namespace Rxn::Graphics::Manager
{
    CommandQueueManager::CommandQueueManager(ComPointer<ID3D12Device> device)
        : m_Device(device)
    {}

    CommandQueueManager::~CommandQueueManager() = default;

    void CommandQueueManager::CreateCommandQueue(const String &queueName)
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ComPointer<ID3D12CommandQueue> commandQueue;

        const uint8_t *p = reinterpret_cast<const uint8_t *>(queueName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());

        if (m_CommandQueues.find(hash) != m_CommandQueues.end())
        {
            RXN_LOGGER::Error(L"Command queue '%s' already exists", queueName);
            return;
        }

        m_CommandQueues.emplace(hash, commandQueue);
        m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueues.at(hash)));

    }

    ComPointer<ID3D12CommandQueue> &CommandQueueManager::GetCommandQueue(const String &queueName)
    {
        const uint8_t *p = reinterpret_cast<const uint8_t *>(queueName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());
        return m_CommandQueues.at(hash);
    }
}
