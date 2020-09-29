// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SomeClassForInstancedProperties.h"
#include "Engine/DataAsset.h"
#include "SomeDataAsset.generated.h"

/**
 * 
 */
USTRUCT()
struct FSomeStructWithInstancedProperty
{
	UPROPERTY(EditDefaultsOnly, Instanced, NoClear)
	USomeClassForInstancedProperties* ObjectForInstancing;
	
	GENERATED_BODY()
	
};


UCLASS()
class UE4CONTRIBUTIONCASES_API USomeDataAsset : public UDataAsset
{
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FSomeStructWithInstancedProperty> ArrayStructWithInstancedObject;
	
	GENERATED_BODY()	
};
