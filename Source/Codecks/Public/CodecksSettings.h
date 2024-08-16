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
	*  If you check this, plugin will send report based on current configuration (Shipping => public , Anything else => Internal). If unchecked everything will sent to "public"
	*/
	UPROPERTY(Config, EditAnywhere, Category = "General", meta = (DisplayName = "Use Different Reporting Token Per Configuration"))
	bool useDifferentReportingTokenPerConfiguration = false;

	/**
	*  Json parser that will be used for json in your clipboard when you click on editor button
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	TSoftClassPtr<UJsonParserBase> CustomJsonParser = TSoftClassPtr<UJsonParserBase>(FSoftObjectPath("/Script/CodecksEditor.JsonParser"));

	/**
	 *  This url will be used for card creation. If you are self hosting, modify this field
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Public Reporting", meta = (DisplayName = "Codecks API Url"))
	FString PublicCodecksURL = TEXT("https://api.codecks.io/user-report/v1/create-report?token=");

	/**
	 *  The Codecks token for your Bug reports deck
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Public Reporting", meta = (DisplayName = "Report Token"))
	FString PublicReportToken;

	/**
	 *  The Codecks token for your Feedback deck, can be same as Bug deck if you want all your reports to fall in the same deck
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Public Reporting", meta = (DisplayName = "Feedback Token"))
	FString PublicFeedbackToken;

	/**
	*  This url will be used for card creation. If you are self hosting, modify this field
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Internal Reporting", meta = (DisplayName = "Codecks API Url", EditCondition = "useDifferentReportingTokenPerConfiguration"))
	FString InternalCodecksURL = TEXT("https://api.codecks.io/user-report/v1/create-report?token=");

	/**
	 *  The Codecks token for your Bug reports deck
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Internal Reporting", meta = (DisplayName = "Report Token", EditCondition = "useDifferentReportingTokenPerConfiguration"))
	FString InternalReportToken;

	/**
	 *  The Codecks token for your Feedback deck, can be same as Bug deck if you want all your reports to fall in the same deck
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Internal Reporting", meta = (DisplayName = "Feedback Token", EditCondition = "useDifferentReportingTokenPerConfiguration"))
	FString InternalFeedbackToken;




	
};