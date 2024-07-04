// Copyright 2022 Maschinen-Mensch UG

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <Delegates/DelegateCombinations.h>
#include "CodecksCardCreator.generated.h"

UENUM()
enum class CodecksSeverity : uint8
{
	None = 0,
	Low,
	High,
	Critical
};

UENUM()
enum class CodecksFileType : uint8
{
	Binary,
	PlainText,
	JSON,
	PNG,
	JPG
};

UENUM(BlueprintType)
enum class CodecksCardCreationStatus : uint8
{
	//Card got created fully
	Success = 0,

	//Card got created, but attachments missing
	Partially,

	//Card got not created
	Fail
};

UENUM(BlueprintType)
enum class Reproducibility : uint8
{
	EveryTime = 0,
	Sometimes,
	Randomly,
	Untested,
	Never,
};

UENUM(BlueprintType)
enum class Category : uint8
{
	Localization = 0,
	Performace,
	Save,
	Settings,
	AI,
	Audio,
	General,
	Graphics,
	UI
};



USTRUCT(BlueprintType)
struct FCodecksFileInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString filename;

	UPROPERTY(BlueprintReadWrite)
	CodecksFileType type = CodecksFileType::PlainText;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> data;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FCodecksCardCreated, CodecksCardCreationStatus, status);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCodecksCardError, FString, ErrorMessage);

DECLARE_DYNAMIC_DELEGATE_OneParam(FCodecksScreenshotCreated, FCodecksFileInfo, screenshotFile);

UCLASS()
class CODECKS_API UCodecksCardCreator : public UObject
{
	GENERATED_BODY()

public:

	// Creates a standardized text that will be used in created card
	UFUNCTION(BlueprintPure, Category = Codecks)
	static void CreateCardText(FString& outCardText, const FString header, const FString content, const Category category, const Reproducibility reproducibility,const FString email);


	// creates a codecks card (requires a codecks token configured in the plugin project settings)
	UFUNCTION(BlueprintCallable, Category = Codecks)
	static void CreateNewCodecksCard(
		const FString & text,
		//pass files as a non-const input variable so we avoid a copy. This array will be moved and cleared after call.
		UPARAM(ref) TArray<FCodecksFileInfo> & files, 
		const FCodecksCardCreated& createdCallback,
		const FCodecksCardError& errorCallback,
		const CodecksSeverity severity = CodecksSeverity::None,
		const FString& userEmail = TEXT("")
	);

	// helper function for taking a screenshot and preparing it as a png attachment for codecks
	UFUNCTION(BlueprintCallable, Category = Codecks)
	static void TakeScreenshotHelper(bool showUI, FString filename,const FCodecksScreenshotCreated& createdCallback);


	static FString EnumToString(const FString& enumName, const uint8 value);

private:
	static FDelegateHandle screenshotDelegateHandle;
	
};
