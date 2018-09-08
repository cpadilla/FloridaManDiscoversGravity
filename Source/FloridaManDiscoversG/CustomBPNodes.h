// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CustomBPNodes.generated.h"

/**
 * 
 */
UCLASS()
class FLORIDAMANDISCOVERSG_API UCustomBPNodes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		UFUNCTION(BlueprintCallable, meta=(DisplayName="ParseByteArrayToJSON", Keywords="Parse Byte Array To JSON"), Category="Util")
		static void ParseByteArrayToJSON(TArray<uint8> byteArray, FString& output);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetBytesFromJSONString", Keywords = "Get Bytes From JSON String"), Category = "Util")
		static void GetBytesFromJSONString(FString JsonString, TArray<uint8>& bytes);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "HandleMindwaveResponse", Keywords = "Handle Mindwave Response"), Category = "Util")
		static void HandleMindwaveResponse(FString message, TMap<FString, bool>& boolMaps, TMap<FString, FString>& stringMaps, TMap<FString, float>& numberMaps, FString& debugOut);
};
