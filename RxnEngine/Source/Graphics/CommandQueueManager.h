#pragma once

namespace Rxn::Graphics::Manager
{
    class CommandQueueManager
    {
    public:

        CommandQueueManager(ComPointer<ID3D12Device8> device);
        ~CommandQueueManager();

        void CreateCommandQueue(const String &queueName);
        ComPointer<ID3D12CommandQueue> &GetCommandQueue(const String &queueName);

    private:
        ComPointer<ID3D12Device8> m_Device;

        std::unordered_map<uint32, ComPointer<ID3D12CommandQueue>> m_CommandQueues;
    };
}
