// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodecksEditorStyle.h"
#include "CodecksEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FCodecksEditorStyle::StyleInstance = nullptr;

void FCodecksEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FCodecksEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FCodecksEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("CodecksEditorStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FCodecksEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("CodecksEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Codecks")->GetBaseDir() / TEXT("Resources"));

	Style->Set("CodecksEditor.PluginAction", new IMAGE_BRUSH_SVG(TEXT("ParseJsonButtonIcon"), Icon20x20));
	return Style;
}

void FCodecksEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FCodecksEditorStyle::Get()
{
	return *StyleInstance;
}
