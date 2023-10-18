// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OSCAddress.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OSCRouteFunctions.generated.h"

USTRUCT(BlueprintType)
struct FOSCRouteConfig
{
	GENERATED_BODY()

	FGuid Id;

	UPROPERTY(EditAnywhere)
	FString Address;

	FOSCRouteConfig();

};

USTRUCT(BlueprintType)
struct FOSCRoute
{
	GENERATED_BODY()

	FGuid Id;
	int32 Index;
	FName PinName;
	FOSCAddress Address;
};

UCLASS()
class UOSCRouteFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	static FOSCMessage& Switch(UPARAM(ref) FOSCMessage& Message, UPARAM(ref) FOSCRoute& Route);
};
