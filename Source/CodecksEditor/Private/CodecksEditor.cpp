// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodecksEditor.h"
#include "CodecksEditorStyle.h"
#include "CodecksEditorCommands.h"
#include "ToolMenus.h"
#include "CodecksSettings.h"
#include "JsonParserBase.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"

#include "GenericPlatform/GenericPlatformApplicationMisc.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

static const FName CodecksEditorTabName("CodecksEditor");

#define LOCTEXT_NAMESPACE "FCodecksEditorModule"

void FCodecksEditorModule::StartupModule()
{

	RegisterSettings();
	FCodecksEditorStyle::Initialize();
	FCodecksEditorStyle::ReloadTextures();

	FCodecksEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCodecksEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FCodecksEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCodecksEditorModule::RegisterMenus));
}

void FCodecksEditorModule::ShutdownModule()
{
	UnregisterSettings();
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FCodecksEditorStyle::Shutdown();
	FCodecksEditorCommands::Unregister();
}

void FCodecksEditorModule::PluginButtonClicked()
{
	FString JsonString;
	FPlatformApplicationMisc::ClipboardPaste(JsonString);
	TSoftClassPtr<UJsonParserBase> CustomJsonParserSoftClass= (GetDefault<UCodecksSettings>()->CustomJsonParser);
	if (CustomJsonParserSoftClass.IsNull())
	{
		FText DialogText = FText::FromString("Json parser wasn't set, set the default one or custom one in Project Settings > Codecks");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return;
	}
	UClass* classReference = CustomJsonParserSoftClass.LoadSynchronous();
	UJsonParserBase* MyCustomJsonParser = Cast<UJsonParserBase>(classReference->GetDefaultObject());
	MyCustomJsonParser->ParseJsonFromClipboard(JsonString);
	
}

void FCodecksEditorModule::RegisterSettings()
{
	if (ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		ISettingsSectionPtr settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "Codecks",
			LOCTEXT("ReportingToolSettingsName", "Codecks"),
			LOCTEXT("ReportingToolSettingsDescription", "Configure the Codecks settings."),
			GetMutableDefault<UCodecksSettings>()
		);
	}
}

void FCodecksEditorModule::UnregisterSettings()
{
	if (ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		settingsModule->UnregisterSettings("Project", "Plugins", "Codecks");
	}
}



void FCodecksEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCodecksEditorCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCodecksEditorCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCodecksEditorModule, CodecksEditor)