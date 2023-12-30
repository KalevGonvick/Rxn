#pragma once
#include <string>
#include <format>
#include <filesystem>

namespace Rxn::Core::Strings
{
    struct StringUtilException : std::runtime_error
    {
        explicit StringUtilException(const String &msg) : std::runtime_error(msg) {};
    };

    inline static WString StringToWideString(const String &str)
    {
        return std::filesystem::path(str).wstring();
    }

    inline static String WideStringToString(const WString &wstr)
    {
        return std::filesystem::path(wstr).string();
    }

    inline static String GetHResultToString(HRESULT hr)
    {
        return std::format("HRESULT 0x{}", static_cast<uint32>(hr));
    }

    inline void GetAssetsPath(_Out_writes_(pathSize) wchar_t *path, uint32 pathSize)
    {
        if (path == nullptr)
        {
            throw StringUtilException("The provided path is null or empty...");
        }

        uword size = GetModuleFileName(nullptr, path, pathSize);
        if (size == 0 || size == pathSize)
        {
            throw StringUtilException("The provided path was truncated/method failed to find module.");
        }

        wchar_t *lastSlash = wcsrchr(path, L'\\');
        if (lastSlash)
        {
            *(lastSlash + 1) = L'\0';
        }
    }
}
