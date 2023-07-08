#pragma once
#include <string>

namespace Rxn::Common::Time
{
    RWString RXN_ENGINE_API GetTime(bool stripped = false);
    RWString RXN_ENGINE_API GetDate(bool stripped = false);
    RWString RXN_ENGINE_API GetDateTimeString(bool stripped = false);
    RWString StripString(RWString in, RWString marker);

}