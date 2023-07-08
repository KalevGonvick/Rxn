#include "Rxn.h"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Rxn::Common::Time
{

    RWString GetTime(bool stripped)
    {
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);
        std::wstringstream wss;
        wss << std::put_time(&ltm, L"%T");

        RWString timeString = wss.str();

        if (stripped)
        {
            timeString = Common::Time::StripString(timeString, L":");
        }

        return timeString;
    }

    RWString GetDate(bool stripped)
    {
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);

        std::wstringstream wss;

        wss << std::put_time(&ltm, L"%F");

        std::wstring timeString = wss.str();

        if (stripped)
        {
            timeString = Common::Time::StripString(timeString, L"/");
        }

        return timeString;
    }

    RWString GetDateTimeString(bool stripped)
    {
        RWString timeString = GetDate(stripped) + L"T" + GetTime(stripped);

        if (stripped)
        {
            timeString = Time::StripString(timeString, L"T");
        }

        return timeString;
    }

    RWString StripString(RWString in, RWString marker)
    {
        for (auto c : marker) {
            in.erase(std::remove(in.begin(), in.end(), c), in.end());
        }

        return in;
    }
} // Rxn::Common::Time


