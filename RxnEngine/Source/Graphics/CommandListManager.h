#pragma once

namespace Rxn::Graphics::Manager
{
    struct CommandListManagerException : std::runtime_error
    {
        explicit CommandListManagerException(const String &msg) : std::runtime_error(msg) {};
    };

    class RXN_ENGINE_API CommandListManager
    {
    public:

        CommandListManager(ComPointer<ID3D12Device> device);
        ~CommandListManager();

        void CreateCommandList(const String &listName, ComPointer<ID3D12CommandAllocator> &cmdAlloc, bool autoClose, D3D12_COMMAND_LIST_TYPE type);
        ComPointer<ID3D12GraphicsCommandList> &GetCommandList(const String &listName);

        void ExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue);
        void CloseCommandList(const String &listName);
        //void CloseAndExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue);

    private:

        uint32 GetHash(const String &str) const;
        bool CommandListExists(const String &listName) const;

    private:

        ComPointer<ID3D12Device> m_Device;
        std::unordered_map<uint32, ComPointer<ID3D12GraphicsCommandList>> m_CommandLists;
    };
}
