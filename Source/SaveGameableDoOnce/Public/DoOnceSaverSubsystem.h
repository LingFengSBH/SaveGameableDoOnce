// Copyright Sun BoHeng(LingFeng) 2022

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DoOnceSaverSubsystem.generated.h"

//A TMap which stores each k2Node_SGDoOnce's state in Object   
USTRUCT(BlueprintType)
struct FDoOnceInfos
{
	GENERATED_BODY()

	//int32 as a key for each K2Node_SGDoOnce node,bool stores if this DoOnce has excuted
	UPROPERTY(BlueprintReadWrite)
	TMap<int32,bool> Info;
	
	FDoOnceInfos():Info(TMap<int32,bool>()){};
	FDoOnceInfos(TMap<int32,bool> InitInfo):Info(InitInfo){};
	FDoOnceInfos(int32 inKey,bool inHasDone){Info.Add(inKey,inHasDone);}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDoOnceDelegate,UObject*,Object,int32,Key);
UCLASS()
class SAVEGAMEABLEDOONCE_API UDoOnceSaverSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//There are many Objects in game,while each object has many K2Node_SGDoOnce nodes
	UPROPERTY()
	TMap<UObject*,FDoOnceInfos> HasDoneMap;

public:
	//Called when a K2Node_SGDoOnce node has executed successfully
	UPROPERTY(BlueprintAssignable)
	FDoOnceDelegate OnceHasDone;

	//Called when a K2Node_SGDoOnce node has been reset
	UPROPERTY(BlueprintAssignable)
	FDoOnceDelegate DoOnceHasReset;

	//Get HasDoneMap in subsystem
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "SaveGameableDoOnce|SaverSubsystem")
	TMap<UObject*,FDoOnceInfos> GetHasDoneMap(){return HasDoneMap;}
	
	//Override HasDoneMap value in subsystem
	UFUNCTION(BlueprintCallable,Category = "SaveGameableDoOnce|SaverSubsystem")
	void SetHasDoneMap(UPARAM(ref) TMap<UObject*,FDoOnceInfos>& NewHasDoneMap){HasDoneMap = NewHasDoneMap;}

	UFUNCTION(BlueprintCallable,Category = "SaveGameableDoOnce|SaverSubsystem")
	void SetHasDoneInfosOfObject(UObject*Object,UPARAM(ref) TMap<int32,bool>& Infos);

	//Clear invalid Keys
	UFUNCTION(BlueprintCallable,Category = "SaveGameableDoOnce|SaverSubsystem")
	void ClearInvalidObjectKeys();
	
	//Returns if this K2Node_SGDoOnce node can be executed
	UFUNCTION(BlueprintCallable,Category = "SaveGameableDoOnce|K2Node Implementation Functions")
	bool TryToDo(UObject*Object,int32 Key,bool StartClosed);
	
	//Reset a K2Node_SGDoOnce node
	void DoOnceReset(UObject*Object,int32 Key);

	//Get DoOnceInfos of a Object,return false if not find
	bool FindHasDoneInfos(UObject* Object,TMap<int32,bool>*& OutDoOnceInfos);
	
};
