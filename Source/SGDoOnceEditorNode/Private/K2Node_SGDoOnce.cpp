// Copyright Sun BoHeng(LingFeng) 2023

#include "K2Node_SGDoOnce.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "DoOnceImplementation.h"
#include "EdGraphSchema_K2.h"
#include "EditorCategoryUtils.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Self.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"

FText UK2Node_SGDoOnce::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NodeName;
}

void UK2Node_SGDoOnce::AllocateDefaultPins()
{
	///////////////////////////////////
	//  SaveGameable Do Once         //
	///////////////////////////////////
	// > Execute         Completed > //
	// > Reset            NodeName O //
	// O StartClosed                 //
	///////////////////////////////////
	
	Super::AllocateDefaultPins();
	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Exec,NAME_None,nullptr,TEXT("Execute"));
	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Exec,NAME_None,nullptr,TEXT("Reset"));
	CreatePin(EGPD_Output,UEdGraphSchema_K2::PC_Exec,NAME_None,nullptr,TEXT("Completed"));
	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Boolean,NAME_None,nullptr,TEXT("StartClosed"));
	CreatePin(EGPD_Output,UEdGraphSchema_K2::PC_String,NAME_None,nullptr,TEXT("NodeName"))
	->PinToolTip = TEXT("The object name of this pin \n\nThis Plugin use this name to create key for each pin\ne.g. If this name is K2Node_SGDoOnce_14,the key will be '14'");

	CreatePin(EGPD_Input,UEdGraphSchema_K2::PC_Boolean,NAME_None,nullptr,TEXT("EfficientMode"))
	->PinToolTip = TEXT("Switch the mode of this node \n\n*If false,each time execute and reset will call functions in subsystem and check 'Do Once state' of this node \n*If true,some temporary boolean will be created to avoid checking every time you execute or reset \n\n*But in EfficientMode the node will not sync with SaverSubsystem if you override the 'Do Once state' in subsystem \n*If you load your game without refresh your objects,it's better to be false");
}

FSlateIcon UK2Node_SGDoOnce::GetIconAndTint(FLinearColor& OutColor) const
{
	//This is the icon of "DoOnce" in editor
	static const FSlateIcon Icon = FSlateIcon("EditorStyle", "GraphEditor.Macro.DoOnce_16x");
	return Icon;

}

FName UK2Node_SGDoOnce::GetCornerIcon() const
{
	//This type is created in FSGDoOnceEditorNodeModule::StartupModule()
	return TEXT("GraphEditor.SaveGameableDoOnce_32x");
}

FLinearColor UK2Node_SGDoOnce::GetNodeTitleColor() const
{
	return FLinearColor::White;
}

FLinearColor UK2Node_SGDoOnce::GetNodeBodyTintColor() const
{
	return FLinearColor::White;
}

/*Create a UK2Node_TemporaryVariable
 *OutVar:UK2Node_TemporaryVariable*,the node that created;
 *PinType:FName,desired a UEdGraphSchema_K2:: It decides the VariableType,like boolean,string,int32. 
 */
#define TEMPORARY_VARIABLE(OutVar,PinType) UK2Node_TemporaryVariable* OutVar = SourceGraph->CreateIntermediateNode<UK2Node_TemporaryVariable>();\
OutVar->VariableType.PinCategory=PinType;\
OutVar->AllocateDefaultPins();\
CompilerContext.MessageLog.NotifyIntermediateObjectCreation(OutVar,this) 

/*Create a UK2Node_AssignmentStatement
 *OutVar:UK2Node_AssignmentStatement*,the node that created;
 *AssignVar:UK2Node_TemporaryVariable*,the Variable to link to VariablePin,it also decides the VariablePin type of this node.
 */
#define ASSIGN_NODE(OutVar,AssignVar,Default) UK2Node_AssignmentStatement* OutVar = SourceGraph->CreateIntermediateNode<UK2Node_AssignmentStatement>();\
OutVar->AllocateDefaultPins();\
CompilerContext.MessageLog.NotifyIntermediateObjectCreation(OutVar,this);\
OutVar->GetVariablePin()->PinType = AssignVar->GetVariablePin()->PinType;\
OutVar->GetValuePin()->PinType = AssignVar->GetVariablePin()->PinType;\
OutVar->GetVariablePin()->MakeLinkTo(AssignVar->GetVariablePin());\
OutVar->GetValuePin()->DefaultValue = Default

/*Create a UK2Node_CallFunction.
 *This macro will only set function from UDoOnceImplementation::StaticClass();
 *OutVar:UK2Node_CallFunction*,the node that created;
 *FunctionName:string,a function name of UDoOnceImplementation::StaticClass()
 */
#define CALL_IMPLEMENTATION(OutVar,FunctionName) UK2Node_CallFunction* OutVar = SourceGraph->CreateIntermediateNode<UK2Node_CallFunction>();\
OutVar->SetFromFunction(UDoOnceImplementation::StaticClass()->FindFunctionByName(#FunctionName));\
OutVar->AllocateDefaultPins();\
CompilerContext.MessageLog.NotifyIntermediateObjectCreation(OutVar,this)

/*Create a UK2Node_IfThenElse node
 *OutVar:UK2Node_IfThenElse*,the node that created;
 *ConditionPin:UEdGraphPin*,Which pin should be linked to the condition pin of Branch?
 */
#define BRANCH_NODE(OutVar,ConditionPin) UK2Node_IfThenElse* OutVar = SourceGraph->CreateIntermediateNode<UK2Node_IfThenElse>();\
OutVar->AllocateDefaultPins();\
CompilerContext.MessageLog.NotifyIntermediateObjectCreation(OutVar,this);\
OutVar->GetConditionPin()->MakeLinkTo(ConditionPin)

void UK2Node_SGDoOnce::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	//Find five pins of this K2Node
	
	///////////////////////////////////
	//  SaveGameable Do Once         //
	///////////////////////////////////
	// > Execute         Completed > //
	// > Reset            NodeName O //
	// O StartClosed                 //
	///////////////////////////////////
	UEdGraphPin* ExecutePin = FindPin(TEXT("Execute"));
	UEdGraphPin* ResetPin = FindPin(TEXT("Reset"));
	UEdGraphPin* ThenPin = FindPin(TEXT("Completed"));
	UEdGraphPin* StartClosed = FindPin(TEXT("StartClosed"));
	UEdGraphPin* NodeNamePin = FindPin(TEXT("NodeName"));
	UEdGraphPin* EfficientModePin = FindPin(TEXT("EfficientMode"));
	
	check(ExecutePin);
	check(ResetPin);
	check(ThenPin);
	check(StartClosed);
	check(NodeNamePin);
	check(EfficientModePin)

	//These functions are declared in UDoOnceImplementation
    CALL_IMPLEMENTATION(CallSGDoOnceImp,SGDoOnceImplementation);
	CALL_IMPLEMENTATION(CallSGResetImp,SGResetImplementation);
	CALL_IMPLEMENTATION(CallSGMakeStringImp,SGMakeStringImplementation);
	/*It will be like:
	UK2Node_CallFunction* CallSGDoOnceImp = SourceGraph->CreateIntermediateNode<UK2Node_CallFunction>();
	CallSGDoOnceImp->SetFromFunction(UDoOnceImplementation::StaticClass()->FindFunctionByName("SGDoOnceImplementation"));
	CallSGDoOnceImp->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallSGDoOnceImp,this);
	*/
	
	//IfThenElse Node,in blueprint,they are called "Branch"
	BRANCH_NODE(BranchComplete,CallSGDoOnceImp->GetReturnValuePin());
	/*It will be like:
	UK2Node_IfThenElse* BranchComplete = SourceGraph->CreateIntermediateNode<UK2Node_IfThenElse>();
	BranchComplete->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(BranchComplete,this);
	*/
	
	//CallSGMakeStringImp parameter
	CallSGMakeStringImp->FindPinChecked(TEXT("String"),EGPD_Input)->DefaultValue=this->GetName();

	//Create Self node
	UK2Node_Self* SelfNode = SourceGraph->CreateIntermediateNode<UK2Node_Self>();
	SelfNode->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(SelfNode,this);

	if (EfficientModePin->DefaultValue==TEXT("false")||EfficientModePin->DefaultValue.IsEmpty())
	{
		UE_LOG(LogTemp,Log,TEXT("%s : EfficientMode==false"),*this->GetName());
		BRANCH_NODE(Branch,CallSGDoOnceImp->GetReturnValuePin());
		
		CompilerContext.MovePinLinksToIntermediate(*ExecutePin,*CallSGDoOnceImp->GetExecPin());
		Schema->TryCreateConnection(CallSGDoOnceImp->GetThenPin(),Branch->GetExecPin());
		CompilerContext.MovePinLinksToIntermediate(*ThenPin,*Branch->GetThenPin());

		CompilerContext.MovePinLinksToIntermediate(*ResetPin,*CallSGResetImp->GetExecPin());
	}
	else
	{
		UE_LOG(LogTemp,Log,TEXT("%s : EfficientMode==true"),*this->GetName());
		//Create temporary variables
		//Boolean if Do Once has been called,if true means it will not do again.With this bool,we don't need to call functions in subsystem to check every time.
		TEMPORARY_VARIABLE(HasDoneBool,UEdGraphSchema_K2::PC_Boolean);
		//If has been reset,if has reset,we will not call functions in subsystem for better performance(Sometimes you reset without execute do once,and reset again,that's unnecessary)
		TEMPORARY_VARIABLE(ResetedBool,UEdGraphSchema_K2::PC_Boolean);
		/*It will be like:
		UK2Node_TemporaryVariable* KeyString = SourceGraph->CreateIntermediateNode<UK2Node_TemporaryVariable>();
		KeyString->VariableType.PinCategory=UEdGraphSchema_K2::PC_String;
		KeyString->AllocateDefaultPins();
		CompilerContext.MessageLog.NotifyIntermediateObjectCreation(KeyString,this);
		*/

		//Some AssignmentNode to set temporary variables' value declared above
		ASSIGN_NODE(AssignmentResetTrue,ResetedBool,TEXT("1"));
		ASSIGN_NODE(AssignmentResetFalse,ResetedBool,TEXT("0"));
		ASSIGN_NODE(AssignmentHasDoneTrue,HasDoneBool,TEXT("1"));
		ASSIGN_NODE(AssignmentHasDoneFalse,HasDoneBool,TEXT("0"));
		/*It will be like:
		UK2Node_AssignmentStatement* AssignmentResetTrue=  SourceGraph->CreateIntermediateNode<UK2Node_AssignmentStatement>();
		AssignmentResetTrue->AllocateDefaultPins();
		CompilerContext.MessageLog.NotifyIntermediateObjectCreation(AssignmentResetTrue,this);
		AssignmentResetTrue->GetVariablePin()->PinType = KeyString->GetVariablePin()->PinType;
		AssignmentResetTrue->GetValuePin()->PinType = KeyString->GetVariablePin()->PinType;
		AssignmentResetTrue->GetVariablePin()->MakeLinkTo(KeyString->GetVariablePin());
		AssignmentResetTrue->GetValuePin()->DefaultValue = this->GetName();
		*/

		//We need two more branches
		BRANCH_NODE(BranchHasDone,HasDoneBool->GetVariablePin());
		BRANCH_NODE(BranchReset,ResetedBool->GetVariablePin());
		
	
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// SaveGameable Do Once                                                                                                                                                                                                            //
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//               |                                                                                                                                                                                                  |              //
		//               |               |BranchHasDone        |                                                                                                                                                            |              //
		//      Execute >|------1--------|>              Then >|       |AssignmentHasDoneTrue|            |CallSGDoOnceImp         |        |BranchComplete       |        |AssignmentHasDoneTrue|                          |              //
		//               | HasDoneBool---|O condition    Else >|---2---|>                   >|------3-----|>                      >|---4----|>              Then >|---5----|>                   >|-------------6------------|> Completed   // 
		//               |                                             |                     |            |            ReturnValueO|--------|O condition    Else >|        |                     |                          |              //
		//               |                            HasDoneBool------|O Variable           |            |                        |                                       |                     |                          |              //
		//               |                                             |O Value "1"          |            |                        |                    HasDoneBool--------|O Variable           |                          |              //
		//               |                                                                        /-----> |O StartClosed           |                                       |O Value "1"          |                          |              //
		//  StartClosed O|-----------------------------------------------13----------------------/        |                        |                                                                                        |              // 
		//               |                                                                      self--12--|O Object                |                                                                                        |              //
		//               |                                                             /--------11--------|O NodeName              |                                                                                        |              //
		//               |                         |CallSGMakeStringImp         |    /                                                                                                                                      |              //
		//               |       this->GetName()---|O String       ReturnValue O|---\-------------------------------------------------------------------------------------------16------------------------------------------|O NodeName    //
		//               |                                                           \                                                                                                                                      |              //
		//               |             |BranchReset          |                        \                                                                                                                                     |              //
		//        Reset >|------7------|>              Then >|                         \                  |CallSGResetImp         |                   |AssignmentHasDoneFalse|                  |AssignmentResetTrue|       |              //
		//               | ResetBool---|O condition    Else >|------------8-------------\-----------------|>                     >|--------9----------|>                    >|---------10-------|>                 >|       |              //
		//               |                                                               \                |                       |   HasDoneBool-----|O Variable            |    ResetBool-----|O Variable         |       |              //
		//               |                                                                \--------14-----|O NodeName             |                   |O Value "0"           |                  |O Value "1"        |       |              //
		//               |                                                                                |                       |                                                                                         |              //
		//               |                                                                      self--15--|O Object               |                                                                                         |              //
		//               |                                                                                                                                                                                                  |              //
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//Now,let's link these nodes like blueprint above
		//So,why I didn't create a Blueprint Macro Library,and do it with blueprint?
		//The main idea of this plugin is to save each "Do Once",so we need to generate a specific "Key" for each node
		//My solution is using Object name of this node,it will not repeat in each blueprint graph
		//If you have many "SaveGameable Do Once" Nodes in one graph,their names would be "K2Node_SGDoOnce_1","K2Node_SGDoOnce_2","K2Node_SGDoOnce_3"etc.
		//But we can't get the node's Object name,"Self" in blueprint refs the object runs this node
		//We need "this",which refs this node itself.
		
		//These number corresponds to the number written in the link lines above
		//We don't need to connect Condition pins of Branches and Variable pins of Assignment nodes.Because we have connected them in MACRO.
		//1
		CompilerContext.MovePinLinksToIntermediate(*ExecutePin,*BranchHasDone->GetExecPin());
		//2
		Schema->TryCreateConnection(BranchHasDone->GetElsePin(),AssignmentHasDoneTrue->GetExecPin());
		//3
		Schema->TryCreateConnection(AssignmentHasDoneTrue->GetThenPin(),CallSGDoOnceImp->GetExecPin());
		//4
		Schema->TryCreateConnection(CallSGDoOnceImp->GetThenPin(),BranchComplete->GetExecPin());
		//5
		Schema->TryCreateConnection(BranchComplete->GetThenPin(),AssignmentResetFalse->GetExecPin());
		//6
		CompilerContext.MovePinLinksToIntermediate(*ThenPin,*AssignmentResetFalse->GetThenPin());

		//7
		CompilerContext.MovePinLinksToIntermediate(*ResetPin,*BranchReset->GetExecPin());
		//8
		Schema->TryCreateConnection(BranchReset->GetElsePin(),CallSGResetImp->GetExecPin());
		//9
		Schema->TryCreateConnection(CallSGResetImp->GetThenPin(),AssignmentHasDoneFalse->GetExecPin());
		//10
		Schema->TryCreateConnection(AssignmentHasDoneFalse->GetThenPin(),AssignmentResetTrue->GetExecPin());
	}
	
	//CallSGDoOnceImp parameters
	//11
	Schema->TryCreateConnection(CallSGMakeStringImp->GetReturnValuePin(),CallSGDoOnceImp->FindPinChecked(TEXT("NodeName"),EGPD_Input));
	//12
	Schema->TryCreateConnection(SelfNode->FindPinChecked(UEdGraphSchema_K2::PN_Self),CallSGDoOnceImp->FindPinChecked(TEXT("Object"),EGPD_Input));
    //13
	CompilerContext.MovePinLinksToIntermediate(*StartClosed,*CallSGDoOnceImp->FindPinChecked(TEXT("StartClosed"),EGPD_Input));

	//CallSGResetImp parameters
	//14
	Schema->TryCreateConnection(CallSGMakeStringImp->GetReturnValuePin(),CallSGResetImp->FindPinChecked(TEXT("NodeName"),EGPD_Input));
	//15
	Schema->TryCreateConnection(SelfNode->FindPinChecked(UEdGraphSchema_K2::PN_Self),CallSGResetImp->FindPinChecked(TEXT("Object"),EGPD_Input));
	
	//This pin returns the node name
	//16
	CompilerContext.MovePinLinksToIntermediate(*NodeNamePin,*CallSGMakeStringImp->GetReturnValuePin());
	
	BreakAllNodeLinks();
}
#undef TEMPORARY_VARIABLE
#undef ASSIGN_NODE
#undef CALL_IMPLEMENTATION
#undef BRANCH_NODE


void UK2Node_SGDoOnce::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_SGDoOnce::GetMenuCategory() const
{
	return FEditorCategoryUtils::BuildCategoryString(FCommonEditorCategory::FlowControl,NodeName);
}

FText UK2Node_SGDoOnce::GetKeywords() const
{
	return FText::FromString("DoOnce");
}

UEdGraphPin* UK2Node_SGDoOnce::FindPin(FString Name)
{
	for (auto pin : Pins)
	{
		if (pin->GetName()==Name)
		{
			return pin;
		}
	}
	return nullptr;
}
