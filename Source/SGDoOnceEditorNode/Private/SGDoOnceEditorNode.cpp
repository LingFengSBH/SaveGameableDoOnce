// Copyright Sun BoHeng(LingFeng) 2023
#include "SGDoOnceEditorNode.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"

//Compatibility 
#if ENGINE_MAJOR_VERSION <= 4 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1)
#define GET_EDITOR_STYLE (FSlateStyleSet*)&FEditorStyle::Get()
typedef FVector2D CompVector2D;
#else
#define GET_EDITOR_STYLE (FSlateStyleSet*)&FAppStyle::Get()
typedef FVector2d CompVector2D;
#endif

#define LOCTEXT_NAMESPACE "SGDoOnceNodeStyle"
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( CurrentEditorStyle->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

void FSGDoOnceEditorNodeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	#if WITH_EDITOR
	FSlateStyleSet* CurrentEditorStyle = GET_EDITOR_STYLE;
	CurrentEditorStyle->SetContentRoot(IPluginManager::Get().FindPlugin("SaveGameableDoOnce")->GetBaseDir() / TEXT("Resources"));
	
	CurrentEditorStyle->Set("GraphEditor.SaveGameableDoOnce_32x", new IMAGE_BRUSH("SGDO_Icon32", CompVector2D(32.0f, 32.0f)));
    #endif
	
}

void FSGDoOnceEditorNodeModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSGDoOnceEditorNodeModule, SGDoOnceEditorNode)