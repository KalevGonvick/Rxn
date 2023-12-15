#pragma once
#include <type_traits>

template<typename CT, typename = std::enable_if_t<std::is_base_of_v<IUnknown, CT>>>
class RXN_ENGINE_API ComPointer
{

public:

    ComPointer() = default;

    explicit(false) ComPointer(CT *pointer)
    {
        SetPointerAndAddRef(pointer);
    }

    explicit(false) ComPointer(const ComPointer<CT> &other)
    {
        SetPointerAndAddRef(other.m_pointer);
    }

    explicit(false) ComPointer(ComPointer<CT> &&other) noexcept
        : m_pointer(other.m_pointer)
    {
        other.m_pointer = nullptr;
    }

public:

    ComPointer<CT> &operator=(const ComPointer<CT> &other)
    {
        ClearPointer();
        SetPointerAndAddRef(other.m_pointer);
        return *this;
    }

    ComPointer<CT> &operator=(ComPointer<CT> &&other)
    {
        ClearPointer();

        m_pointer = other.m_pointer;
        other.m_pointer = nullptr;
        return *this;
    }

    ComPointer<CT> &operator=(CT *other)
    {
        ClearPointer();
        SetPointerAndAddRef(other);
        return *this;
    }

    uint64 Release()
    {
        return ClearPointer();
    }

    CT *GetRef()
    {
        if (m_pointer)
        {
            m_pointer->AddRef();
            return m_pointer;
        }
        return nullptr;
    }

    CT *Get()
    {
        return m_pointer;
    }

    CT *Detatch() noexcept
    {
        CT *ptr = m_pointer;
        m_pointer = nullptr;
        return ptr;
    }

    template<typename T>
    bool QueryInterface(ComPointer<T> &other, HRESULT *errorCode = nullptr)
    {
        if (m_pointer)
        {
            HRESULT result = m_pointer->QueryInterface(IID_PPV_ARGS(&other));
            if (errorCode) *errorCode = result;
            return result == S_OK;
        }
        return false;
    }

    bool operator==(const ComPointer<CT> &other)
    {
        return m_pointer == other.m_pointer;
    }

    bool operator==(const CT *other)
    {
        return m_pointer == other;
    }

    CT *operator->()
    {
        return m_pointer;
    }

    CT **operator&()
    {
        return &m_pointer;
    }

    explicit(false) operator bool()
    {
        return m_pointer != nullptr;
    }
    explicit(false) operator CT *()
    {
        return m_pointer;
    }

private:

    uint64 ClearPointer()
    {
        uint64 newRef = 0;
        if (m_pointer)
        {
            newRef = m_pointer->Release();
            m_pointer = nullptr;
        }

        return newRef;
    }

    void SetPointerAndAddRef(CT *pointer)
    {
        m_pointer = pointer;
        if (m_pointer)
        {
            m_pointer->AddRef();
        }
    }

private:
    CT *m_pointer = nullptr;
};