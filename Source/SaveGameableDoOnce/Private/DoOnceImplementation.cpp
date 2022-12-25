// Copyright Sun BoHeng(LingFeng) 2022

#include "DoOnceImplementation.h"
#include "DoOnceSaverSubsystem.h"
#include "Kismet/KismetStringLibrary.h"

bool UDoOnceImplementation::SGDoOnceImplementation(FString NodeName,bool StartClosed,UObject* Object)
{
	int32 Key = MakeKey(NodeName);

    UWorld* World = Object->GetWorld();
	if (World)
	{
		return World->GetGameInstance()->GetSubsystem<UDoOnceSaverSubsystem>()->TryToDo(Object,Key,StartClosed);
	}
	return false;
}

void UDoOnceImplementation::SGResetImplementation(FString NodeName, UObject* Object)
{
	UWorld* World = Object->GetWorld();
	if (World)
	{
		int32 Key = MakeKey(NodeName);
		World->GetGameInstance()->GetSubsystem<UDoOnceSaverSubsystem>()->DoOnceReset(Object,Key);
	}
}

int32 UDoOnceImplementation::MakeKey(FString NodeName)
{
	FString SaveKey;
	FString LeftString;
	NodeName.Split("_",&LeftString,&SaveKey,ESearchCase::CaseSensitive,ESearchDir::FromEnd);
	return UKismetStringLibrary::Conv_StringToInt(SaveKey);
}
