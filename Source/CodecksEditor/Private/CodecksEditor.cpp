// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodecksEditor.h"
#include "CodecksEditorStyle.h"
#include "CodecksEditorCommands.h"
#include "ToolMenus.h"
#include "CodecksSettings.h"
#include "JsonParserBase.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "HAL/PlatformApplicationMisc.h"

static const FName CodecksEditorTabName("CodecksEditor");

#define LOCTEXT_NAMESPACE "FCodecksEditorModule"



bool AreSettingInvalid(const UCodecksSettings* CodecksSettings){
	bool InternalSettingsInvalid = CodecksSettings->useDifferentReportingTokenPerConfiguration && (CodecksSettings->InternalReportToken.IsEmpty() || CodecksSettings->InternalFeedbackToken.IsEmpty() || CodecksSettings->InternalCodecksURL.IsEmpty());
	return CodecksSettings->PublicFeedbackToken.IsEmpty() || CodecksSettings->PublicReportToken.IsEmpty() || CodecksSettings->PublicCodecksURL.IsEmpty() || InternalSettingsInvalid;
}

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


	if (const UCodecksSettings* CodecksSettings = GetDefault<UCodecksSettings>())
	{
		if (AreSettingInvalid(CodecksSettings))
		{
			TSharedRef<FTokenizedMessage> WarnAboutNoReportTokens = FMessageLog("LoadErrors").Warning();
			WarnAboutNoReportTokens->AddToken(FTextToken::Create(INVTEXT("Codecks plugin wasnt set up properly! Make sure you do so in the settings.")));

			const auto GotoAction = FOnActionTokenExecuted::CreateLambda([CodecksSettings] {
				FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
					.ShowViewer(CodecksSettings->GetContainerName(), CodecksSettings->GetCategoryName(), CodecksSettings->GetSectionName());
				});
			WarnAboutNoReportTokens->AddToken(FActionToken::Create(INVTEXT("Open Codecks Settings"), INVTEXT("Goto Codecks User Report Settings"), GotoAction));
		}
	}
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
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Json parser wasn't set, set the default one or custom one in Project Settings > Codecks"));
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