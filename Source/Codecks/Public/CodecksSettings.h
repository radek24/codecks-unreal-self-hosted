// Copyright 2022 Maschinen-Mensch UG

#pragma once

#include <CoreMinimal.h>
#include "JsonParserBase.h"
#include "CodecksSettings.generated.h"


//class UJsonParser;

UCLASS(NotBlueprintType, DefaultConfig, Config = Game, ClassGroup = (Codecks), DisplayName = "Codecks")
class CODECKS_API UCodecksSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UCodecksSettings();
	/**
	 *  The Codecks token for your Parent Card. See https://github.com/codecks-io/codecks-unity/blob/main/Assets/Codecks_io/Codecks%20Bug%20%26%20Feedback%20Reporter/Documentation/docs.md#report-token
	 */
	UPROPERTY(Config, EditAnywhere)
	FString ReportToken = TEXT("XXXXXXXXXXXXXXXXXXXX");

	/**
	 *  This url will be used for card creation. If you are self hosting, modify this field
	 */
	UPROPERTY(Config, EditAnywhere)
	FString ReportCreateUrl = TEXT("https://api.codecks.io/user-report/v1/create-report?token=");

	/**
	*  Json parser that will be used for json in your clipboard when you click on editor button
	*/
	UPROPERTY(Config, EditAnywhere)
	TSoftClassPtr<UJsonParserBase> CustomJsonParser = TSoftClassPtr<UJsonParserBase>(FSoftObjectPath("/Script/CodecksEditor.JsonParser"));
};