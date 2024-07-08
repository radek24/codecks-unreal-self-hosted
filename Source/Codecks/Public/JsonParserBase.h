#pragma once
#include "CoreMinimal.h"
#include "JsonParserBase.generated.h"


UCLASS(Abstract)
class CODECKS_API UJsonParserBase : public UObject
{
	GENERATED_BODY()

public:
    // Constructor
    UJsonParserBase();

    // Function that can be overridden in Blueprint
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Codecks")
    void ParseJsonFromClipboard(const FString& input);
    virtual void ParseJsonFromClipboard_Implementation(const FString& input);
};