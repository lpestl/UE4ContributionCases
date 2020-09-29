// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "TestsSplitFullObjectPathCommandlet.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogTestRef, Log, All);
/**
 * 
 */
UCLASS()
class UE4CONTRIBUTIONCASES_API UTestsSplitFullObjectPathCommandlet : public UCommandlet
{
	GENERATED_BODY()
	
public:
	UTestsSplitFullObjectPathCommandlet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		LogToConsole = true;
	}

	void PrintTestResult(const FString& InRef)
	{
		FString ClassName;
		FString PackagePath;
		FString ObjectName;
		FString SubObjectName;
		FPackageName::SplitFullObjectPath(InRef, ClassName, PackagePath, ObjectName, SubObjectName);
		UE_LOG(LogTestRef, Display, TEXT("Results:\nInString: '%s'\n\tOutClassName: '%s'\n\tOutPackageName: '%s'\n\tOutObjectName: '%s'\n\tOutSubObjectName: '%s'"), *InRef, *ClassName, *PackagePath, *ObjectName, *SubObjectName);
	}
	
	virtual int32 Main(const FString& Params) override
	{
		UE_LOG(LogTestRef, Display, TEXT("UTestsSplitFullObjectPathCommandlet::Main => '%s'"), *Params);

		// Links (References) known to me
		TArray<FString> Refs
		{
			"Class'/Script/UE4ContributionCases.SomePrimaryDataAsset'",
			"SomePrimaryDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomePrimaryDataAsset.DA_SomePrimaryDataAsset'",
			"SomeDataAsset'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset'",
			"SecondTypeForInstancing'/Game/ExamplesAssets/CustomDataAssets/DA_SomeDataAsset.DA_SomeDataAsset:SecondTypeForInstancing_0'"
		};

		for (int32 i = 0; i < Refs.Num(); ++i)
		{
			UE_LOG(LogTestRef, Display, TEXT("UTestsSplitFullObjectPathCommandlet::Main => Test # '%d'"), i);
			PrintTestResult(Refs[i]);
		}
		
		return  0;
	}	
};
