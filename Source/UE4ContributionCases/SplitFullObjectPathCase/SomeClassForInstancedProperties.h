// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SomeClassForInstancedProperties.generated.h"

/**
 * 
 */
UCLASS()
class UE4CONTRIBUTIONCASES_API USomeClassForInstancedProperties : public UObject
{
	GENERATED_BODY()
	
};

UCLASS(EditInlineNew, meta=(DisplayName="First type"))
class UFirstTypeForInstancing : public USomeClassForInstancedProperties
{
public:
	UPROPERTY(EditDefaultsOnly)
	FString SomeString;

	GENERATED_BODY()
};

UCLASS(EditInlineNew, meta=(DisplayName="Second type"))
class USecondTypeForInstancing : public USomeClassForInstancedProperties
{
public:
	UPROPERTY(EditDefaultsOnly)
	int32 SomeIntValue;

	GENERATED_BODY()
};