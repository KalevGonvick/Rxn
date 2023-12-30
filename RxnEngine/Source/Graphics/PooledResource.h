#pragma once
#include <type_traits>

namespace Rxn::Graphics::Pooled
{
    struct PooledResourceException : std::runtime_error
    {
        explicit PooledResourceException(const String &msg) : std::runtime_error(msg) {};
    };

    template<typename CT, typename = std::enable_if_t<std::is_base_of_v<IUnknown, CT>>>
    class RXN_ENGINE_API PooledCommandResource : NonCopyable
    {
    public:

        explicit PooledCommandResource(D3D12_COMMAND_LIST_TYPE type)
            : m_CommandListType(type)
        {};

        virtual ~PooledCommandResource() = default;

    public:

        virtual void Create(ID3D12Device8 *device) = 0;
        virtual CT *Request(const uint32 fenceValue) = 0;
        virtual void Discard(const uint32 fenceValue, CT *pResource) = 0;

    protected:

        void CreateInternal(ID3D12Device8 *device)
        {
            m_Device = device;
        }

        CT *RequestInternal(const uint32 fenceValue)
        {
            std::scoped_lock<std::mutex> lock(m_Mutex);
            ComPointer<CT> commandResource;
            if (!m_ReadyCommandResources.empty())
            {
                auto &[rdyFence, rdyResource] = m_ReadyCommandResources.front();

                if (rdyFence <= fenceValue)
                {
                    commandResource = rdyResource;
                    m_ReadyCommandResources.pop();
                    return commandResource;
                }
            }

            return nullptr;
        };

        void DiscardInternal(const uint32 fenceValue, CT *resource)
        {
            std::scoped_lock<std::mutex> lock(m_Mutex);
            m_ReadyCommandResources.push(std::make_pair(fenceValue, resource));
        };

        CT *PushInternal(CT *resource)
        {
            return m_CommandResourcePool.emplace_back(resource);
        }

        std::vector<ComPointer<CT>> &GetResourcePoolInternal()
        {
            return m_CommandResourcePool;
        };

        std::mutex &GetMutexInternal()
        {
            return m_Mutex;
        }

        D3D12_COMMAND_LIST_TYPE GetCommandListTypeInternal()
        {
            return m_CommandListType;
        };

        ID3D12Device8 *GetDeviceInternal()
        {
            return m_Device;
        };

    private:

        const D3D12_COMMAND_LIST_TYPE m_CommandListType;
        ID3D12Device8 *m_Device = nullptr;

        std::vector<ComPointer<CT>> m_CommandResourcePool;
        std::queue<std::pair<uint64, ComPointer<CT>>> m_ReadyCommandResources;
        std::mutex m_Mutex;
    };
}
