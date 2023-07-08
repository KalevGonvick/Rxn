#pragma once
#include <string>

namespace Rxn::Common::Time
{
    std::wstring RXN_ENGINE_API GetTime(bool stripped = false);
    std::wstring RXN_ENGINE_API GetDate(bool stripped = false);
    std::wstring RXN_ENGINE_API GetDateTimeString(bool stripped = false);
    std::wstring StripString(std::wstring in, std::wstring marker);

}