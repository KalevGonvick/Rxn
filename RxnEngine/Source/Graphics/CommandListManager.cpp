#include "Rxn.h"
#include "CommandListManager.h"
#include "Core/Math/Math.h"

namespace Rxn::Graphics::Manager
{
    CommandListManager::CommandListManager(ComPointer<ID3D12Device> device)
        : m_Device(device)
    {}

    CommandListManager::~CommandListManager() = default;

    void CommandListManager::CreateCommandList(const String &listName, ComPointer<ID3D12CommandAllocator> &cmdAlloc, bool autoClose)
    {
        ComPointer<ID3D12GraphicsCommandList> commandList;
        const uint8_t *p = reinterpret_cast<const uint8_t *>(listName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());

        if (m_CommandLists.contains(hash))
        {
            RXN_LOGGER::Error(L"Command list '%s' already exists", listName.c_str());
            return;
        }

        m_CommandLists.emplace(hash, commandList);
        m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&m_CommandLists.at(hash)));
        
        NAME_D3D12_OBJECT(m_CommandLists.at(hash));

        if (autoClose)
        {
            ThrowIfFailed(m_CommandLists.at(hash)->Close());
        }
    }

    ComPointer<ID3D12GraphicsCommandList> &CommandListManager::GetCommandList(const String &listName)
    {
        const uint8 * p = reinterpret_cast<const uint8 *>(listName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());
        return m_CommandLists.at(hash);
    }

    void CommandListManager::ExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue)
    {
        ID3D12CommandList *ppCommandLists[] = { GetCommandList(listName).Get() };
        cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }

    void CommandListManager::CloseCommandList(const String &listName)
    {
        ThrowIfFailed(GetCommandList(listName)->Close());
    }
    void CommandListManager::CloseAndExecuteCommandList(const String &listName, ComPointer<ID3D12CommandQueue> cmdQueue)
    {
        CloseCommandList(listName);
        ExecuteCommandList(listName, cmdQueue);
    }
}
