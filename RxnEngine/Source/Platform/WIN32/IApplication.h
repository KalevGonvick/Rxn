#pragma once

#define ENTRYAPP(x) std::unique_ptr<Rxn::Platform::Win32::IApplication> EntryApplication() { return std::make_unique<x>(); }

namespace Rxn::Platform::Win32
{
	class RXN_ENGINE_API IApplication
	{

	public:
		IApplication();

		virtual ~IApplication() {};

	public:
		virtual void ConfigureEngine() = 0;

		virtual void PreInitialize() = 0;

		virtual void Initialize() = 0;

		virtual void Update() = 0;
	};

} // Rxn::Platform::Win32
