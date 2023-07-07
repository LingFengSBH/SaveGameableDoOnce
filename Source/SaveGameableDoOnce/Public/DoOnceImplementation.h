// Copyright Sun BoHeng(LingFeng) 2023

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/World.h"
#include "DoOnceImplementation.generated.h"

//forward declaration UWorld to avid some strange issue when packaging
class UWorld;

UCLASS()
class SAVEGAMEABLEDOONCE_API UDoOnceImplementation : public UObject
{
	GENERATED_BODY()

	//K2Node_SGDoOnce will call this functions,Don't call manually
	//Returns if a DoOnce in the Object has executed
	UFUNCTION(BlueprintCallable,Category = "SaveGameableDoOnce|K2Node Implementation Functions")
	static bool SGDoOnceImplementation(FString NodeName,bool StartClosed,UObject* Object);

	//K2Node_SGDoOnce will call this functions,Don't call manually
	//Reset a DoOnce
	UFUNCTION(BlueprintCallable,Category = "SaveGameableDoOnce|K2Node Implementation Functions")
	static void SGResetImplementation(FString NodeName,UObject* Object);

	//Just return the input string
	//We need something for our K2Node_SGDoOnce to store it's node name
	//This function is a pure function,and we get the K2Node_SGDoOnce's name as default value for input
	//So that this function will return K2Node_SGDoOnce's name after compiling the Blueprint
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "SaveGameableDoOnce|K2Node Implementation Functions")
	static FString SGMakeStringImplementation(FString String){return String;}

	//Transform "K2Node_SGDoOnce_1" to "1"
	static int32 MakeKey(FString NodeName);
};
