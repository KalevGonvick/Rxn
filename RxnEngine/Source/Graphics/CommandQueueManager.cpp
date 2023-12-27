#include "Rxn.h"
#include "CommandQueueManager.h"

namespace Rxn::Graphics::Manager
{
    CommandQueueManager::CommandQueueManager(ComPointer<ID3D12Device8> device)
        : m_Device(device)
    {}

    CommandQueueManager::~CommandQueueManager() = default;

    void CommandQueueManager::CreateCommandQueue(const String &queueName)
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
        ComPointer<ID3D12CommandQueue> commandQueue;

        auto p = reinterpret_cast<const uint8_t *>(queueName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());

        if (m_CommandQueues.contains(hash))
        {
            RXN_LOGGER::Error(L"Command queue '%s' already exists", queueName.c_str());
            return;
        }

        m_CommandQueues.try_emplace(hash, commandQueue);
        m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueues.at(hash)));

    }

    ComPointer<ID3D12CommandQueue> &CommandQueueManager::GetCommandQueue(const String &queueName)
    {
        auto p = reinterpret_cast<const uint8_t *>(queueName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());
        return m_CommandQueues.at(hash);
    }
}
