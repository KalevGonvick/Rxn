#pragma once

namespace Rxn::Graphics
{
    class CommandQueueManager
    {
    public:

        CommandQueueManager(uint32 num);
        ~CommandQueueManager();

        void CreateCommantQueues();
        ComPointer<ID3D12CommandQueue> &GetCommandQueue(int index = 0);

    private:

        uint32 m_NumCommandQueues;

        std::vector<ComPointer<ID3D12CommandQueue>> m_CommandQueues;
    };
}
