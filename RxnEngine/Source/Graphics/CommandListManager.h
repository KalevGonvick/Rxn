#pragma once

namespace Rxn::Graphics::Manager
{
    class RXN_ENGINE_API CommandListManager
    {
    public:

        CommandListManager(ComPointer<ID3D12Device> device);
        ~CommandListManager();

        void CreateCommandList(const String &listName, ComPointer<ID3D12CommandAllocator> &cmdAlloc, bool autoClose = false);
        ComPointer<ID3D12GraphicsCommandList> &GetCommandList(const String &listName);

        void ExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue);
        void CloseCommandList(const String &listName);
        void CloseAndExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue);

    private:
        ComPointer<ID3D12Device> m_Device;
        std::unordered_map<uint32, ComPointer<ID3D12GraphicsCommandList>> m_CommandLists;
    };
}
