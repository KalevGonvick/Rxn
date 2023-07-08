#pragma once
#include <string>

namespace Rxn::Common::Time
{
    WString RXN_ENGINE_API GetTime(bool stripped = false);
    WString RXN_ENGINE_API GetDate(bool stripped = false);
    WString RXN_ENGINE_API GetDateTimeString(bool stripped = false);
    WString StripString(WString in, WString marker);

}