// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
// Note: 3. Reason for the error. If you uncomment this include,
// thereby reproducing the situation that the programmer simply forgot to remove the extra include,
// for example, after moving the logic to a dependent module (or the include just accidentally got there),
// then UBT will give us a completely UNINFORMATIVE error
// "Error C1083 : Cannot open include file: 'ErrorReason.generated.h': No such file or directory"
// Accordingly, Ryder's inspector code in no way tells us that the error is in the wrong include (there is no underlining or other hint)
// 
//#include "DependentModule/Example/ErrorReason.h"
//
// UPD: You can guess the cause of the error only if you look into the log of the compilation process
// and see what precedes the compilation error "MainModuleWithError.cpp"
// Example:
// --------------------Project: Default-------------------------------------------
// MainModuleWithError.cpp (0:00.39 at +0:00)
// 0>C:\Dev\External\UE4ContributionCases\Plugins\UBTinformerErrorExapmle\Source\DependentModule\Example\ErrorReason.h(7): Error C1083 : Cannot open include file: 'ErrorReason.generated.h': No such file or directory
// 
// Error executing C:\EpicGames\UE_4.27\Engine\Build\Windows\cl-filter\cl-filter.exe (tool returned code: 2)

#include "Modules/ModuleManager.h"

class FMainModuleWithErrorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
