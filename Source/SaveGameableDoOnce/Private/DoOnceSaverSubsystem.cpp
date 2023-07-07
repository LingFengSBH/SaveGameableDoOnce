// Copyright Sun BoHeng(LingFeng) 2023

#include "DoOnceSaverSubsystem.h"

bool UDoOnceSaverSubsystem::TryToDo(UObject* Object,int32 Key, bool StartClosed)
{
	TMap<int32,bool>* DoOnceInfos;
	//This object is not found
	//It's the first time to call execute;
	if (!FindHasDoneInfos(Object,DoOnceInfos))
	{
		HasDoneMap.Add(Object,FDoOnceInfos(Key,true));
		if (StartClosed)
		{
			//Start closed,not found in map,so it is not reset too;
			return false;
		}
		//else
		if (OnceHasDone.IsBound())
		{
			OnceHasDone.Broadcast(Object,Key);
		}
		return true;
	}
	//else,the object is found
	{
		bool* HasDoneRef = DoOnceInfos->Find(Key);
		if (HasDoneRef==nullptr)
		{
			//It's the first time to call execute of this node;
			DoOnceInfos->Add(Key,true);
			if (StartClosed)
			{
				//Start closed,not found in map,so it is not reset too;
				return false;
			}
			//else
			if (OnceHasDone.IsBound())
			{
				OnceHasDone.Broadcast(Object,Key);
			}
			return true;
		}
		//else found bool HasDone
		{
			if (*HasDoneRef)
			{
				//Has executed
				return false;
			}
			//else,set HasDone true,and return true;
			*HasDoneRef = true;
			if (OnceHasDone.IsBound())
			{
				OnceHasDone.Broadcast(Object,Key);
			}
			return true;
		}
	}
}

void UDoOnceSaverSubsystem::SetHasDoneInfosOfObject(UObject* Object, TMap<int32, bool>& Infos)
{
	HasDoneMap.Add(Object,FDoOnceInfos(Infos));
}

void UDoOnceSaverSubsystem::ClearInvalidObjectKeys()
{
	TArray<UObject*> ObjectKeys;
	HasDoneMap.GetKeys(ObjectKeys);
	for (auto Key : ObjectKeys)
	{
		if (!IsValid(Key))
		{
			HasDoneMap.Remove(Key);
		}
	}
}

void UDoOnceSaverSubsystem::DoOnceReset(UObject* Object,int32 Key)
{
	TMap<int32,bool>* DoOnceInfos;
	if (FindHasDoneInfos(Object,DoOnceInfos))
	{
		//Directly add,when this key exists,it will replace the old value
		DoOnceInfos->Add(Key,false);
	}
	else
	{
		//The Object is not found in map,so we add a new one
		HasDoneMap.Add(Object,FDoOnceInfos(Key,false));
	}
	
	if (DoOnceHasReset.IsBound())
	{
		DoOnceHasReset.Broadcast(Object,Key);
	}
}

bool UDoOnceSaverSubsystem::FindHasDoneInfos(UObject* Object,TMap<int32,bool>*& OutDoOnceInfos)
{
	OutDoOnceInfos = &HasDoneMap.Find(Object)->Info;
	return OutDoOnceInfos!=nullptr;
}
