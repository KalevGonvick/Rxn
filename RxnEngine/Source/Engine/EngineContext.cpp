#include "Rxn.h"
#include "EngineContext.h"

namespace Rxn::Engine
{
	static EngineContext& GetContext()
	{
		static EngineContext inst;
		return inst;
	}

	EngineContext::EngineContext()
		: m_EngineSettings()
	{
	}

	EngineContext::~EngineContext() = default;

	EngineSettings& EngineContext::GetEngineSettings()
	{
		return GetContext().m_EngineSettings;
	}


}