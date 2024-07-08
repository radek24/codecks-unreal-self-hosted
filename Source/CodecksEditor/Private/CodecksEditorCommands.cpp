// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodecksEditorCommands.h"

#define LOCTEXT_NAMESPACE "FCodecksEditorModule"

void FCodecksEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Parse JSON Location", "Parse location from json in your clipboard and teleports you there", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
