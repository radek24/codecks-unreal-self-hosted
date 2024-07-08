// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CodecksEditorStyle.h"

class FCodecksEditorCommands : public TCommands<FCodecksEditorCommands>
{
public:

	FCodecksEditorCommands()
		: TCommands<FCodecksEditorCommands>(TEXT("CodecksEditor"), NSLOCTEXT("Contexts", "CodecksEditor", "CodecksEditor Plugin"), NAME_None, FCodecksEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
