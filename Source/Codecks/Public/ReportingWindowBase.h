#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReportingWindowBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CODECKS_API UReportingWindowBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReportText")
	TMap<FString, FString> Metadata;

	UFUNCTION(BlueprintPure)
	FString GetMetadataString();

	
};
