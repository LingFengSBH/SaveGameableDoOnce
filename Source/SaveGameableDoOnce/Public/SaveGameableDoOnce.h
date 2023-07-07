// Copyright Sun BoHeng(LingFeng) 2023

#pragma once

#include "Modules/ModuleManager.h"

class FSaveGameableDoOnceModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
