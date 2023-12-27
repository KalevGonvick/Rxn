#include "Rxn.h"
#include "CommandListManager.h"
#include "Core/Math/Math.h"
#include <format>

namespace Rxn::Graphics::Manager
{
    CommandListManager::CommandListManager(ComPointer<ID3D12Device8> device)
        : m_Device(device)
    {}

    CommandListManager::~CommandListManager() = default;

    void CommandListManager::CreateCommandList(const String &listName, ComPointer<ID3D12CommandAllocator> &cmdAlloc, bool autoClose, D3D12_COMMAND_LIST_TYPE type)
    {
        RXN_LOGGER::Debug(L"Creating a new ID3D12GraphicsCommandList with the properties { key: '%s', autoClose: '%d', type: '%d' }", listName.c_str(), autoClose, type);
        ComPointer<ID3D12GraphicsCommandList6> commandList;
        
        uint32 hash = GetHash(listName);

        if (m_CommandLists.contains(hash))
        {
            RXN_LOGGER::Error(L"Command list '%s' already exists", listName.c_str());
            throw CommandListManagerException("Command list '" + listName + "' already exists!");
        }

        m_CommandLists.emplace(hash, commandList);
        m_Device->CreateCommandList(0, type, cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&m_CommandLists.at(hash)));
        
        WString cmdListName = std::format(L"CMDLIST_{}", std::to_wstring(hash));
        m_CommandLists.at(hash)->SetName(cmdListName.c_str());

        if (autoClose)
        {
            ThrowIfFailed(m_CommandLists.at(hash)->Close());
        }
    }

    ComPointer<ID3D12GraphicsCommandList6> &CommandListManager::GetCommandList(const String &listName)
    {
        auto p = reinterpret_cast<const uint8 *>(listName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());
        return m_CommandLists.at(hash);
    }

    void CommandListManager::ExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue)
    {
        if (!CommandListExists(listName))
        {
            RXN_LOGGER::Error(L"Attempting to execute non-existant command list '%s'!", listName.c_str());
            throw CommandListManagerException("Attempting to execute non-existant command list '" + listName + "'!");
        }

        ID3D12CommandList *ppCommandLists[] = { GetCommandList(listName).Get() };
        cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }

    void CommandListManager::CloseCommandList(const String &listName)
    {

        if (!CommandListExists(listName))
        {
            RXN_LOGGER::Error(L"Attempting to close non-existant command list '%s'!", listName.c_str());
            throw CommandListManagerException("Attempting to close non-existant command list '" + listName + "'!");
        }

        ThrowIfFailed(GetCommandList(listName)->Close());
    }

    uint32 CommandListManager::GetHash(const String &str) const
    {
        auto p = reinterpret_cast<const uint8_t *>(str.c_str());
        return Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());
    }

    bool CommandListManager::CommandListExists(const String &listName) const
    {
        
        if (uint32 hash = GetHash(listName))
        {
            return m_CommandLists.contains(hash);
        }

        return false;


    }
}
