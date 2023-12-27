#pragma once

namespace Rxn::Graphics
{
    const float32 INTERMEDIATE_CLEAR_COLOUR[4] = { 0.0f, 0.2f, 0.3f, 1.0f };

    struct DrawConstantBuffer
    {
        DirectX::XMMATRIX worldViewProjection;
    };

    struct Resolution
    {
        uint32 Width;
        uint32 Height;
    };

    // graphics object hash keys
    namespace GOHKeys
    {
        namespace CmdList
        {
            const static String INIT = "iCmdLst";
            const static String PRIMARY = "pCmdLst";
            const static String SECONDARY = "sCmdLst";
        };

        namespace CmdQueue
        {
            const static String INIT = "iCmdQueue";
            const static String PRIMARY = "pCmdQueue";
            const static String SECONDARY = "sCmdQueue";
        };
    };

    template<class T>
    void ResetComPointerArray(T *comPtrArray)
    {
        for (auto &i : *comPtrArray)
        {
            i.Release();
        }
    }

    template<class T>
    void ResetUniquePtrArray(T *uniquePtrArray)
    {
        for (auto &i : *uniquePtrArray)
        {
            i.reset();
        }
    }

    class HrException : public std::runtime_error
    {
    public:
        explicit HrException(HRESULT hr) : std::runtime_error(Core::Strings::GetHResultToString(hr)), m_hr(hr) {}
        HRESULT Error() const { return m_hr; }
    private:
        const HRESULT m_hr;
    };

    template<typename T>
    inline void SafeRelease(T *&rpInterface)
    {
        T *pInterface = rpInterface;

        if (pInterface)
        {
            pInterface->Release();
            rpInterface = nullptr;
        }
    }

    inline UINT Align(UINT location, UINT align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
    {
        return (location + (align - 1)) & ~(align - 1);
    }

    inline void ThrowIfFailed(HRESULT result)
    {
        if (FAILED(result))
        {
            throw HrException(result);
        }
    }

#ifdef _DEBUG

    inline UINT CalculateConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffer size is required to be aligned.
        return (byteSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
    };

    inline void SetName(ID3D12Object *pObject, LPCWSTR name)
    {
        pObject->SetName(name);
    };

    inline void SetNameIndexed(ID3D12Object *pObject, LPCWSTR name, UINT index)
    {
        WCHAR fullName[50];
        if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
        {
            pObject->SetName(fullName);
        }
    };


#else
    inline void SetName(ID3D12Object *, LPCWSTR) {};
    inline void SetNameIndexed(ID3D12Object *, LPCWSTR, UINT) {};
#endif

#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

}
