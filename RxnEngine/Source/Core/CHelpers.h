#pragma once

namespace Rxn::Core::C
{
    template<typename T, uint64 n>
    constexpr uint64 ArraySize(T(&array)[n])
    {
        return n;
    }
}
