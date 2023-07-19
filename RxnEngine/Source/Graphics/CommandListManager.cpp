#include "Rxn.h"
#include "CommandListManager.h"
#include "Core/Math/Math.h"

namespace Rxn::Graphics::Manager
{
    CommandListManager::CommandListManager(ComPointer<ID3D12Device> device)
        : m_Device(device)
    {}

    CommandListManager::~CommandListManager() = default;

    void CommandListManager::CreateCommandList(const String listName, ComPointer<ID3D12CommandAllocator> &cmdAlloc)
    {
        ComPointer<ID3D12GraphicsCommandList> commandList;
        const uint8_t *p = reinterpret_cast<const uint8_t *>(listName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());

        if (m_CommandLists.find(hash) != m_CommandLists.end())
        {
            RXN_LOGGER::Error(L"Command list '%s' already exists", listName);
            return;
        }



        m_CommandLists.emplace(hash, commandList);
        m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&m_CommandLists.at(hash)));
    }

    ComPointer<ID3D12GraphicsCommandList> &CommandListManager::GetCommandList(String listName)
    {
        const uint8_t *p = reinterpret_cast<const uint8_t *>(listName.c_str());
        uint32 hash = Core::Math::Murmer3(p, sizeof(p), RenderContext::GetEngineSeed());
        return m_CommandLists.at(hash);
    }
}
