// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SomeDataAsset.h"
#include "Engine/DataAsset.h"
#include "SomePrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class UE4CONTRIBUTIONCASES_API USomePrimaryDataAsset : public UPrimaryDataAsset
{
public:
	UPROPERTY(EditDefaultsOnly)
	USomeDataAsset* GameplayDataAsset {nullptr};
	
	GENERATED_BODY()	
};
