// Copyright 2022 Maschinen-Mensch UG

#pragma once

#include <CoreMinimal.h>

#include "CodecksSettings.generated.h"

UCLASS(NotBlueprintType, DefaultConfig, Config = Game, ClassGroup = (Codecks), DisplayName = "Codecks")
class CODECKS_API UCodecksSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UCodecksSettings();

	/**
	 * @brief The Codecks token for your Parent Card. See https://github.com/codecks-io/codecks-unity/blob/main/Assets/Codecks_io/Codecks%20Bug%20%26%20Feedback%20Reporter/Documentation/docs.md#report-token
	 */
	UPROPERTY(Config, EditAnywhere)
	FString ReportToken = TEXT("XXXXXXXXXXXXXXXXXXXX");


	/**
	 * @brief This url will be used for card creation. If you are self hosting, modify this field
	 */
	UPROPERTY(Config, EditAnywhere)
	FString ReportCreateUrl = TEXT("https://api.codecks.io/user-report/v1/create-report?token=");
};