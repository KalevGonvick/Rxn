#pragma once

namespace Rxn::Common::Cmd
{
    void RXN_ENGINE_API ReadCommandLineArguments();
    void RXN_ENGINE_API ReadArgument(const wchar_t* argKey, const wchar_t* argValue = L"", boolean argumentIsFlag = true);
};