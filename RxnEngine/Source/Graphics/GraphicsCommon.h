#pragma once

namespace Rxn::Graphics
{

    struct VertexPositionUV
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT2 uv;
    };

    struct VertexPositionColour
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT3 colour;
    };

    enum SwapChainBuffers : uint32
    {
        BUFFER_ONE = 0,
        BUFFER_TWO,
        TOTAL_BUFFERS
    };

    const float INTERMEDIATE_CLEAR_COLOUR[4] = { 0.0f, 0.2f, 0.3f, 1.0f };

    enum RootParameters : uint32
    {
        RootParameterUberShaderCB = 0,
        RootParameterCB,
        RootParameterSRV,
        RootParametersCount
    };

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

    inline String HrToString(HRESULT hr)
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
        return std::string(s_str);
    }

    class HrException : public std::runtime_error
    {
    public:
        explicit HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
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

    inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR *path, UINT pathSize)
    {
        if (path == nullptr)
        {
            RXN_LOGGER::Error(L"Provided path is null");
            throw std::exception();
        }

        DWORD size = GetModuleFileName(nullptr, path, pathSize);
        if (size == 0 || size == pathSize)
        {
            RXN_LOGGER::Error(L"Provided path was truncated/method failed to find module.");
            throw std::exception();
        }

        WCHAR *lastSlash = wcsrchr(path, L'\\');
        if (lastSlash)
        {
            *(lastSlash + 1) = L'\0';
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
