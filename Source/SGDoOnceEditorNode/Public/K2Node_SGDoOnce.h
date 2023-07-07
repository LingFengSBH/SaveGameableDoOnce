// Copyright Sun BoHeng(LingFeng) 2023

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_SGDoOnce.generated.h"

/**
 * 
 */
UCLASS()
class SGDOONCEEDITORNODE_API UK2Node_SGDoOnce : public UK2Node
{
	GENERATED_BODY()

	FText NodeName = FText::FromString("SaveGameable Do Once");
	
	// UEdGraphNode interface
	virtual void AllocateDefaultPins() override;
	virtual bool IncludeParentNodeContextMenu() const override { return true; }
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FName GetCornerIcon() const override;;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FLinearColor GetNodeBodyTintColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetKeywords() const override;
	// End of UK2Node interface

private:
	//Find a pin of this node by pin name
	UEdGraphPin* FindPin(FString Name);
};
