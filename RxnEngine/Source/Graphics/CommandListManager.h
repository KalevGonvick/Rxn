#pragma once

namespace Rxn::Graphics::Manager
{
    class RXN_ENGINE_API CommandListManager
    {
    public:

        CommandListManager(ComPointer<ID3D12Device> device);
        ~CommandListManager();

        void CreateCommandList(String listName, ComPointer<ID3D12CommandAllocator> &cmdAlloc);
        ComPointer<ID3D12GraphicsCommandList> &GetCommandList(String listName);

    private:
        ComPointer<ID3D12Device> m_Device;
        std::unordered_map<uint32, ComPointer<ID3D12GraphicsCommandList>> m_CommandLists;
    };
}
