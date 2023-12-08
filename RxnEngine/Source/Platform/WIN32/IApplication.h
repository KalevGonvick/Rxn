/*****************************************************************//**
 * \file   IApplication.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

#define ENTRYAPP(x) std::unique_ptr<Rxn::Platform::Win32::IApplication> EntryApplication() { return std::make_unique<x>(); }

namespace Rxn::Platform::Win32
{
    class RXN_ENGINE_API IApplication
    {

    public:

        IApplication();
        virtual ~IApplication();

    public:

        virtual void SetupEngineConfigurations() = 0;
        virtual void InitializeEngineSystems() = 0;
        virtual void InitializeRuntime() = 0;
        virtual void UpdateEngine() = 0;
        virtual void OnDestroy() = 0;

    };

} // Rxn::Platform::Win32
