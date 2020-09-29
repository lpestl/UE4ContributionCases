// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CustomImportCallbackCommandlet.generated.h"


DEFINE_LOG_CATEGORY_STATIC(LogDemoJsonCallback, Log, All);
/**
 * 
 */
UCLASS()
class UE4CONTRIBUTIONCASES_API UCustomImportCallbackCommandlet : public UCommandlet
{
public:
	UCustomImportCallbackCommandlet(const FObjectInitializer& ObjectInitializer);
	
	virtual int32 Main(const FString& Params) override;

	void ExportCase(const FString& InReferenceString, const FString& InFilePath);
	
	GENERATED_BODY()	
};
